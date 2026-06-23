/**
 * @file core/tests/domain/values/TestFilterSpec.cpp
 * @brief Tests for filter specification normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/FilterSpec.h"

namespace core::domain {

TEST(FilterSpecTest, TrimsWhitespaceAndKeepsStableValue) {
    const FilterSpec spec{"  type:tab  "};

    EXPECT_EQ(spec.value(), "type:tab");
    EXPECT_FALSE(spec.empty());
    EXPECT_TRUE(FilterSpec::isValid("  type:tab  "));
}

TEST(FilterSpecTest, RejectsBlankInput) {
    EXPECT_FALSE(FilterSpec::isValid("   "));
    EXPECT_TRUE(FilterSpec{}.empty());
}

} // namespace core::domain
