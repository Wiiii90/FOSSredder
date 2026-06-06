/**
 * @file persistence/src/repositories/SqliteTransactionRepository.cpp
 * @brief Implements the SQLite-backed transaction repository.
 */

#include "persistence/repositories/SqliteTransactionRepository.h"
#include "persistence/StmtGuard.h"
#include "core/domain/entities/Transaction.h"
#include "persistence/SqliteDb.h"
#include "persistence/SqliteTransaction.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqliteTransactionRepository::Impl {
    std::shared_ptr<SqliteDb> db;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

SqliteTransactionRepository::SqliteTransactionRepository(const std::string& dbPath)
    : SqliteTransactionRepository(std::make_shared<SqliteDb>(dbPath), nullptr) {}

SqliteTransactionRepository::SqliteTransactionRepository(const std::string& dbPath,
                                                         std::shared_ptr<core::errors::IErrorReporter> er)
    : SqliteTransactionRepository(std::make_shared<SqliteDb>(dbPath), std::move(er)) {}

SqliteTransactionRepository::SqliteTransactionRepository(std::shared_ptr<SqliteDb> db)
    : SqliteTransactionRepository(std::move(db), nullptr) {}

SqliteTransactionRepository::SqliteTransactionRepository(std::shared_ptr<SqliteDb> db,
                                                         std::shared_ptr<core::errors::IErrorReporter> er)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
    pimpl_->errorReporter = std::move(er);
}

SqliteTransactionRepository::~SqliteTransactionRepository() = default;

namespace {

constexpr auto kSelectTx =
    "SELECT id, name, booking_date, amount, statement_id,"
    " status, actor_id, allocatable, contract_id, valuta, created_at, updated_at FROM transactions";

void bindNullableText(persistence::StmtGuard& stmt, int column, const std::string& value) {
    if (value.empty()) {
        sqlite3_bind_null(stmt.get(), column);
        return;
    }
    stmt.bindText(column, value);
}

void loadTxProperties(sqlite3* db, Transaction& t) {
    persistence::StmtGuard s(db,
        "SELECT property_id FROM transaction_properties WHERE transaction_id = ?;");
    if (!s) return;
    s.bindText(1, t.id());
    while (s.step() == SQLITE_ROW) t.addPropertyId(s.columnText(0));
}

std::shared_ptr<Transaction> readTx(persistence::StmtGuard& s, sqlite3* db) {
    auto t = std::make_shared<Transaction>();
    t->setId(s.columnText(0));
    t->setName(s.columnText(1));
    t->setBookingDate(s.columnText(2));
    t->setAmount(s.columnDouble(3));
    t->setStatementId(s.columnText(4));
    t->setStatus(static_cast<Transaction::Status>(s.columnInt(5)));
    t->setActorId(s.columnText(6));
    t->setAllocatable(s.columnInt(7) != 0);
    t->setContractId(s.columnText(8));
    t->setValuta(s.columnText(9));
    t->setCreatedAt(s.columnText(10));
    t->setUpdatedAt(s.columnText(11));
    loadTxProperties(db, *t);
    return t;
}

void insertTxProperties(sqlite3* db, const Transaction& t) {
    persistence::StmtGuard s(db,
        "INSERT OR IGNORE INTO transaction_properties (transaction_id, property_id) VALUES (?, ?);");
    if (!s) return;
    for (const auto& pid : t.propertyIds()) {
        if (pid.empty()) continue;
        s.reset(); s.bindText(1, t.id()); s.bindText(2, pid); s.step();
    }
}

void deleteTxProperties(sqlite3* db, const std::string& txId) {
    persistence::StmtGuard s(db,
        "DELETE FROM transaction_properties WHERE transaction_id = ?;");
    if (!s) return;
    s.bindText(1, txId); s.step();
}

} // namespace

void SqliteTransactionRepository::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction || transaction->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO transactions"
        " (id, name, booking_date, amount, statement_id,"
        "  status, actor_id, allocatable, contract_id, valuta, created_at, updated_at)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText  (1, transaction->id());
    stmt.bindText  (2, transaction->name());
    stmt.bindText  (3, transaction->bookingDate());
    stmt.bindDouble(4, transaction->amount());
    bindNullableText(stmt, 5, transaction->statementId());
    stmt.bindInt   (6, static_cast<int>(transaction->status()));
    bindNullableText(stmt, 7, transaction->actorId());
    stmt.bindInt   (8, transaction->isAllocatable() ? 1 : 0);
    bindNullableText(stmt, 9, transaction->contractId());
    stmt.bindText  (10, transaction->valuta());
    stmt.bindText  (11, transaction->createdAt());
    stmt.bindText  (12, transaction->updatedAt());
    if (stmt.step() == SQLITE_DONE)
        insertTxProperties(pimpl_->db->handle(), *transaction);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactions() const {
    std::vector<std::shared_ptr<Transaction>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectTx) + " ORDER BY rowid;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW)
        out.push_back(readTx(stmt, pimpl_->db->handle()));
    return out;
}

