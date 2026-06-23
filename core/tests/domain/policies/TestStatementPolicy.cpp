/**
 * @file core/tests/domain/policies/TestStatementPolicy.cpp
 * @brief Tests for statement policy helpers.
 */

#include <gtest/gtest.h>

#include "core/domain/policies/StatementPolicy.h"

namespace core::domain::policies::statement {

TEST(StatementPolicyTest, TrimsAndNormalizesTransactionIds) {
    EXPECT_EQ(trimCopy("  tx-1  "), "tx-1");
    EXPECT_EQ(normalizeId("  tx-1  "), "tx-1");
}

TEST(StatementPolicyTest, NormalizesAndDeduplicatesTransactionIdLists) {
    std::vector<std::string> ids{" tx-1 ", "tx-1", "tx-2"};

    normalizeIds(ids);

    ASSERT_EQ(ids.size(), 2u);
    EXPECT_EQ(ids[0], "tx-1");
    EXPECT_EQ(ids[1], "tx-2");
}

TEST(StatementPolicyTest, FindsTransactionMembershipAndIndexDeterministically) {
    const std::vector<std::string> ids{"tx-1", "tx-2", "tx-3"};

    EXPECT_TRUE(containsTransactionId(ids, " tx-2 "));
    EXPECT_EQ(indexOfTransactionId(ids, " tx-3 "), 2u);
    EXPECT_TRUE(hasUniqueTransactionIds(ids));
}

} // namespace core::domain::policies::statement
