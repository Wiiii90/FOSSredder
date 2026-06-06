/**
 * @file persistence/include/persistence/SqliteDb.h
 * @brief Declares the lightweight SQLite connection wrapper.
 */

#pragma once

#include <memory>
#include <string>

#include "persistence/ISqlConnectionProvider.h"

struct sqlite3;

class SqliteDb : public ISqlConnectionProvider {
public:
    /**
     * @brief Open a SQLite database at the given path.
     * @param path SQLite database path.
     */
    explicit SqliteDb(const std::string& path);
    /**
     * @brief Close the database connection.
     */
    ~SqliteDb();

    SqliteDb(const SqliteDb&) = delete;
    SqliteDb& operator=(const SqliteDb&) = delete;

    /**
     * @brief Get the raw SQLite handle.
     * @return Raw sqlite3 handle.
     */
    sqlite3* handle() const noexcept { return db_; }

    /**
     * @brief Get the raw SQLite handle.
     * @return Raw sqlite3 handle.
     */
    sqlite3* sqliteHandle() const noexcept override { return db_; }

private:
    sqlite3* db_ = nullptr;
};
