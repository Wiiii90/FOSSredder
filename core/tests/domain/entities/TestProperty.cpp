/**
 * @file core/tests/domain/entities/TestProperty.cpp
 * @brief Tests for property behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Property.h"

namespace core::domain {

namespace {

Alias MakeAlias(const std::string& value)
{
    return Alias{value, "kind", value, "created", "updated", 1, "last"};
}

} // namespace

TEST(PropertyTest, RenamesAndKeepsTechnicalStateStable) {
    Property property;
    property.setId("property-1");
    property.setCreatedAt("created");
    property.setUpdatedAt("updated");

    property.rename("  Alpha Property  ");

    EXPECT_EQ(property.id(), "property-1");
    EXPECT_EQ(property.name(), "Alpha Property");
    EXPECT_EQ(property.createdAt(), "created");
    EXPECT_EQ(property.updatedAt(), "updated");
}

TEST(PropertyTest, NormalizesAliasesAndDeduplicatesCanonicalValues) {
    Property property;

    property.addAlias(MakeAlias("alpha"));
    property.addAlias(MakeAlias(" alpha "));
    property.addAlias(MakeAlias("beta"));

    EXPECT_EQ(property.aliasCount(), 2u);
    EXPECT_TRUE(property.hasAlias("alpha"));
    EXPECT_TRUE(property.hasAlias(" beta "));
}

TEST(PropertyTest, RemovesAndRecordsAliasHitsDeterministically) {
    Property property;
    property.setAliases({MakeAlias("alpha"), MakeAlias("beta")});

    property.recordAliasHit("alpha", "2024-01-01T00:00:00Z");
    property.removeAlias(" beta ");

    ASSERT_EQ(property.aliasCount(), 1u);
    EXPECT_TRUE(property.hasAlias("alpha"));
    EXPECT_EQ(property.aliases().front().hitCount(), 2);
    EXPECT_EQ(property.aliases().front().lastUsedAt(), "2024-01-01T00:00:00Z");
}

} // namespace core::domain
