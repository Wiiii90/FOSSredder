/**
 * @file persistence/src/repositories/SqliteAnnualRepository.cpp
 * @brief Implements the SQLite-backed annual repository.
 */

#include "persistence/repositories/SqliteAnnualRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Annual.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

#include <vector>

namespace {

std::vector<std::string> loadAnalysisIds(sqlite3* db, const std::string& annualId)
{
    std::vector<std::string> out;
    if (!db || annualId.empty()) {
        return out;
    }

    persistence::StmtGuard stmt(db,
        "SELECT analysis_id FROM annual_analyses WHERE annual_id = ? ORDER BY position;");
    if (!stmt) {
        return out;
    }

    stmt.bindText(1, annualId);
    while (stmt.step() == SQLITE_ROW) {
        out.push_back(stmt.columnText(0));
    }
    return out;
}

constexpr auto kSelectAnnual =
    "SELECT id, name, year, created_at, updated_at FROM annuals";

void saveAnalysisIds(sqlite3* db, const std::string& annualId, const std::vector<std::string>& analysisIds)
{
    if (!db || annualId.empty()) {
        return;
    }

    persistence::StmtGuard deleteStmt(db, "DELETE FROM annual_analyses WHERE annual_id = ?;");
    if (!deleteStmt) {
        return;
    }
    deleteStmt.bindText(1, annualId);
    deleteStmt.step();

    persistence::StmtGuard insertStmt(db,
        "INSERT OR IGNORE INTO annual_analyses (annual_id, analysis_id, position) VALUES (?, ?, ?);");
    if (!insertStmt) {
        return;
    }

    for (std::size_t index = 0; index < analysisIds.size(); ++index) {
        if (analysisIds[index].empty()) {
            continue;
        }
        insertStmt.reset();
        insertStmt.bindText(1, annualId);
        insertStmt.bindText(2, analysisIds[index]);
        insertStmt.bindInt(3, static_cast<int>(index));
        insertStmt.step();
    }
}

std::shared_ptr<Annual> readAnnual(sqlite3* db, persistence::StmtGuard& s) {
    auto a = std::make_shared<Annual>();
    a->setId(s.columnText(0));
    a->rename(s.columnText(1));
    a->setYear(s.columnInt(2));
    a->setCreatedAt(s.columnText(3));
    a->setUpdatedAt(s.columnText(4));
    a->setAnalysisIds(loadAnalysisIds(db, a->id()));
    return a;
}

} // namespace

struct SqliteAnnualRepository::Impl
{
    std::shared_ptr<SqliteDb> db;
};

SqliteAnnualRepository::SqliteAnnualRepository(const std::string& dbPath)
    : SqliteAnnualRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteAnnualRepository::SqliteAnnualRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>())
{
    if (!db) {
        throw std::runtime_error("db is null");
    }

    pimpl_->db = std::move(db);
}

SqliteAnnualRepository::~SqliteAnnualRepository() = default;

void SqliteAnnualRepository::addAnnual(const std::shared_ptr<Annual>& annual)
{
    if (!annual || annual->id().empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO annuals"
        " (id, name, year, created_at, updated_at)"
        " VALUES (?, ?, ?, ?, ?);");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, annual->id());
    stmt.bindText(2, annual->name());
    stmt.bindInt (3, annual->year());
    stmt.bindText(4, annual->createdAt());
    stmt.bindText(5, annual->updatedAt());
    stmt.step();
    saveAnalysisIds(pimpl_->db->handle(), annual->id(), annual->analysisIds());
}

std::vector<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnuals() const
{
    std::vector<std::shared_ptr<Annual>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectAnnual) + " ORDER BY year;");
    if (!stmt) {
        return out;
    }

    while (stmt.step() == SQLITE_ROW) {
        out.push_back(readAnnual(pimpl_->db->handle(), stmt));
    }
    return out;
}

std::optional<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnualById(const std::string& id) const
{
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectAnnual) + " WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }
    return readAnnual(pimpl_->db->handle(), stmt);
}

void SqliteAnnualRepository::removeAnnual(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM annuals WHERE id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, id);
    stmt.step();
}

void SqliteAnnualRepository::updateAnnual(const std::shared_ptr<Annual>& annual)
{
    if (!annual || annual->id().empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE annuals SET name=?, year=?, created_at=?, updated_at=? WHERE id=?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, annual->name());
    stmt.bindInt(2, annual->year());
    stmt.bindText(3, annual->createdAt());
    stmt.bindText(4, annual->updatedAt());
    stmt.bindText(5, annual->id());
    stmt.step();
    saveAnalysisIds(pimpl_->db->handle(), annual->id(), annual->analysisIds());
}

void SqliteAnnualRepository::upsertAnnual(const std::shared_ptr<Annual>& annual)
{
    if (!annual || annual->id().empty()) {
        return;
    }

    updateAnnual(annual);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) {
        addAnnual(annual);
    }
}

void SqliteAnnualRepository::clearAnnuals()
{
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM annuals;", nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
}

sqlite3* SqliteAnnualRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
