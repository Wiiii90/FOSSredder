/**
 * @file persistence/tests/unit/TestSqliteDb.cpp
 * @brief Tests for the SQLite database wrapper lifecycle.
 */

#include <gtest/gtest.h>

#include "persistence/SqliteDb.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(SqliteDbTest, OpensTheDatabaseAndInitializesTheSchema)
{
    TempDatabase dbFile("sqlite-db");
    SqliteDb db(dbFile.string());

    ASSERT_NE(db.handle(), nullptr);
    EXPECT_EQ(pragmaInt(db.handle(), "foreign_keys"), 1);
    EXPECT_TRUE(tableExists(db.handle(), "actors"));
    EXPECT_TRUE(tableExists(db.handle(), "configs"));
    EXPECT_TRUE(tableExists(db.handle(), "transactions"));
    EXPECT_GE(pragmaInt(db.handle(), "user_version"), 1);
}

TEST(SqliteDbTest, ReopensAnExistingDatabaseSafely)
{
    TempDatabase dbFile("sqlite-db-reopen");

    {
        SqliteDb first(dbFile.string());
        ASSERT_NE(first.handle(), nullptr);
        EXPECT_TRUE(tableExists(first.handle(), "statements"));
    }

    SqliteDb second(dbFile.string());
    ASSERT_NE(second.handle(), nullptr);
    EXPECT_TRUE(tableExists(second.handle(), "statements"));
}

} // namespace persistence::tests
