/**
 * @file core/tests/domain/policies/TestAliasPolicy.cpp
 * @brief Tests for alias policy helpers.
 */

#include <gtest/gtest.h>

#include "core/domain/policies/AliasPolicy.h"

namespace core::domain::policies::alias {

TEST(AliasPolicyTest, TrimsAndCanonicalizesAliasText) {
    EXPECT_EQ(trimCopy("  alpha  "), "alpha");
    EXPECT_EQ(canonicalAliasValue("  alpha  "), "alpha");
}

TEST(AliasPolicyTest, NormalizesAndDeduplicatesAliases) {
    std::vector<Alias> aliases{
        Alias{"  alpha  ", " kind ", " source ", "created", "updated", 0, {}},
        Alias{"alpha", "kind", "source", "created", "updated", 1, {}},
        Alias{"beta", "kind", "source", "created", "updated", 2, {}}
    };

    normalizeAliases(aliases);

    ASSERT_EQ(aliases.size(), 2u);
    EXPECT_EQ(aliases.front().value(), "alpha");
    EXPECT_EQ(aliases.back().value(), "beta");
}

TEST(AliasPolicyTest, RecordsAliasHitsDeterministically) {
    std::vector<Alias> aliases;

    recordAliasHit(aliases, "  alpha  ", "2024-01-01T00:00:00Z");
    recordAliasHit(aliases, "alpha", "2024-01-02T00:00:00Z");

    ASSERT_EQ(aliases.size(), 1u);
    EXPECT_EQ(aliases.front().value(), "alpha");
    EXPECT_EQ(aliases.front().source(), "alpha");
    EXPECT_EQ(aliases.front().hitCount(), 2);
    EXPECT_EQ(aliases.front().lastUsedAt(), "2024-01-02T00:00:00Z");
    EXPECT_TRUE(containsAliasValue(aliases, " alpha "));
    removeAliasValue(aliases, "alpha");
    EXPECT_TRUE(aliases.empty());
}

} // namespace core::domain::policies::alias
