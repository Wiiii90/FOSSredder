/**
 * @file persistence/src/repositories/SqliteActorRepository.cpp
 * @brief Implements the SQLite-backed actor repository.
 */

#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Actor.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <stdexcept>

struct SqliteActorRepository::Impl
{
    std::shared_ptr<SqliteDb> db;
};

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

std::vector<core::domain::Alias> collectAliases(const core::domain::Actor& actor)
{
    std::vector<core::domain::Alias> out;
    out.reserve(actor.aliases().size());
    for (const auto& alias : actor.aliases()) {
        if (alias.value().empty()) continue;
        auto value = alias;
        if (value.hitCount() < 1) value.setHitCount(1);
        out.push_back(std::move(value));
    }
    return out;
}

std::unordered_map<std::string, AliasStats> loadActorAliasStats(sqlite3* db, const std::string& actorId)
{
    std::unordered_map<std::string, AliasStats> out;
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM actor_aliases WHERE actor_id = ?;");
    if (!stmt) return out;

    stmt.bindText(1, actorId);
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

void loadActorAliases(sqlite3* db, core::domain::Actor& actor)
{
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM actor_aliases WHERE actor_id = ? ORDER BY hit_count DESC, last_used_at DESC, alias ASC;");
    if (!stmt) return;

    stmt.bindText(1, actor.id());
    std::vector<core::domain::Alias> aliases;
    while (stmt.step() == SQLITE_ROW) {
        aliases.push_back(makeAlias(stmt.columnText(0),
                                    stmt.columnInt(1),
                                    stmt.columnText(2),
                                    stmt.columnText(3),
                                    stmt.columnText(4)));
    }
    actor.setAliases(std::move(aliases));
}

void deleteActorAliases(sqlite3* db, const std::string& actorId)
{
    persistence::StmtGuard stmt(db, "DELETE FROM actor_aliases WHERE actor_id = ?;");
    if (!stmt) return;

    stmt.bindText(1, actorId);
    stmt.step();
}

void replaceActorAliases(sqlite3* db,
                         const core::domain::Actor& actor,
                         const std::unordered_map<std::string, AliasStats>& existing)
{
    if (actor.id().empty()) return;

    deleteActorAliases(db, actor.id());

    persistence::StmtGuard stmt(db,
        "INSERT INTO actor_aliases (actor_id, alias, hit_count, created_at, updated_at, last_used_at) VALUES (?, ?, ?, COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP));");
    if (!stmt) return;

    std::unordered_set<std::string> seen;
    for (const auto& alias : collectAliases(actor)) {
        if (alias.value().empty() || !seen.insert(alias.value()).second) continue;

        const auto it = existing.find(alias.value());
        const int hitCount = it == existing.end() ? std::max(1, alias.hitCount()) : std::max(1, it->second.hitCount);
        const std::string createdAt = it == existing.end() || it->second.createdAt.empty() ? alias.createdAt() : it->second.createdAt;
        const std::string updatedAt = it == existing.end() || it->second.updatedAt.empty() ? alias.updatedAt() : it->second.updatedAt;
        const std::string lastUsedAt = it == existing.end() || it->second.lastUsedAt.empty() ? alias.lastUsedAt() : it->second.lastUsedAt;
        stmt.reset();
        stmt.bindText(1, actor.id());
        stmt.bindText(2, alias.value());
        stmt.bindInt(3, hitCount);
        stmt.bindText(4, createdAt);
        stmt.bindText(5, updatedAt);
        stmt.bindText(6, lastUsedAt);
        stmt.step();
    }
}

} // namespace

SqliteActorRepository::SqliteActorRepository(const std::string& dbPath)
    : SqliteActorRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteActorRepository::SqliteActorRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>())
{
    if (!db) {
        throw std::runtime_error("db is null");
    }

    pimpl_->db = std::move(db);
}

SqliteActorRepository::~SqliteActorRepository() = default;

void SqliteActorRepository::addActor(const std::shared_ptr<Actor>& actor)
{
    if (!actor || actor->id().empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO actors (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, actor->id());
    stmt.bindText(2, actor->name());
    stmt.bindText(3, actor->createdAt());
    stmt.bindText(4, actor->updatedAt());
    if (stmt.step() == SQLITE_DONE) {
        replaceActorAliases(pimpl_->db->handle(), *actor, {});
    }
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const
{
    std::vector<std::shared_ptr<Actor>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM actors ORDER BY id;");
    if (!stmt) {
        return out;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto a = std::make_shared<Actor>();
        a->setId(stmt.columnText(0));
        a->rename(stmt.columnText(1));
        a->setCreatedAt(stmt.columnText(2));
        a->setUpdatedAt(stmt.columnText(3));
        loadActorAliases(pimpl_->db->handle(), *a);
        out.push_back(std::move(a));
    }
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const
{
    if (id.empty()) {
        return std::nullopt;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM actors WHERE id = ?;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto a = std::make_shared<Actor>();
    a->setId(stmt.columnText(0));
    a->rename(stmt.columnText(1));
    a->setCreatedAt(stmt.columnText(2));
    a->setUpdatedAt(stmt.columnText(3));
    loadActorAliases(pimpl_->db->handle(), *a);
    return a;
}

void SqliteActorRepository::removeActor(const std::string& id)
{
    if (id.empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM actors WHERE id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, id);
    stmt.step();
}

void SqliteActorRepository::updateActor(const std::shared_ptr<Actor>& actor)
{
    if (!actor || actor->id().empty()) {
        return;
    }

    const auto existing = loadActorAliasStats(pimpl_->db->handle(), actor->id());
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE actors SET name = ?, created_at = COALESCE(NULLIF(created_at, ''), ?), updated_at = ? WHERE id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, actor->name());
    stmt.bindText(2, actor->createdAt());
    stmt.bindText(3, actor->updatedAt());
    stmt.bindText(4, actor->id());
    if (stmt.step() == SQLITE_DONE) {
        replaceActorAliases(pimpl_->db->handle(), *actor, existing);
    }
}

void SqliteActorRepository::upsertActor(const std::shared_ptr<Actor>& actor)
{
    if (!actor || actor->id().empty()) {
        return;
    }

    updateActor(actor);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) {
        addActor(actor);
    }
}

void SqliteActorRepository::clearActors()
{
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM actor_aliases; DELETE FROM actors;", nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
}

sqlite3* SqliteActorRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
