/**
 * @file persistence/include/persistence/Factory.h
 * @brief Declares factories for SQLite storage dependencies and repository bundles.
 */

#pragma once

#include <memory>
#include <string>

namespace core::ports::storage {
class IRegistry;
}

class SqliteDb;

/**
 * @brief Create a SQLite database wrapper for the given path.
 * @param dbPath SQLite database path.
 * @return Shared SQLite database wrapper.
 */
std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath);

/**
 * @brief Create the SQLite-backed registry for the given path.
 * @param dbPath SQLite database path.
 * @return Shared registry implementation.
 */
std::shared_ptr<core::ports::storage::IRegistry> createSqliteRegistry(const std::string& dbPath);

