/**
 * @file core/tests/domain/values/TestBookingDate.cpp
 * @brief Tests for booking date normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/BookingDate.h"

namespace core::domain {

TEST(BookingDateTest, TrimsWhitespaceAndKeepsStableValue) {
    const BookingDate date{" 2024-01-31 "};

    EXPECT_EQ(date.value(), "2024-01-31");
    EXPECT_FALSE(date.empty());
    EXPECT_TRUE(BookingDate::isValid(" 2024-01-31 "));
}

TEST(BookingDateTest, RejectsBlankInput) {
    EXPECT_FALSE(BookingDate::isValid("   "));
    EXPECT_TRUE(BookingDate{}.empty());
}

} // namespace core::domain
