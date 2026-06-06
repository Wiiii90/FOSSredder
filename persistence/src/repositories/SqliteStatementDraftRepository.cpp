/**
 * @file persistence/src/repositories/SqliteStatementDraftRepository.cpp
 * @brief Implements the SQLite-backed statement draft repository.
 */

#include "persistence/repositories/SqliteStatementDraftRepository.h"

#include "core/application/import/draft/StatementDraft.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

std::vector<std::string> loadStatementDraftTransactionIds(sqlite3* db, const std::string& statementDraftId)
{
    std::vector<std::string> ids;
    if (!db || statementDraftId.empty()) {
        return ids;
    }

    persistence::StmtGuard stmt(db,
        "SELECT transaction_draft_id FROM statement_draft_transactions WHERE statement_draft_id = ? ORDER BY position, transaction_draft_id;");
    if (!stmt) {
        return ids;
    }

    stmt.bindText(1, statementDraftId);
    while (stmt.step() == SQLITE_ROW) {
        ids.push_back(stmt.columnText(0));
    }
    return ids;
}

void saveStatementDraftTransactionIds(sqlite3* db, const std::string& statementDraftId, const std::vector<std::string>& transactionIds)
{
    if (!db || statementDraftId.empty()) {
        return;
    }

    persistence::StmtGuard deleteStmt(db, "DELETE FROM statement_draft_transactions WHERE statement_draft_id = ?;");
    if (!deleteStmt) {
        return;
    }
    deleteStmt.bindText(1, statementDraftId);
    deleteStmt.step();

    persistence::StmtGuard insertStmt(db,
        "INSERT OR IGNORE INTO statement_draft_transactions (statement_draft_id, transaction_draft_id, position) VALUES (?, ?, ?);");
    if (!insertStmt) {
        return;
    }

    for (std::size_t i = 0; i < transactionIds.size(); ++i) {
        if (transactionIds[i].empty()) {
            continue;
        }
        insertStmt.reset();
        insertStmt.bindText(1, statementDraftId);
        insertStmt.bindText(2, transactionIds[i]);
        insertStmt.bindInt(3, static_cast<int>(i));
        insertStmt.step();
    }
}

void writeStatementDraft(sqlite3* db,
                         const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft,
                         const char* sql)
{
    if (!db || !draft) {
        return;
    }

    if (draft->id.empty()) {
        draft->id = persistence::generateUuid();
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return;
    }

    stmt.bindText(1, draft->id);
    stmt.bindText(2, draft->name);
    stmt.bindText(3, draft->createdAt);
    stmt.bindText(4, draft->updatedAt);
    stmt.step();
}

} // namespace

struct SqliteStatementDraftRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteStatementDraftRepository::SqliteStatementDraftRepository(const std::string& dbPath)
    : SqliteStatementDraftRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteStatementDraftRepository::SqliteStatementDraftRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteStatementDraftRepository::~SqliteStatementDraftRepository() = default;

void SqliteStatementDraftRepository::addStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeStatementDraft(
        db,
        draft,
        "INSERT INTO statement_drafts (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (draft) {
        saveStatementDraftTransactionIds(db, draft->id, draft->transactionIds);
    }
}

std::vector<std::shared_ptr<core::application::importing::draft::StatementDraft>> SqliteStatementDraftRepository::getStatementDrafts() const
{
    std::vector<std::shared_ptr<core::application::importing::draft::StatementDraft>> drafts;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "SELECT id, name, created_at, updated_at FROM statement_drafts ORDER BY id;");
    if (!stmt) {
        return drafts;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto draft = std::make_shared<core::application::importing::draft::StatementDraft>();
        draft->id = stmt.columnText(0);
        draft->name = stmt.columnText(1);
        draft->createdAt = stmt.columnText(2);
        draft->updatedAt = stmt.columnText(3);
        draft->transactionIds = loadStatementDraftTransactionIds(pimpl_->db->handle(), draft->id);
        drafts.push_back(std::move(draft));
    }

    return drafts;
}

std::optional<std::shared_ptr<core::application::importing::draft::StatementDraft>> SqliteStatementDraftRepository::getStatementDraftById(const std::string& id) const
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "SELECT id, name, created_at, updated_at FROM statement_drafts WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto draft = std::make_shared<core::application::importing::draft::StatementDraft>();
    draft->id = stmt.columnText(0);
    draft->name = stmt.columnText(1);
    draft->createdAt = stmt.columnText(2);
    draft->updatedAt = stmt.columnText(3);
    draft->transactionIds = loadStatementDraftTransactionIds(pimpl_->db->handle(), draft->id);
    return draft;
}

void SqliteStatementDraftRepository::removeStatementDraft(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statement_drafts WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteStatementDraftRepository::updateStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeStatementDraft(
        db,
        draft,
        "UPDATE statement_drafts SET name = ?2, created_at = ?3, updated_at = ?4 WHERE id = ?1;");
    if (draft) {
        saveStatementDraftTransactionIds(db, draft->id, draft->transactionIds);
    }
}

void SqliteStatementDraftRepository::upsertStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeStatementDraft(
        db,
        draft,
        "INSERT OR REPLACE INTO statement_drafts (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (draft) {
        saveStatementDraftTransactionIds(db, draft->id, draft->transactionIds);
    }
}

void SqliteStatementDraftRepository::clearStatementDrafts()
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statement_drafts;");
    if (!stmt) {
        return;
    }
    stmt.step();
}
