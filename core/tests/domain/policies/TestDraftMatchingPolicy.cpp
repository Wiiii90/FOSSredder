/**
 * @file core/tests/domain/policies/TestDraftMatchingPolicy.cpp
 * @brief Tests for draft matching policy helpers.
 */

#include <gtest/gtest.h>

#include "core/domain/policies/DraftMatchingPolicy.h"

namespace core::domain::policies::matching {

TEST(DraftMatchingPolicyTest, NormalizesAndSplitsTextFamilies) {
    EXPECT_EQ(trim("  Alpha  "), "Alpha");
    EXPECT_TRUE(matchesDraftText("Alpha", " alpha "));

    const auto lines = splitLines("  one \n\n two \r\n three ");
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "one");
    EXPECT_EQ(lines[1], "two");
    EXPECT_EQ(lines[2], "three");
}

TEST(DraftMatchingPolicyTest, ExtractsLeadingAndTokenizedTextDeterministically) {
    EXPECT_EQ(joinNonEmptyLines({" one ", "", " two "}, " | "), "one | two");
    EXPECT_EQ(firstMeaningfulLine("\n\n  first\nsecond"), "first");
    EXPECT_EQ(leadingPhrase("alpha beta gamma", 2), "alpha beta");
    EXPECT_EQ(normalizeText("Alpha-1 Beta"), "alpha 1 beta");
    EXPECT_EQ(leadingText("Alpha-1 Beta Gamma", 2), "alpha 1");
    EXPECT_TRUE(tokenIsMatchable("abc"));
    EXPECT_TRUE(tokenIsMatchable("12"));
    EXPECT_EQ(tokenOverlapScore({"alpha", "beta"}, {"beta", "gamma"}), 1);
}

TEST(DraftMatchingPolicyTest, ScoresAliasSignalsAndExtractsHints) {
    EXPECT_EQ(aliasHitWeight(0), aliasHitWeight(1));
    EXPECT_EQ(aliasRecencyWeight("2024-01-01T00:00:00Z"), 0.0);
    EXPECT_EQ(metadataSignalText("Kundennr: 12345"), "Kundennr 12345");
    EXPECT_FALSE(referenceAliasesFromMetadata("Kundennr: 12345").empty());

    core::domain::catalog::WorkspaceCatalog state;
    auto contract = std::make_shared<core::domain::Contract>();
    contract->setType("rent");
    state.setContracts({contract});

    EXPECT_EQ(knownContractTypes(state).size(), 1u);
    EXPECT_EQ(extractTypeText(state, "rent", {"rent - Alpha"}), "rent");
    EXPECT_EQ(extractActorText({"Alpha Rent", "2024-01-01"}, "rent"), "Alpha Rent");
}

} // namespace core::domain::policies::matching
