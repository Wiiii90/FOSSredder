/**
 * @file persistence/include/persistence/StmtGuard.h
 * @brief RAII wrapper for sqlite3_stmt* that auto-finalizes on destruction.
 */
#pragma once

#include <sqlite3.h>
#include <string>

namespace persistence {

class StmtGuard {
public:
  StmtGuard(sqlite3* db, const char* sql) noexcept {
    sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr);
  }

  StmtGuard(sqlite3* db, const std::string& sql) noexcept {
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr);
  }

  ~StmtGuard() noexcept {
    if (stmt_)
      sqlite3_finalize(stmt_);
  }

  StmtGuard(const StmtGuard&) = delete;
  StmtGuard& operator=(const StmtGuard&) = delete;

  explicit operator bool() const noexcept {
    return stmt_ != nullptr;
  }
  sqlite3_stmt* get() const noexcept {
    return stmt_;
  }

  int step() noexcept {
    return sqlite3_step(stmt_);
  }
  void reset() noexcept {
    sqlite3_reset(stmt_);
  }

  void bindText(int col, const std::string& v) noexcept {
    sqlite3_bind_text(stmt_, col, v.c_str(), -1, SQLITE_TRANSIENT);
  }
  void bindInt(int col, int v) noexcept {
    sqlite3_bind_int(stmt_, col, v);
  }
  void bindInt64(int col, int64_t v) noexcept {
    sqlite3_bind_int64(stmt_, col, v);
  }
  void bindDouble(int col, double v) noexcept {
    sqlite3_bind_double(stmt_, col, v);
  }

  std::string columnText(int col) const noexcept {
    const auto* t = sqlite3_column_text(stmt_, col);
    return t ? reinterpret_cast<const char*>(t) : std::string{};
  }
  int columnInt(int col) const noexcept {
    return sqlite3_column_int(stmt_, col);
  }
  int64_t columnInt64(int col) const noexcept {
    return sqlite3_column_int64(stmt_, col);
  }
  double columnDouble(int col) const noexcept {
    return sqlite3_column_double(stmt_, col);
  }

private:
  sqlite3_stmt* stmt_ = nullptr;
};

} // namespace persistence
