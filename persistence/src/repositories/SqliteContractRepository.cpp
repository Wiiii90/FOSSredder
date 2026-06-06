/**
 * @file persistence/src/repositories/SqliteContractRepository.cpp
 * @brief Implements the SQLite-backed contract repository.
 */

#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Contract.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <stdexcept>

struct SqliteContractRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteContractRepository::SqliteContractRepository(const std::string& dbPath)
    : SqliteContractRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteContractRepository::SqliteContractRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteContractRepository::~SqliteContractRepository() = default;

namespace {

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

std::vector<core::domain::Alias> collectAliases(const Contract& contract)
{
    std::vector<core::domain::Alias> out;
    out.reserve(contract.aliases().size());
    for (const auto& alias : contract.aliases()) {
        if (alias.value().empty()) continue;
        auto value = alias;
        if (value.hitCount() < 1) value.setHitCount(1);
        out.push_back(std::move(value));
    }
    return out;
}

std::unordered_map<std::string, AliasStats> loadContractAliasStats(sqlite3* db, const std::string& contractId) {
    std::unordered_map<std::string, AliasStats> out;
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM contract_aliases WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
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

void loadContractAliases(sqlite3* db, Contract& contract) {
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM contract_aliases WHERE contract_id = ? ORDER BY hit_count DESC, last_used_at DESC, alias ASC;");
    if (!stmt) return;
    stmt.bindText(1, contract.id());
    std::vector<core::domain::Alias> aliases;
    while (stmt.step() == SQLITE_ROW) {
        aliases.push_back(makeAlias(stmt.columnText(0),
                                    stmt.columnInt(1),
                                    stmt.columnText(2),
                                    stmt.columnText(3),
                                    stmt.columnText(4)));
    }
    contract.setAliases(std::move(aliases));
}

void deleteContractAliases(sqlite3* db, const std::string& contractId) {
    persistence::StmtGuard stmt(db, "DELETE FROM contract_aliases WHERE contract_id = ?;");
    if (!stmt) return;
    stmt.bindText(1, contractId);
    stmt.step();
}

void replaceContractAliases(sqlite3* db, const Contract& contract, const std::unordered_map<std::string, AliasStats>& existing) {
    if (contract.id().empty()) return;
    deleteContractAliases(db, contract.id());

    persistence::StmtGuard stmt(db,
        "INSERT INTO contract_aliases (contract_id, alias, hit_count, created_at, updated_at, last_used_at) VALUES (?, ?, ?, COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP));");
    if (!stmt) return;

    std::unordered_set<std::string> seen;
    for (const auto& alias : collectAliases(contract)) {
        if (alias.value().empty() || !seen.insert(alias.value()).second) continue;
        const auto it = existing.find(alias.value());
        const int hitCount = it == existing.end() ? std::max(1, alias.hitCount()) : std::max(1, it->second.hitCount);
        const std::string createdAt = it == existing.end() || it->second.createdAt.empty() ? alias.createdAt() : it->second.createdAt;
        const std::string updatedAt = it == existing.end() || it->second.updatedAt.empty() ? alias.updatedAt() : it->second.updatedAt;
        const std::string lastUsedAt = it == existing.end() || it->second.lastUsedAt.empty() ? alias.lastUsedAt() : it->second.lastUsedAt;
        stmt.reset();
        stmt.bindText(1, contract.id());
        stmt.bindText(2, alias.value());
        stmt.bindInt(3, hitCount);
        stmt.bindText(4, createdAt);
        stmt.bindText(5, updatedAt);
        stmt.bindText(6, lastUsedAt);
        stmt.step();
    }
}

void insertContractRelations(sqlite3* db, const std::string& contractId, const Contract& c) {
    {
        persistence::StmtGuard s(db,
            "INSERT OR IGNORE INTO contract_actors (contract_id, actor_id) VALUES (?, ?);");
        if (s) for (const auto& id : c.actorIds()) {
            if (id.empty()) continue;
            s.reset(); s.bindText(1, contractId); s.bindText(2, id); s.step();
        }
    }
    {
        persistence::StmtGuard s(db,
            "INSERT OR IGNORE INTO contract_properties (contract_id, property_id) VALUES (?, ?);");
        if (s) for (const auto& id : c.propertyIds()) {
            if (id.empty()) continue;
            s.reset(); s.bindText(1, contractId); s.bindText(2, id); s.step();
        }
    }
}

void deleteContractRelations(sqlite3* db, const std::string& contractId) {
    persistence::StmtGuard a(db, "DELETE FROM contract_actors WHERE contract_id = ?;");
    if (a) { a.bindText(1, contractId); a.step(); }
    persistence::StmtGuard p(db, "DELETE FROM contract_properties WHERE contract_id = ?;");
    if (p) { p.bindText(1, contractId); p.step(); }
    deleteContractAliases(db, contractId);
}

constexpr auto kSelectContract =
    "SELECT id, name, type, allocatable_mode FROM contracts";

std::shared_ptr<Contract> readContractRow(persistence::StmtGuard& s) {
    auto c = std::make_shared<Contract>();
    c->setId(s.columnText(0));
    c->rename(s.columnText(1));
    c->setType(s.columnText(2));
    c->setAllocatableMode(s.columnText(3));
    return c;
}

void loadContractRelations(sqlite3* db, Contract& c) {
    {
        persistence::StmtGuard s(db,
            "SELECT actor_id FROM contract_actors WHERE contract_id = ?;");
        if (s) { s.bindText(1, c.id()); while (s.step() == SQLITE_ROW) c.addActorId(s.columnText(0)); }
    }
    {
        persistence::StmtGuard s(db,
            "SELECT property_id FROM contract_properties WHERE contract_id = ?;");
        if (s) { s.bindText(1, c.id()); while (s.step() == SQLITE_ROW) c.addPropertyId(s.columnText(0)); }
    }
}

} // namespace

void SqliteContractRepository::addContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO contracts (id, name, type, allocatable_mode, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, contract->id());    stmt.bindText(2, contract->name());
    stmt.bindText(3, contract->type());
    stmt.bindText(4, contract->allocatableMode());
    stmt.bindText(5, contract->createdAt());
    stmt.bindText(6, contract->updatedAt());
    if (stmt.step() == SQLITE_DONE) {
        replaceContractAliases(pimpl_->db->handle(), *contract, {});
        insertContractRelations(pimpl_->db->handle(), contract->id(), *contract);
    }
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContracts() const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, allocatable_mode, created_at, updated_at FROM contracts ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto c = readContractRow(stmt);
        c->setCreatedAt(stmt.columnText(4));
        c->setUpdatedAt(stmt.columnText(5));
        loadContractAliases(pimpl_->db->handle(), *c);
        loadContractRelations(pimpl_->db->handle(), *c);
        out.push_back(std::move(c));
    }
    return out;
}