std::optional<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectTx) + " WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    return readTx(stmt, pimpl_->db->handle());
}

void SqliteTransactionRepository::removeTransaction(const std::string& id) {
    if (id.empty()) return;
    deleteTxProperties(pimpl_->db->handle(), id);
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM transactions WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteTransactionRepository::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction || transaction->id().empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE transactions SET name=?, booking_date=?, amount=?,"
        " statement_id=?, status=?, actor_id=?, allocatable=?, contract_id=?, valuta=?, created_at=?, updated_at=?"
        " WHERE id=?;");
    if (!stmt) return;
    stmt.bindText  (1, transaction->name());
    stmt.bindText  (2, transaction->bookingDate());
    stmt.bindDouble(3, transaction->amount());
    bindNullableText(stmt, 4, transaction->statementId());
    stmt.bindInt   (5, static_cast<int>(transaction->status()));
    bindNullableText(stmt, 6, transaction->actorId());
    stmt.bindInt   (7, transaction->isAllocatable() ? 1 : 0);
    bindNullableText(stmt, 8, transaction->contractId());
    stmt.bindText  (9, transaction->valuta());
    stmt.bindText  (10, transaction->createdAt());
    stmt.bindText  (11, transaction->updatedAt());
    stmt.bindText  (12, transaction->id());
    stmt.step();
    deleteTxProperties(pimpl_->db->handle(), transaction->id());
    insertTxProperties(pimpl_->db->handle(), *transaction);
}

void SqliteTransactionRepository::upsertTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction || transaction->id().empty()) return;

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT INTO transactions (id, name, booking_date, amount, statement_id, status, actor_id, allocatable, contract_id, valuta, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name, booking_date = excluded.booking_date, amount = excluded.amount, "
        "statement_id = excluded.statement_id, status = excluded.status, actor_id = excluded.actor_id, allocatable = excluded.allocatable, "
        "contract_id = excluded.contract_id, valuta = excluded.valuta, created_at = excluded.created_at, updated_at = excluded.updated_at;"
    );
    if (!stmt) {
        return;
    }

    stmt.bindText  (1, transaction->id());
    stmt.bindText  (2, transaction->name());
    stmt.bindText  (3, transaction->bookingDate());
    stmt.bindDouble(4, transaction->amount());
    bindNullableText(stmt, 5, transaction->statementId());
    stmt.bindInt   (6, static_cast<int>(transaction->status()));
    bindNullableText(stmt, 7, transaction->actorId());
    stmt.bindInt   (8, transaction->isAllocatable() ? 1 : 0);
    bindNullableText(stmt, 9, transaction->contractId());
    stmt.bindText  (10, transaction->valuta());
    stmt.bindText  (11, transaction->createdAt());
    stmt.bindText  (12, transaction->updatedAt());

    stmt.step();

    deleteTxProperties(pimpl_->db->handle(), transaction->id());
    insertTxProperties(pimpl_->db->handle(), *transaction);
}

void SqliteTransactionRepository::clearTransactions() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(),
        "DELETE FROM transaction_properties; DELETE FROM transactions;",
        nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionsForContract(
    const std::string& contractId) const
{
    std::vector<std::shared_ptr<Transaction>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectTx) + " WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
    while (stmt.step() == SQLITE_ROW)
        out.push_back(readTx(stmt, pimpl_->db->handle()));
    return out;
}

void SqliteTransactionRepository::assignTransactionsToContract(
    const std::string& contractId, const std::vector<std::string>& transactionIds)
{
    if (transactionIds.empty()) return;
    std::vector<std::string> ids;
    ids.reserve(transactionIds.size());
    for (const auto& id : transactionIds) if (!id.empty()) ids.push_back(id);
    if (ids.empty()) return;

    SqliteTransaction tx(pimpl_->db->handle());

    std::string sql = "UPDATE transactions SET contract_id = ? WHERE id IN (";
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i) sql += ",";
        sql += "?";
    }
    sql += ");";

    persistence::StmtGuard stmt(pimpl_->db->handle(), sql.c_str());
    if (!stmt) return;

    stmt.bindText(1, contractId);
    for (size_t i = 0; i < ids.size(); ++i) {
        stmt.bindText(static_cast<int>(i + 2), ids[i]);
    }

    stmt.step();
    tx.commit();
}
