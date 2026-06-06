/**
 * @file core/tests/domain/values/TestAnalysisType.cpp
 * @brief Tests for analysis type normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/AnalysisType.h"

namespace core::domain {

TEST(AnalysisTypeTest, TrimsWhitespaceAndKeepsStableValue) {
    const AnalysisType type{"  plot  "};

    EXPECT_EQ(type.value(), "plot");
    EXPECT_FALSE(type.empty());
    EXPECT_TRUE(AnalysisType::isValid("  plot  "));
}

TEST(AnalysisTypeTest, RejectsBlankInput) {
    EXPECT_FALSE(AnalysisType::isValid("   "));
    EXPECT_TRUE(AnalysisType{}.empty());
}

} // namespace core::domain
