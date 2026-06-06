/**
 * @file persistence/include/persistence/SqliteTransaction.h
 * @brief Declares a lightweight SQLite transaction guard.
 */

#pragma once

#include <sqlite3.h>

class SqliteTransaction {
public:
    /**
     * @brief Start a SQLite transaction.
     * @param db SQLite database handle.
     */
    explicit SqliteTransaction(sqlite3* db);
    /**
     * @brief Roll back the transaction unless it was committed.
     */
    ~SqliteTransaction();

    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;

    /**
     * @brief Commit the transaction.
     */
    void commit();

private:
    sqlite3* db_;
    bool committed_ = false;
};
