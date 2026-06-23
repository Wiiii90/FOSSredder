/**
 * @file core/tests/domain/values/TestYear.cpp
 * @brief Tests for year normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/Year.h"

namespace core::domain {

TEST(YearTest, StoresValidYear) {
    const Year year{2024};

    EXPECT_EQ(year.value(), 2024);
    EXPECT_FALSE(year.empty());
    EXPECT_TRUE(Year::isValid(2024));
}

TEST(YearTest, NormalizesInvalidYearToZero) {
    EXPECT_FALSE(Year::isValid(0));
    EXPECT_TRUE(Year{0}.empty());
    EXPECT_EQ(Year::normalize(0), 0);
}

} // namespace core::domain
