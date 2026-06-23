/**
 * @file core/tests/domain/values/TestContractType.cpp
 * @brief Tests for contract type normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/ContractType.h"

namespace core::domain {

TEST(ContractTypeTest, TrimsWhitespaceAndKeepsStableValue) {
    const ContractType type{"  electricity  "};

    EXPECT_EQ(type.value(), "electricity");
    EXPECT_FALSE(type.empty());
    EXPECT_TRUE(ContractType::isValid("  electricity  "));
}

TEST(ContractTypeTest, RejectsBlankInput) {
    EXPECT_FALSE(ContractType::isValid("   "));
    EXPECT_TRUE(ContractType{}.empty());
}

} // namespace core::domain
