/**
 * @file core/tests/domain/policies/TestTransactionPolicy.cpp
 * @brief Tests for transaction policy helpers.
 */

#include <limits>

#include <gtest/gtest.h>

#include "core/domain/policies/TransactionPolicy.h"

namespace core::domain::policies::transaction {

TEST(TransactionPolicyTest, TrimsAndNormalizesTextAndIds) {
    EXPECT_EQ(trimCopy("  rent  "), "rent");
    EXPECT_EQ(normalizeText("  rent  "), "rent");
    EXPECT_EQ(normalizeId("  tx-1  "), "tx-1");
}

TEST(TransactionPolicyTest, ValidatesDraftInputsAndFinalizationReadiness) {
    EXPECT_TRUE(hasValidBookingDate("2024-01-31"));
    EXPECT_TRUE(hasValidAmount(12.5));
    EXPECT_TRUE(hasStatementAssignment(" statement-1 "));
    EXPECT_TRUE(canFinalizeFromDraft("2024-01-31", 12.5, "statement-1"));
    EXPECT_FALSE(canFinalizeFromDraft("", 12.5, "statement-1"));
    EXPECT_FALSE(hasValidAmount(std::numeric_limits<double>::infinity()));
}

TEST(TransactionPolicyTest, NormalizesRelationIdsAndStatusTransitions) {
    std::vector<std::string> ids{" prop-1 ", "prop-1", "prop-2"};

    normalizeIds(ids);

    ASSERT_EQ(ids.size(), 2u);
    EXPECT_EQ(ids[0], "prop-1");
    EXPECT_EQ(ids[1], "prop-2");
    EXPECT_TRUE(statusCanAdvance(1, 2));
    EXPECT_FALSE(statusCanAdvance(2, 1));
    EXPECT_TRUE(hasPropertyIds(ids));
}

} // namespace core::domain::policies::transaction
