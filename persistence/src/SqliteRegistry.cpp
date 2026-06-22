/**
 * @file persistence/src/SqliteRegistry.cpp
 * @brief Implements the SQLite-backed latest-path registry factory.
 */

#include "core/ports/infra/storage/IRegistry.h"

#include <optional>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace {

class SqliteRegistry final : public core::ports::storage::IRegistry {
public:
  explicit SqliteRegistry(const std::string& dbPath)
      : db_(openDatabase(dbPath)) {}

  ~SqliteRegistry() override {
    if (db_) {
      sqlite3_close(db_);
    }
  }

  std::optional<std::string> getLatest() const override {
    if (!db_) {
      return std::nullopt;
    }

    constexpr const char* sql =
        "SELECT value FROM configs WHERE name = '__latest__' LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return std::nullopt;
    }

    std::optional<std::string> latest;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const auto* value = sqlite3_column_text(stmt, 0);
      if (value) {
        latest = std::string(reinterpret_cast<const char*>(value));
      }
    }

    sqlite3_finalize(stmt);
    return latest;
  }

  void setLatest(const std::string& path) override {
    if (!db_) {
      return;
    }

    constexpr const char* sql = "INSERT OR REPLACE INTO configs (name, value) "
                                "VALUES ('__latest__', ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return;
    }

    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }

private:
  sqlite3* db_ = nullptr;

  static sqlite3* openDatabase(const std::string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
      sqlite3_close(db);
      throw std::runtime_error("failed to open registry db");
    }

    ensureRegistryTable(db);
    return db;
  }

  static void ensureRegistryTable(sqlite3* db) {
    char* err = nullptr;
    if (sqlite3_exec(db,
                     "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY "
                     "KEY, value TEXT);",
                     nullptr, nullptr, &err) != SQLITE_OK) {
      const std::string message = err ? err : "failed to create registry table";
      if (err) {
        sqlite3_free(err);
      }
      throw std::runtime_error(message);
    }
  }
};

} // namespace

std::shared_ptr<core::ports::storage::IRegistry>
createSqliteRegistry(const std::string& dbPath) {
  return std::make_shared<SqliteRegistry>(dbPath);
}
