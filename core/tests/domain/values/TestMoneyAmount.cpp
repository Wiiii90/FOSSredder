/**
 * @file core/tests/domain/values/TestMoneyAmount.cpp
 * @brief Tests for money amount validation.
 */

#include <limits>

#include <gtest/gtest.h>

#include "core/domain/values/MoneyAmount.h"

namespace core::domain {

TEST(MoneyAmountTest, StoresFiniteValue) {
    const MoneyAmount amount{12.5};

    EXPECT_DOUBLE_EQ(amount.value(), 12.5);
    EXPECT_TRUE(MoneyAmount::isValid(12.5));
}

TEST(MoneyAmountTest, RejectsNonFiniteValues) {
    EXPECT_FALSE(MoneyAmount::isValid(std::numeric_limits<double>::infinity()));
    EXPECT_FALSE(MoneyAmount::isValid(std::numeric_limits<double>::quiet_NaN()));
}

} // namespace core::domain
