/**
 * @file core/tests/domain/entities/TestAnnual.cpp
 * @brief Tests for annual aggregate behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Annual.h"

namespace core::domain {

TEST(AnnualTest, AppliesNameYearAndAnalysisIds) {
    Annual annual;

    annual.apply("  Annual 2024  ", 2024, {" analysis-a ", "analysis-a", "analysis-b"});

    EXPECT_EQ(annual.name(), "Annual 2024");
    EXPECT_EQ(annual.year(), 2024);
    EXPECT_EQ(annual.analysisCount(), 2u);
    EXPECT_TRUE(annual.hasYear());
    EXPECT_FALSE(annual.isEmpty());
}

TEST(AnnualTest, InsertsMovesAndRemovesAnalysisIds) {
    Annual annual;
    annual.setYear(2024);
    annual.setAnalysisIds({"analysis-1", "analysis-2"});

    annual.insertAnalysisId("analysis-0", 0);
    annual.moveAnalysisId("analysis-2", 1);
    annual.removeAnalysisId("analysis-1");

    ASSERT_EQ(annual.analysisCount(), 2u);
    EXPECT_EQ(annual.analysisIds().at(0), "analysis-0");
    EXPECT_EQ(annual.analysisIds().at(1), "analysis-2");
    EXPECT_TRUE(annual.containsAnalysisId("analysis-2"));
    EXPECT_EQ(annual.indexOfAnalysisId("analysis-2"), 1u);
}

TEST(AnnualTest, ClearsAndTracksEmptiness) {
    Annual annual;

    EXPECT_TRUE(annual.isEmpty());

    annual.setYear(2024);
    annual.addAnalysisId("analysis-a");
    EXPECT_FALSE(annual.isEmpty());

    annual.clearAnalysisIds();
    annual.rename("");
    annual.setYear(0);

    EXPECT_TRUE(annual.isEmpty());
}

} // namespace core::domain
