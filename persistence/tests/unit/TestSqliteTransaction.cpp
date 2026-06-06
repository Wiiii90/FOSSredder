/**
 * @file persistence/tests/unit/TestSqliteTransaction.cpp
 * @brief Tests for the SQLite transaction guard.
 */

#include <gtest/gtest.h>

#include <sqlite3.h>
#include <stdexcept>

#include "persistence/SqliteDb.h"
#include "persistence/SqliteTransaction.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

namespace {

void execSql(sqlite3* db, const char* sql)
{
    char* error = nullptr;
    ASSERT_EQ(sqlite3_exec(db, sql, nullptr, nullptr, &error), SQLITE_OK) << (error ? error : "");
    if (error) {
        sqlite3_free(error);
    }
}

int countRows(sqlite3* db, const char* tableName)
{
    return rowCount(db, tableName);
}

} // namespace

TEST(SqliteTransactionTest, CommitsPersistedWrites)
{
    TempDatabase dbFile("sqlite-transaction-commit");
    SqliteDb db(dbFile.string());
    execSql(db.handle(), "CREATE TABLE IF NOT EXISTS tx_guard_rows (id TEXT PRIMARY KEY);");

    {
        SqliteTransaction transaction(db.handle());
        execSql(db.handle(), "INSERT INTO tx_guard_rows (id) VALUES ('row-1');");
        transaction.commit();
    }

    EXPECT_EQ(countRows(db.handle(), "tx_guard_rows"), 1);
}

TEST(SqliteTransactionTest, RollsBackUncommittedWrites)
{
    TempDatabase dbFile("sqlite-transaction-rollback");
    SqliteDb db(dbFile.string());
    execSql(db.handle(), "CREATE TABLE IF NOT EXISTS tx_guard_rows (id TEXT PRIMARY KEY);");

    {
        SqliteTransaction transaction(db.handle());
        execSql(db.handle(), "INSERT INTO tx_guard_rows (id) VALUES ('row-1');");
        (void)transaction;
    }

    EXPECT_EQ(countRows(db.handle(), "tx_guard_rows"), 0);
}

TEST(SqliteTransactionTest, DoubleCommitIsHarmless)
{
    TempDatabase dbFile("sqlite-transaction-double-commit");
    SqliteDb db(dbFile.string());
    execSql(db.handle(), "CREATE TABLE IF NOT EXISTS tx_guard_rows (id TEXT PRIMARY KEY);");

    SqliteTransaction transaction(db.handle());
    execSql(db.handle(), "INSERT INTO tx_guard_rows (id) VALUES ('row-1');");
    transaction.commit();
    EXPECT_NO_THROW(transaction.commit());
    EXPECT_EQ(countRows(db.handle(), "tx_guard_rows"), 1);
}

TEST(SqliteTransactionTest, ConstructorRejectsNullHandles)
{
    EXPECT_THROW(SqliteTransaction(nullptr), std::runtime_error);
}

} // namespace persistence::tests
