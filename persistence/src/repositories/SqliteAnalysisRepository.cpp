/**
 * @file persistence/src/repositories/SqliteAnalysisRepository.cpp
 * @brief Implements the SQLite-backed analysis repository.
 */

#include "persistence/repositories/SqliteAnalysisRepository.h"

#include "core/domain/entities/Analysis.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct SqliteAnalysisRepository::Impl {
    std::shared_ptr<SqliteDb> db;
};

namespace {

std::unordered_map<std::string, double> loadAdjustments(sqlite3* db, const std::string& analysisId)
{
    std::unordered_map<std::string, double> out;
    if (!db || analysisId.empty()) {
        return out;
    }

    persistence::StmtGuard stmt(db,
        "SELECT adjustment_key, adjustment_value FROM analysis_adjustments WHERE analysis_id = ? ORDER BY position, adjustment_key;");
    if (!stmt) {
        return out;
    }

    stmt.bindText(1, analysisId);
    while (stmt.step() == SQLITE_ROW) {
        out.emplace(stmt.columnText(0), stmt.columnDouble(1));
    }

    return out;
}

void saveAdjustments(sqlite3* db, const Analysis& analysis)
{
    if (!db || analysis.id().empty()) {
        return;
    }

    persistence::StmtGuard deleteStmt(db, "DELETE FROM analysis_adjustments WHERE analysis_id = ?;");
    if (!deleteStmt) {
        return;
    }
    deleteStmt.bindText(1, analysis.id());
    deleteStmt.step();

    persistence::StmtGuard insertStmt(db,
        "INSERT OR REPLACE INTO analysis_adjustments (analysis_id, adjustment_key, adjustment_value, position) VALUES (?, ?, ?, ?);");
    if (!insertStmt) {
        return;
    }

    std::size_t position = 0;
    for (const auto& [key, value] : analysis.adjustments()) {
        if (key.empty()) {
            continue;
        }

        insertStmt.reset();
        insertStmt.bindText(1, analysis.id());
        insertStmt.bindText(2, key);
        insertStmt.bindDouble(3, value);
        insertStmt.bindInt(4, static_cast<int>(position++));
        insertStmt.step();
    }
}

std::shared_ptr<Analysis> readAnalysis(persistence::StmtGuard& stmt, sqlite3* db)
{
    auto analysis = std::make_shared<Analysis>();
    analysis->setId(stmt.columnText(0));
    analysis->rename(stmt.columnText(1));
    analysis->setType(stmt.columnText(2));
    analysis->setConfigJson(stmt.columnText(3));
    analysis->setFilterSpec(stmt.columnText(4));
    analysis->setExportFormat(stmt.columnText(5));
    analysis->setIncludeCalculationAdjustments(stmt.columnInt(6) != 0);
    analysis->setExportStateJson(stmt.columnText(7));
    analysis->setSnapshotTransactionsJson(stmt.columnText(8));
    analysis->setCreatedAt(stmt.columnText(9));
    analysis->setUpdatedAt(stmt.columnText(10));

    for (const auto& [key, value] : loadAdjustments(db, analysis->id())) {
        analysis->setAdjustment(key, value);
    }

    return analysis;
}

} // namespace

SqliteAnalysisRepository::SqliteAnalysisRepository(const std::string& dbPath)
    : SqliteAnalysisRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteAnalysisRepository::SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>())
{
    if (!db) {
        throw std::runtime_error("db is null");
    }

    pimpl_->db = std::move(db);
}

SqliteAnalysisRepository::~SqliteAnalysisRepository() = default;

void SqliteAnalysisRepository::addAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id().empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO analyses (id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, analysis->id());
    stmt.bindText(2, analysis->name());
    stmt.bindText(3, analysis->type());
    stmt.bindText(4, analysis->configJson());
    stmt.bindText(5, analysis->filterSpec());
    stmt.bindText(6, analysis->exportFormat());
    stmt.bindInt(7, analysis->includeCalculationAdjustments() ? 1 : 0);
    stmt.bindText(8, analysis->exportStateJson());
    stmt.bindText(9, analysis->snapshotTransactionsJson());
    stmt.bindText(10, analysis->createdAt());
    stmt.bindText(11, analysis->updatedAt());
    if (stmt.step() == SQLITE_DONE) {
        saveAdjustments(pimpl_->db->handle(), *analysis);
    }
}

std::vector<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalyses() const
{
    std::vector<std::shared_ptr<Analysis>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at FROM analyses ORDER BY id;");
    if (!stmt) {
        return out;
    }

    while (stmt.step() == SQLITE_ROW) {
        out.push_back(readAnalysis(stmt, pimpl_->db->handle()));
    }
    return out;
}

std::optional<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalysisById(const std::string& id) const
{
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at FROM analyses WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    return readAnalysis(stmt, pimpl_->db->handle());
}

void SqliteAnalysisRepository::removeAnalysis(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM analyses WHERE id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, id);
    stmt.step();
}

void SqliteAnalysisRepository::updateAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id().empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE analyses SET name=?, type=?, config_json=?, filter_spec=?, export_format=?, include_calc_adjustments=?, export_state_json=?, snapshot_transactions_json=?, created_at=?, updated_at=?"
        " WHERE id=?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, analysis->name());
    stmt.bindText(2, analysis->type());
    stmt.bindText(3, analysis->configJson());
    stmt.bindText(4, analysis->filterSpec());
    stmt.bindText(5, analysis->exportFormat());
    stmt.bindInt(6, analysis->includeCalculationAdjustments() ? 1 : 0);
    stmt.bindText(7, analysis->exportStateJson());
    stmt.bindText(8, analysis->snapshotTransactionsJson());
    stmt.bindText(9, analysis->createdAt());
    stmt.bindText(10, analysis->updatedAt());
    stmt.bindText(11, analysis->id());
    if (stmt.step() == SQLITE_DONE) {
        saveAdjustments(pimpl_->db->handle(), *analysis);
    }
}

void SqliteAnalysisRepository::upsertAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id().empty()) {
        return;
    }

    updateAnalysis(analysis);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) {
        addAnalysis(analysis);
    }
}

void SqliteAnalysisRepository::clearAnalyses()
{
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM analyses;", nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
}

sqlite3* SqliteAnalysisRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
