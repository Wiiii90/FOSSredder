/**
 * @file persistence/include/persistence/SqliteSchema.h
 * @brief Declares schema initialization and migration helpers for SQLite.
 */

#pragma once

#include <string>

struct sqlite3;

class SqliteSchema {
public:
    /**
     * @brief Ensure the database schema exists and is migrated.
     * @param db SQLite database handle.
     */
    static void ensure(sqlite3* db);
    /**
     * @brief Run schema migrations for the database.
     * @param db SQLite database handle.
     */
    static void migrate(sqlite3* db);

private:
    /**
     * @brief Read the SQLite user version.
     * @param db SQLite database handle.
     * @return Current schema version.
     */
    static int getUserVersion(sqlite3* db);
    /**
     * @brief Set the SQLite user version.
     * @param db SQLite database handle.
     * @param v Schema version.
     */
    static void setUserVersion(sqlite3* db, int v);
};
