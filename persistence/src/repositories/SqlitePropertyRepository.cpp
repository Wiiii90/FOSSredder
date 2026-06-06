/**
 * @file persistence/src/repositories/SqlitePropertyRepository.cpp
 * @brief Implements the SQLite-backed property repository.
 */

#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Property.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <stdexcept>

struct SqlitePropertyRepository::Impl {
    std::shared_ptr<SqliteDb> db;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

namespace {

constexpr auto kSelectProperty =
    "SELECT id, name FROM properties";

struct AliasStats {
    int hitCount = 1;
    std::string lastUsedAt;
    std::string createdAt;
    std::string updatedAt;
};

core::domain::Alias makeAlias(const std::string& alias,
                              int hitCount,
                              const std::string& createdAt,
                              const std::string& updatedAt,
                              const std::string& lastUsedAt)
{
    core::domain::Alias value;
    value.setValue(alias);
    value.setSource(alias);
    value.setHitCount(hitCount);
    value.setCreatedAt(createdAt);
    value.setUpdatedAt(updatedAt);
    value.setLastUsedAt(lastUsedAt);
    return value;
}

std::vector<core::domain::Alias> collectAliases(const Property& property)
{
    std::vector<core::domain::Alias> out;
    out.reserve(property.aliases().size());
    for (const auto& alias : property.aliases()) {
        if (alias.value().empty()) continue;
        auto value = alias;
        if (value.hitCount() < 1) value.setHitCount(1);
        out.push_back(std::move(value));
    }
    return out;
}

std::unordered_map<std::string, AliasStats> loadPropertyAliasStats(sqlite3* db, const std::string& propertyId)
{
    std::unordered_map<std::string, AliasStats> out;
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM property_aliases WHERE property_id = ?;");
    if (!stmt) return out;

    stmt.bindText(1, propertyId);
    while (stmt.step() == SQLITE_ROW) {
        AliasStats stats;
        stats.hitCount = stmt.columnInt(1);
        stats.createdAt = stmt.columnText(2);
        stats.updatedAt = stmt.columnText(3);
        stats.lastUsedAt = stmt.columnText(4);
        out.emplace(stmt.columnText(0), std::move(stats));
    }

    return out;
}

void loadPropertyAliases(sqlite3* db, Property& property)
{
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM property_aliases WHERE property_id = ? ORDER BY hit_count DESC, last_used_at DESC, alias ASC;");
    if (!stmt) return;

    stmt.bindText(1, property.id());
    std::vector<core::domain::Alias> aliases;
    while (stmt.step() == SQLITE_ROW) {
        aliases.push_back(makeAlias(stmt.columnText(0),
                                    stmt.columnInt(1),
                                    stmt.columnText(2),
                                    stmt.columnText(3),
                                    stmt.columnText(4)));
    }
    property.setAliases(std::move(aliases));
}

void deletePropertyAliases(sqlite3* db, const std::string& propertyId)
{
    persistence::StmtGuard stmt(db, "DELETE FROM property_aliases WHERE property_id = ?;");
    if (!stmt) return;

    stmt.bindText(1, propertyId);
    stmt.step();
}

void replacePropertyAliases(sqlite3* db,
                            const Property& property,
                            const std::unordered_map<std::string, AliasStats>& existing)
{
    if (property.id().empty()) return;

    deletePropertyAliases(db, property.id());

    persistence::StmtGuard stmt(db,
        "INSERT INTO property_aliases (property_id, alias, hit_count, created_at, updated_at, last_used_at) VALUES (?, ?, ?, COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP));");
    if (!stmt) return;

    std::unordered_set<std::string> seen;
    for (const auto& alias : collectAliases(property)) {
        if (alias.value().empty() || !seen.insert(alias.value()).second) continue;

        const auto it = existing.find(alias.value());
        const int hitCount = it == existing.end() ? std::max(1, alias.hitCount()) : std::max(1, it->second.hitCount);
        const std::string createdAt = it == existing.end() || it->second.createdAt.empty() ? alias.createdAt() : it->second.createdAt;
        const std::string updatedAt = it == existing.end() || it->second.updatedAt.empty() ? alias.updatedAt() : it->second.updatedAt;
        const std::string lastUsedAt = it == existing.end() || it->second.lastUsedAt.empty() ? alias.lastUsedAt() : it->second.lastUsedAt;
        stmt.reset();
        stmt.bindText(1, property.id());
        stmt.bindText(2, alias.value());
        stmt.bindInt(3, hitCount);
        stmt.bindText(4, createdAt);
        stmt.bindText(5, updatedAt);
        stmt.bindText(6, lastUsedAt);
        stmt.step();
    }
}

} // namespace

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath)
    : SqlitePropertyRepository(std::make_shared<SqliteDb>(dbPath), nullptr) {}

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath,
                                                   std::shared_ptr<core::errors::IErrorReporter> er)
    : SqlitePropertyRepository(std::make_shared<SqliteDb>(dbPath), std::move(er)) {}

SqlitePropertyRepository::SqlitePropertyRepository(std::shared_ptr<SqliteDb> db)
    : SqlitePropertyRepository(std::move(db), nullptr) {}

SqlitePropertyRepository::SqlitePropertyRepository(std::shared_ptr<SqliteDb> db,
                                                   std::shared_ptr<core::errors::IErrorReporter> er)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
    pimpl_->errorReporter = std::move(er);
}

SqlitePropertyRepository::~SqlitePropertyRepository() = default;

namespace {

std::shared_ptr<Property> readProperty(persistence::StmtGuard& s) {
    auto p = std::make_shared<Property>();
    p->setId(s.columnText(0));
    p->rename(s.columnText(1));
    return p;
}

} // namespace

void SqlitePropertyRepository::addProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO properties (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText  (1, property->id());    stmt.bindText  (2, property->name());
    stmt.bindText(3, property->createdAt()); stmt.bindText(4, property->updatedAt());
    if (stmt.step() == SQLITE_DONE) {
        replacePropertyAliases(pimpl_->db->handle(), *property, {});
    }
}

std::vector<std::shared_ptr<Property>> SqlitePropertyRepository::getProperties() const {
    std::vector<std::shared_ptr<Property>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM properties ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto property = readProperty(stmt);
        property->setCreatedAt(stmt.columnText(2));
        property->setUpdatedAt(stmt.columnText(3));
        loadPropertyAliases(pimpl_->db->handle(), *property);
        out.push_back(std::move(property));
    }
    return out;
}

std::optional<std::shared_ptr<Property>> SqlitePropertyRepository::getPropertyById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM properties WHERE id = ?;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto property = readProperty(stmt);
    property->setCreatedAt(stmt.columnText(2));
    property->setUpdatedAt(stmt.columnText(3));
    loadPropertyAliases(pimpl_->db->handle(), *property);
    return property;
}

void SqlitePropertyRepository::removeProperty(const std::string& id) {
    if (id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM properties WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqlitePropertyRepository::updateProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id().empty()) return;
    const auto existing = loadPropertyAliasStats(pimpl_->db->handle(), property->id());
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE properties SET name = ?, created_at = COALESCE(NULLIF(created_at, ''), ?), updated_at = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, property->name());
    stmt.bindText(2, property->createdAt());
    stmt.bindText(3, property->updatedAt());
    stmt.bindText(4, property->id());
    if (stmt.step() == SQLITE_DONE) {
        replacePropertyAliases(pimpl_->db->handle(), *property, existing);
    }
}

void SqlitePropertyRepository::upsertProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id().empty()) return;
    updateProperty(property);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addProperty(property);
}

void SqlitePropertyRepository::clearProperties() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM property_aliases; DELETE FROM properties;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
