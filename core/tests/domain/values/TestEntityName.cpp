/**
 * @file core/tests/domain/values/TestEntityName.cpp
 * @brief Tests for entity name normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/EntityName.h"

namespace core::domain {

TEST(EntityNameTest, TrimsWhitespaceAndKeepsStableValue) {
    const EntityName name{"  Alpha Beta  "};

    EXPECT_EQ(name.value(), "Alpha Beta");
    EXPECT_FALSE(name.empty());
    EXPECT_TRUE(EntityName::isValid("  Alpha Beta  "));
}

TEST(EntityNameTest, RejectsBlankInput) {
    EXPECT_FALSE(EntityName::isValid("   "));
    EXPECT_TRUE(EntityName{}.empty());
}

} // namespace core::domain
