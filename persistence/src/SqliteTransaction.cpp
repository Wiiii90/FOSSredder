/**
 * @file persistence/src/SqliteTransaction.cpp
 * @brief Implements the lightweight SQLite transaction guard.
 */

#include "persistence/SqliteTransaction.h"

#include <stdexcept>
#include <string>

static void exec(sqlite3* db, const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        if (err) {
            sqlite3_free(err);
        }
        throw std::runtime_error(e);
    }
}

SqliteTransaction::SqliteTransaction(sqlite3* db)
    : db_(db)
{
    if (!db_) {
        throw std::runtime_error("db is null");
    }

    exec(db_, "BEGIN IMMEDIATE;");
}

SqliteTransaction::~SqliteTransaction()
{
    if (!db_) {
        return;
    }

    try {
        if (!committed_) {
            exec(db_, "ROLLBACK;");
        }
    } catch (...) {
    }
}

void SqliteTransaction::commit()
{
    if (committed_) {
        return;
    }

    exec(db_, "COMMIT;");
    committed_ = true;
}
