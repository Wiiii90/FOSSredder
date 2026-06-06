/**
 * @file persistence/src/SqliteDb.cpp
 * @brief Implements the lightweight SQLite connection wrapper.
 */

#include "persistence/SqliteDb.h"

#include <sqlite3.h>
#include <stdexcept>
#include <string>

#include "persistence/SqliteSchema.h"

SqliteDb::SqliteDb(const std::string& path)
{
    const int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err;
        if (db_) {
            const char* msg = sqlite3_errmsg(db_);
            if (msg) {
                err = msg;
            }
            sqlite3_close(db_);
            db_ = nullptr;
        }

        const std::string message = std::string("Failed to open sqlite db '") + path + "'" +
                                     (err.empty() ? std::string() : std::string(": ") + err);
        throw std::runtime_error(message);
    }

    SqliteSchema::ensure(db_);
}

SqliteDb::~SqliteDb()
{
    if (db_) {
        sqlite3_close(db_);
    }
}
