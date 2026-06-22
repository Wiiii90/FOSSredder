/**
 * @file persistence/tests/unit/TestFactory.cpp
 * @brief Tests for SQLite storage factories.
 */

#include <gtest/gtest.h>

#include "core/ports/infra/storage/IRegistry.h"
#include "persistence/Factory.h"
#include "persistence/SqliteDb.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(FactoryTest, CreatesAUsableDatabaseWrapper) {
  TempDatabase dbFile("factory-db");
  auto db = createSqliteDb(dbFile.string());
  ASSERT_NE(db, nullptr);
  ASSERT_NE(db->handle(), nullptr);
  EXPECT_GE(pragmaInt(db->handle(), "user_version"), 1);
}

TEST(FactoryTest, CreatesAUsableRegistry) {
  TempDatabase dbFile("factory-registry");
  auto registry = createSqliteRegistry(dbFile.string());
  ASSERT_NE(registry, nullptr);

  registry->setLatest("workspace-c.fr");
  ASSERT_TRUE(registry->getLatest().has_value());
  EXPECT_EQ(registry->getLatest().value(), "workspace-c.fr");
}

} // namespace persistence::tests
