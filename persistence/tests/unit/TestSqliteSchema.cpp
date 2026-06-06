/**
 * @file persistence/tests/unit/TestSqliteSchema.cpp
 * @brief Tests for SQLite schema lifecycle helpers.
 */

#include <gtest/gtest.h>

#include <sqlite3.h>
#include <stdexcept>

#include "persistence/SqliteSchema.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(SqliteSchemaTest, EnsureRejectsNullHandles)
{
    EXPECT_THROW(SqliteSchema::ensure(nullptr), std::runtime_error);
}

TEST(SqliteSchemaTest, EnsureCreatesTheWorkspaceTablesAndForeignKeyMode)
{
    TempDatabase dbFile("sqlite-schema");
    sqlite3* handle = nullptr;
    ASSERT_EQ(sqlite3_open(dbFile.string().c_str(), &handle), SQLITE_OK);

    SqliteSchema::ensure(handle);

    EXPECT_EQ(pragmaInt(handle, "foreign_keys"), 1);
    EXPECT_TRUE(tableExists(handle, "actors"));
    EXPECT_TRUE(tableExists(handle, "actor_aliases"));
    EXPECT_TRUE(tableExists(handle, "contracts"));
    EXPECT_TRUE(tableExists(handle, "transactions"));
    EXPECT_TRUE(tableExists(handle, "statement_transactions"));
    EXPECT_TRUE(tableExists(handle, "statement_drafts"));
    EXPECT_TRUE(tableExists(handle, "statement_draft_transactions"));
    EXPECT_TRUE(tableExists(handle, "transaction_drafts"));
    EXPECT_TRUE(tableExists(handle, "transaction_draft_properties"));
    EXPECT_TRUE(tableExists(handle, "import_logs"));
    EXPECT_TRUE(tableExists(handle, "import_log_statement_drafts"));
    EXPECT_TRUE(tableExists(handle, "export_logs"));
    EXPECT_TRUE(tableExists(handle, "export_log_annuals"));
    EXPECT_TRUE(tableExists(handle, "export_log_analyses"));
    EXPECT_TRUE(tableExists(handle, "analyses"));
    EXPECT_TRUE(tableExists(handle, "analysis_adjustments"));
    EXPECT_TRUE(tableExists(handle, "annuals"));
    EXPECT_TRUE(tableExists(handle, "annual_analyses"));
    EXPECT_EQ(pragmaInt(handle, "user_version"), 1);

    sqlite3_close(handle);
}

} // namespace persistence::tests
