/**
 * @file persistence/src/repositories/SqliteStatementRepository.cpp
 * @brief Implements the SQLite-backed statement repository.
 */

#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Statement.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

namespace {

std::vector<std::string> loadTransactionIds(sqlite3* db, const std::string& statementId)
{
    std::vector<std::string> ids;
    if (!db || statementId.empty()) return ids;

    persistence::StmtGuard stmt(db,
        "SELECT transaction_id FROM statement_transactions WHERE statement_id = ? ORDER BY position, transaction_id;");
    if (!stmt) return ids;

    stmt.bindText(1, statementId);
    while (stmt.step() == SQLITE_ROW) {
        ids.push_back(stmt.columnText(0));
    }
    return ids;
}

void saveTransactionIds(sqlite3* db, const std::string& statementId, const std::vector<std::string>& transactionIds)
{
    if (!db || statementId.empty()) return;

    persistence::StmtGuard deleteStmt(db, "DELETE FROM statement_transactions WHERE statement_id = ?;");
    if (!deleteStmt) return;
    deleteStmt.bindText(1, statementId);
    deleteStmt.step();

    persistence::StmtGuard insertStmt(db,
        "INSERT OR IGNORE INTO statement_transactions (statement_id, transaction_id, position) VALUES (?, ?, ?);");
    if (!insertStmt) return;

    for (std::size_t i = 0; i < transactionIds.size(); ++i) {
        if (transactionIds[i].empty()) continue;
        insertStmt.reset();
        insertStmt.bindText(1, statementId);
        insertStmt.bindText(2, transactionIds[i]);
        insertStmt.bindInt(3, static_cast<int>(i));
        insertStmt.step();
    }
}

} // namespace

struct SqliteStatementRepository::Impl {
    std::shared_ptr<SqliteDb> db;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath)
    : SqliteStatementRepository(std::make_shared<SqliteDb>(dbPath), nullptr) {}

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath,
                                                     std::shared_ptr<core::errors::IErrorReporter> er)
    : SqliteStatementRepository(std::make_shared<SqliteDb>(dbPath), std::move(er)) {}

SqliteStatementRepository::SqliteStatementRepository(std::shared_ptr<SqliteDb> db)
    : SqliteStatementRepository(std::move(db), nullptr) {}

SqliteStatementRepository::SqliteStatementRepository(std::shared_ptr<SqliteDb> db,
                                                     std::shared_ptr<core::errors::IErrorReporter> er)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
    pimpl_->errorReporter = std::move(er);
}

SqliteStatementRepository::~SqliteStatementRepository() = default;

void SqliteStatementRepository::addStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO statements (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, statement->id()); stmt.bindText(2, statement->name());
    stmt.bindText(3, statement->createdAt()); stmt.bindText(4, statement->updatedAt());
    stmt.step();
    saveTransactionIds(pimpl_->db->handle(), statement->id(), statement->transactionIds());
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM statements ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto s = std::make_shared<Statement>();
        s->setId(stmt.columnText(0));
        s->rename(stmt.columnText(1));
        s->setCreatedAt(stmt.columnText(2));
        s->setUpdatedAt(stmt.columnText(3));
        s->setTransactionIds(loadTransactionIds(pimpl_->db->handle(), s->id()));
        out.push_back(std::move(s));
    }
    return out;
}

std::optional<std::shared_ptr<Statement>> SqliteStatementRepository::getStatementById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM statements WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto s = std::make_shared<Statement>();
    s->setId(stmt.columnText(0));
    s->rename(stmt.columnText(1));
    s->setCreatedAt(stmt.columnText(2));
    s->setUpdatedAt(stmt.columnText(3));
    s->setTransactionIds(loadTransactionIds(pimpl_->db->handle(), s->id()));
    return s;
}

void SqliteStatementRepository::removeStatement(const std::string& id) {
    if (id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statements WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteStatementRepository::updateStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE statements SET name = ?, created_at = ?, updated_at = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, statement->name()); stmt.bindText(2, statement->createdAt()); stmt.bindText(3, statement->updatedAt()); stmt.bindText(4, statement->id());
    stmt.step();
    saveTransactionIds(pimpl_->db->handle(), statement->id(), statement->transactionIds());
}

void SqliteStatementRepository::upsertStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id().empty()) return;
    updateStatement(statement);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addStatement(statement);
}

void SqliteStatementRepository::clearStatements() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM statements;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
