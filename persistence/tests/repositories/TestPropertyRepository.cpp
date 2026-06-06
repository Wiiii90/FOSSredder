/**
 * @file persistence/tests/repositories/TestPropertyRepository.cpp
 * @brief Tests for the SQLite-backed property repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqlitePropertyRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(PropertyRepositoryTest, AddsUpdatesRemovesAndClearsProperties)
{
    TempDatabase dbFile("property-repository");
    SqlitePropertyRepository repo(dbFile.string());

    const auto property = makeProperty();
    repo.addProperty(property);

    ASSERT_EQ(repo.getProperties().size(), 1);
    ASSERT_TRUE(repo.getPropertyById("property-1").has_value());
    EXPECT_EQ(repo.getPropertyById("property-1").value()->name(), "Primary Property");
    EXPECT_EQ(repo.getPropertyById("property-1").value()->aliases().size(), 1);
    EXPECT_EQ(repo.getPropertyById("property-1").value()->aliases().at(0).value(), "Property Alias");

    auto updated = makeProperty();
    updated->rename("Updated Property");
    updated->setAliases({ makeAlias("Updated Alias") });
    repo.updateProperty(updated);

    ASSERT_TRUE(repo.getPropertyById("property-1").has_value());
    EXPECT_EQ(repo.getPropertyById("property-1").value()->name(), "Updated Property");
    EXPECT_EQ(repo.getPropertyById("property-1").value()->aliases().size(), 1);
    EXPECT_EQ(repo.getPropertyById("property-1").value()->aliases().at(0).value(), "Updated Alias");

    repo.removeProperty("property-1");
    EXPECT_FALSE(repo.getPropertyById("property-1").has_value());
    EXPECT_TRUE(repo.getProperties().empty());

    repo.addProperty(makeProperty("property-2"));
    repo.clearProperties();
    EXPECT_TRUE(repo.getProperties().empty());
}

} // namespace persistence::tests
