/**
 * @file persistence/tests/unit/TestSqliteRegistry.cpp
 * @brief Tests for the SQLite-backed latest-path registry factory.
 */

#include <gtest/gtest.h>

#include "core/ports/infra/storage/IRegistry.h"
#include "persistence/Factory.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(SqliteRegistryTest, StartsEmptyAndPersistsTheLatestPath)
{
    TempDatabase dbFile("sqlite-registry");
    auto registry = createSqliteRegistry(dbFile.string());
    ASSERT_NE(registry, nullptr);

    EXPECT_FALSE(registry->getLatest().has_value());
    registry->setLatest("workspace-a.fr");
    ASSERT_TRUE(registry->getLatest().has_value());
    EXPECT_EQ(registry->getLatest().value(), "workspace-a.fr");
}

TEST(SqliteRegistryTest, ReopensTheLatestPathValueDeterministically)
{
    TempDatabase dbFile("sqlite-registry-reopen");
    {
        auto registry = createSqliteRegistry(dbFile.string());
        ASSERT_NE(registry, nullptr);
        registry->setLatest("workspace-b.fr");
    }

    auto reopened = createSqliteRegistry(dbFile.string());
    ASSERT_NE(reopened, nullptr);
    ASSERT_TRUE(reopened->getLatest().has_value());
    EXPECT_EQ(reopened->getLatest().value(), "workspace-b.fr");
}

} // namespace persistence::tests
