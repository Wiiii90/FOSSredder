/**
 * @file persistence/src/Factory.cpp
 * @brief Implements the SQLite database factory.
 */

#include "persistence/Factory.h"
#include "persistence/SqliteDb.h"

std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath) {
    return std::make_shared<SqliteDb>(dbPath);
}

