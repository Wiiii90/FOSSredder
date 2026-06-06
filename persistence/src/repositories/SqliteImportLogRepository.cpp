/**
 * @file persistence/src/repositories/SqliteImportLogRepository.cpp
 * @brief Implements the SQLite-backed import log repository.
 */

#include "persistence/repositories/SqliteImportLogRepository.h"

#include "core/application/import/ImportLog.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

std::vector<std::string> loadStatementDraftIds(sqlite3* db, const std::string& importLogId)
{
    std::vector<std::string> ids;
    if (!db || importLogId.empty()) {
        return ids;
    }

    persistence::StmtGuard stmt(
        db,
        "SELECT statement_draft_id FROM import_log_statement_drafts WHERE import_log_id = ? ORDER BY position, statement_draft_id;");
    if (!stmt) {
        return ids;
    }

    stmt.bindText(1, importLogId);
    while (stmt.step() == SQLITE_ROW) {
        ids.push_back(stmt.columnText(0));
    }
    return ids;
}

void saveStatementDraftIds(sqlite3* db, const std::string& importLogId, const std::vector<std::string>& statementDraftIds)
{
    if (!db || importLogId.empty()) {
        return;
    }

    persistence::StmtGuard deleteStmt(db, "DELETE FROM import_log_statement_drafts WHERE import_log_id = ?;");
    if (!deleteStmt) {
        return;
    }
    deleteStmt.bindText(1, importLogId);
    deleteStmt.step();

    persistence::StmtGuard insertStmt(
        db,
        "INSERT OR IGNORE INTO import_log_statement_drafts (import_log_id, statement_draft_id, position) VALUES (?, ?, ?);");
    if (!insertStmt) {
        return;
    }

    for (std::size_t i = 0; i < statementDraftIds.size(); ++i) {
        const auto& statementDraftId = statementDraftIds[i];
        if (statementDraftId.empty()) {
            continue;
        }
        insertStmt.reset();
        insertStmt.bindText(1, importLogId);
        insertStmt.bindText(2, statementDraftId);
        insertStmt.bindInt(3, static_cast<int>(i));
        insertStmt.step();
    }
}

void writeImportLog(sqlite3* db,
                    const std::shared_ptr<core::application::importing::ImportLog>& log,
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
    stmt.bindText(3, log->type);
    stmt.bindText(4, log->file);
    stmt.bindText(5, log->status);
    stmt.bindText(6, log->message);
    stmt.bindInt(7, log->draftAttached ? 1 : 0);
    stmt.bindText(8, log->draftId);
    stmt.bindText(9, log->statementId);
    stmt.step();
}

} // namespace

struct SqliteImportLogRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteImportLogRepository::SqliteImportLogRepository(const std::string& dbPath)
    : SqliteImportLogRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteImportLogRepository::SqliteImportLogRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteImportLogRepository::~SqliteImportLogRepository() = default;

void SqliteImportLogRepository::addImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log)
{
    const auto db = pimpl_->db->handle();
    writeImportLog(
        db,
        log,
        "INSERT INTO import_logs (id, time, type, file, status, message, draft_attached, draft_id, statement_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (log) {
        saveStatementDraftIds(db, log->id, log->statementDraftIds);
    }
}

std::vector<std::shared_ptr<core::application::importing::ImportLog>> SqliteImportLogRepository::getImportLogs() const
{
    std::vector<std::shared_ptr<core::application::importing::ImportLog>> logs;
    const auto db = pimpl_->db->handle();
    persistence::StmtGuard stmt(
        db,
        "SELECT id, time, type, file, status, message, draft_attached, draft_id, statement_id FROM import_logs ORDER BY time DESC, id DESC;");
    if (!stmt) {
        return logs;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto log = std::make_shared<core::application::importing::ImportLog>();
        log->id = stmt.columnText(0);
        log->time = stmt.columnText(1);
        log->type = stmt.columnText(2);
        log->file = stmt.columnText(3);
        log->status = stmt.columnText(4);
        log->message = stmt.columnText(5);
        log->draftAttached = stmt.columnInt(6) != 0;
        log->draftId = stmt.columnText(7);
        log->statementId = stmt.columnText(8);
        log->statementDraftIds = loadStatementDraftIds(pimpl_->db->handle(), log->id);
        logs.push_back(std::move(log));
    }

    return logs;
}

std::optional<std::shared_ptr<core::application::importing::ImportLog>> SqliteImportLogRepository::getImportLogById(const std::string& id) const
{
    const auto db = pimpl_->db->handle();
    persistence::StmtGuard stmt(
        db,
        "SELECT id, time, type, file, status, message, draft_attached, draft_id, statement_id FROM import_logs WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto log = std::make_shared<core::application::importing::ImportLog>();
    log->id = stmt.columnText(0);
    log->time = stmt.columnText(1);
    log->type = stmt.columnText(2);
    log->file = stmt.columnText(3);
    log->status = stmt.columnText(4);
    log->message = stmt.columnText(5);
    log->draftAttached = stmt.columnInt(6) != 0;
    log->draftId = stmt.columnText(7);
    log->statementId = stmt.columnText(8);
    log->statementDraftIds = loadStatementDraftIds(pimpl_->db->handle(), log->id);
    return log;
}

void SqliteImportLogRepository::removeImportLog(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM import_logs WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteImportLogRepository::updateImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log)
{
    writeImportLog(
        pimpl_->db->handle(),
        log,
        "UPDATE import_logs SET time = ?2, type = ?3, file = ?4, status = ?5, message = ?6, draft_attached = ?7, draft_id = ?8, statement_id = ?9 WHERE id = ?1;");
    if (log) {
        saveStatementDraftIds(pimpl_->db->handle(), log->id, log->statementDraftIds);
    }
}

void SqliteImportLogRepository::upsertImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log)
{
    writeImportLog(
        pimpl_->db->handle(),
        log,
        "INSERT OR REPLACE INTO import_logs (id, time, type, file, status, message, draft_attached, draft_id, statement_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (log) {
        saveStatementDraftIds(pimpl_->db->handle(), log->id, log->statementDraftIds);
    }
}

void SqliteImportLogRepository::clearImportLogs()
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM import_logs;");
    if (!stmt) {
        return;
    }
    stmt.step();
}
