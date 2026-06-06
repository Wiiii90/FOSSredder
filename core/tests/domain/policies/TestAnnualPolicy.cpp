/**
 * @file core/tests/domain/policies/TestAnnualPolicy.cpp
 * @brief Tests for annual policy helpers.
 */

#include <memory>

#include <gtest/gtest.h>

#include "core/domain/policies/AnnualPolicy.h"

namespace core::domain::policies::annual {

TEST(AnnualPolicyTest, ValidatesYearAndAnalysisIdCollections) {
    EXPECT_TRUE(isValidYear(2024));
    EXPECT_FALSE(isValidYear(0));
    EXPECT_TRUE(isValidAnnual(2024, {"analysis-a"}));
    EXPECT_FALSE(isValidAnnual(2024, {}));
}

TEST(AnnualPolicyTest, NormalizesAndChecksAnalysisIds) {
    std::vector<std::string> ids{" analysis-a ", "analysis-a", "analysis-b"};

    normalizeIds(ids);

    ASSERT_EQ(ids.size(), 2u);
    EXPECT_TRUE(containsAnalysisId(ids, " analysis-a "));
    EXPECT_TRUE(hasUniqueAnalysisIds(ids));
    EXPECT_TRUE(containsAllAnalysisIds(ids, {"analysis-a"}));
}

TEST(AnnualPolicyTest, ExtractsBookingYearsAndValidatesTransactions) {
    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setBookingDate("2024-01-31");

    const auto year = extractBookingYear("2024-01-31");
    ASSERT_TRUE(year.has_value());
    EXPECT_EQ(*year, 2024);

    const auto result = validateTransactionsForYear(2024, {transaction});
    EXPECT_TRUE(result.isValid());
    EXPECT_FALSE(result.hasIssues());
}

} // namespace core::domain::policies::annual
