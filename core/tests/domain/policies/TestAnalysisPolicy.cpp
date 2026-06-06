/**
 * @file core/tests/domain/policies/TestAnalysisPolicy.cpp
 * @brief Tests for analysis policy helpers.
 */

#include <gtest/gtest.h>

#include "core/constants/analysis.h"
#include "core/domain/policies/AnalysisPolicy.h"

namespace core::domain::policies::analysis {

TEST(AnalysisPolicyTest, ValidatesSupportedTypesAndFormats) {
    EXPECT_TRUE(supportsResultType(" tab "));
    EXPECT_TRUE(isTabularType(" tab "));
    EXPECT_TRUE(isChartLikeType(" pie "));
    EXPECT_TRUE(isExportable(" tab ", " xlsx "));
}

TEST(AnalysisPolicyTest, NormalizesKeysAndConfigurationState) {
    EXPECT_EQ(normalizeKey("  rent  "), "rent");
    EXPECT_TRUE(isConfigured("tab", "", "", ""));
    EXPECT_TRUE(isConfigured("", R"({"group":"month"})", "", ""));
    EXPECT_FALSE(isConfigured("", "", "", ""));
}

TEST(AnalysisPolicyTest, ResolvesExecutionAndOutputTypes) {
    EXPECT_EQ(resolveExecutionType(""), core::constants::analysis::kTypeTab);
    EXPECT_EQ(resolveExecutionType(" tab "), "tab");
    EXPECT_EQ(resolveOutputType("plot", ""), core::constants::analysis::plotTypes::kPie);
    EXPECT_EQ(resolveOutputType("plot", R"({"plotType":"histogram"})"), "histogram");
}

} // namespace core::domain::policies::analysis