std::optional<std::shared_ptr<Contract>> SqliteContractRepository::getContractById(const std::string& id) const {
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, allocatable_mode, created_at, updated_at FROM contracts WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto c = readContractRow(stmt);
    c->setCreatedAt(stmt.columnText(4));
    c->setUpdatedAt(stmt.columnText(5));
    loadContractAliases(pimpl_->db->handle(), *c);
    loadContractRelations(pimpl_->db->handle(), *c);
    return c;
}

void SqliteContractRepository::removeContract(const std::string& id) {
    deleteContractRelations(pimpl_->db->handle(), id);
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM contracts WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteContractRepository::updateContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id().empty()) return;
    const auto existing = loadContractAliasStats(pimpl_->db->handle(), contract->id());
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE contracts SET name = ?, type = ?, allocatable_mode = ?, created_at = COALESCE(NULLIF(created_at, ''), ?), updated_at = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, contract->name());
    stmt.bindText(2, contract->type());
    stmt.bindText(3, contract->allocatableMode());
    stmt.bindText(4, contract->createdAt());
    stmt.bindText(5, contract->updatedAt());
    stmt.bindText(6, contract->id());
    if (stmt.step() != SQLITE_DONE) return;
    deleteContractRelations(pimpl_->db->handle(), contract->id());
    insertContractRelations(pimpl_->db->handle(), contract->id(), *contract);
    replaceContractAliases(pimpl_->db->handle(), *contract, existing);
}

void SqliteContractRepository::upsertContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id().empty()) return;
    updateContract(contract);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addContract(contract);
}

void SqliteContractRepository::clearContracts() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(),
        "DELETE FROM contract_aliases; DELETE FROM contract_actors; DELETE FROM contract_properties; DELETE FROM contracts;",
        nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForActor(const std::string& actorId) const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard ids(pimpl_->db->handle(),
        "SELECT contract_id FROM contract_actors WHERE actor_id = ?;");
    if (!ids) return out;
    ids.bindText(1, actorId);
    while (ids.step() == SQLITE_ROW) {
        auto c = getContractById(ids.columnText(0));
        if (c) out.push_back(*c);
    }
    return out;
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForProperty(const std::string& propertyId) const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard ids(pimpl_->db->handle(),
        "SELECT contract_id FROM contract_properties WHERE property_id = ?;");
    if (!ids) return out;
    ids.bindText(1, propertyId);
    while (ids.step() == SQLITE_ROW) {
        auto c = getContractById(ids.columnText(0));
        if (c) out.push_back(*c);
    }
    return out;
}

std::vector<std::string> SqliteContractRepository::getActorIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT actor_id FROM contract_actors WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
    while (stmt.step() == SQLITE_ROW) out.push_back(stmt.columnText(0));
    return out;
}

std::vector<std::string> SqliteContractRepository::getPropertyIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT property_id FROM contract_properties WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
    while (stmt.step() == SQLITE_ROW) out.push_back(stmt.columnText(0));
    return out;
}
