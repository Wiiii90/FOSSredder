/**
 * @file persistence/src/repositories/SqliteExportLogRepository.cpp
 * @brief Implements the SQLite-backed export log repository.
 */

#include "persistence/repositories/SqliteExportLogRepository.h"

#include "core/application/export/ExportLog.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

std::vector<std::string> loadIds(sqlite3* db, const char* sql, const std::string& exportLogId)
{
    std::vector<std::string> ids;
    if (!db || exportLogId.empty()) {
        return ids;
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return ids;
    }

    stmt.bindText(1, exportLogId);
    while (stmt.step() == SQLITE_ROW) {
        ids.push_back(stmt.columnText(0));
    }
    return ids;
}

void saveIds(sqlite3* db,
             const std::string& exportLogId,
             const std::vector<std::string>& ids,
             const char* deleteSql,
             const char* insertSql)
{
    if (!db || exportLogId.empty()) {
        return;
    }

    persistence::StmtGuard deleteStmt(db, deleteSql);
    if (!deleteStmt) {
        return;
    }
    deleteStmt.bindText(1, exportLogId);
    deleteStmt.step();

    persistence::StmtGuard insertStmt(db, insertSql);
    if (!insertStmt) {
        return;
    }

    for (std::size_t i = 0; i < ids.size(); ++i) {
        const auto& id = ids[i];
        if (id.empty()) {
            continue;
        }
        insertStmt.reset();
        insertStmt.bindText(1, exportLogId);
        insertStmt.bindText(2, id);
        insertStmt.bindInt(3, static_cast<int>(i));
        insertStmt.step();
    }
}

void writeExportLog(sqlite3* db,
                    const std::shared_ptr<core::application::exporting::ExportLog>& log,
                    const char* sql)
{
    if (!db || !log) {
        return;
    }

    if (log->id.empty()) {
        log->id = persistence::generateUuid();
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return;
    }

    stmt.bindText(1, log->id);
    stmt.bindText(2, log->time);
    stmt.bindText(3, log->targetPath);
    stmt.bindText(4, log->status);
    stmt.bindText(5, log->message);
    stmt.step();
}

} // namespace

struct SqliteExportLogRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteExportLogRepository::SqliteExportLogRepository(const std::string& dbPath)
    : SqliteExportLogRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteExportLogRepository::SqliteExportLogRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteExportLogRepository::~SqliteExportLogRepository() = default;

void SqliteExportLogRepository::addExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log)
{
    const auto db = pimpl_->db->handle();
    writeExportLog(
        db,
        log,
        "INSERT INTO export_logs (id, time, target_path, status, message) VALUES (?, ?, ?, ?, ?);");
    if (log) {
        saveIds(db, log->id, log->annualIds,
                "DELETE FROM export_log_annuals WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_annuals (export_log_id, annual_id, position) VALUES (?, ?, ?);");
        saveIds(db, log->id, log->analysisIds,
                "DELETE FROM export_log_analyses WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_analyses (export_log_id, analysis_id, position) VALUES (?, ?, ?);");
    }
}

std::vector<std::shared_ptr<core::application::exporting::ExportLog>> SqliteExportLogRepository::getExportLogs() const
{
    std::vector<std::shared_ptr<core::application::exporting::ExportLog>> logs;
    const auto db = pimpl_->db->handle();
    persistence::StmtGuard stmt(
        db,
        "SELECT id, time, target_path, status, message FROM export_logs ORDER BY time DESC, id DESC;");
    if (!stmt) {
        return logs;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto log = std::make_shared<core::application::exporting::ExportLog>();
        log->id = stmt.columnText(0);
        log->time = stmt.columnText(1);
        log->targetPath = stmt.columnText(2);
        log->status = stmt.columnText(3);
        log->message = stmt.columnText(4);
        log->annualIds = loadIds(db, "SELECT annual_id FROM export_log_annuals WHERE export_log_id = ? ORDER BY position, annual_id;", log->id);
        log->analysisIds = loadIds(db, "SELECT analysis_id FROM export_log_analyses WHERE export_log_id = ? ORDER BY position, analysis_id;", log->id);
        logs.push_back(std::move(log));
    }

    return logs;
}

std::optional<std::shared_ptr<core::application::exporting::ExportLog>> SqliteExportLogRepository::getExportLogById(const std::string& id) const
{
    const auto db = pimpl_->db->handle();
    persistence::StmtGuard stmt(
        db,
        "SELECT id, time, target_path, status, message FROM export_logs WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto log = std::make_shared<core::application::exporting::ExportLog>();
    log->id = stmt.columnText(0);
    log->time = stmt.columnText(1);
    log->targetPath = stmt.columnText(2);
    log->status = stmt.columnText(3);
    log->message = stmt.columnText(4);
    log->annualIds = loadIds(db, "SELECT annual_id FROM export_log_annuals WHERE export_log_id = ? ORDER BY position, annual_id;", log->id);
    log->analysisIds = loadIds(db, "SELECT analysis_id FROM export_log_analyses WHERE export_log_id = ? ORDER BY position, analysis_id;", log->id);
    return log;
}

void SqliteExportLogRepository::removeExportLog(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM export_logs WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteExportLogRepository::updateExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log)
{
    const auto db = pimpl_->db->handle();
    writeExportLog(
        db,
        log,
        "UPDATE export_logs SET time = ?2, target_path = ?3, status = ?4, message = ?5 WHERE id = ?1;");
    if (log) {
        saveIds(db, log->id, log->annualIds,
                "DELETE FROM export_log_annuals WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_annuals (export_log_id, annual_id, position) VALUES (?, ?, ?);");
        saveIds(db, log->id, log->analysisIds,
                "DELETE FROM export_log_analyses WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_analyses (export_log_id, analysis_id, position) VALUES (?, ?, ?);");
    }
}

void SqliteExportLogRepository::upsertExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log)
{
    const auto db = pimpl_->db->handle();
    writeExportLog(
        db,
        log,
        "INSERT OR REPLACE INTO export_logs (id, time, target_path, status, message) VALUES (?, ?, ?, ?, ?);");
    if (log) {
        saveIds(db, log->id, log->annualIds,
                "DELETE FROM export_log_annuals WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_annuals (export_log_id, annual_id, position) VALUES (?, ?, ?);");
        saveIds(db, log->id, log->analysisIds,
                "DELETE FROM export_log_analyses WHERE export_log_id = ?;",
                "INSERT OR IGNORE INTO export_log_analyses (export_log_id, analysis_id, position) VALUES (?, ?, ?);");
    }
}

void SqliteExportLogRepository::clearExportLogs()
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM export_logs;");
    if (!stmt) {
        return;
    }
    stmt.step();
}
