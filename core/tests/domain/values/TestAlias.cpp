/**
 * @file core/tests/domain/values/TestAlias.cpp
 * @brief Tests for alias storage and mutation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/Alias.h"

namespace core::domain {

TEST(AliasTest, StoresAndMutatesFields) {
    Alias alias{"  alpha  ", "  kind  ", "  source  ", "created", "updated", 2, "last"};

    EXPECT_EQ(alias.value(), "  alpha  ");
    EXPECT_EQ(alias.kind(), "  kind  ");
    EXPECT_EQ(alias.source(), "  source  ");
    EXPECT_EQ(alias.createdAt(), "created");
    EXPECT_EQ(alias.updatedAt(), "updated");
    EXPECT_EQ(alias.hitCount(), 2);
    EXPECT_EQ(alias.lastUsedAt(), "last");

    alias.setValue("beta");
    alias.setKind("contract");
    alias.setSource("mail");
    alias.setCreatedAt("c");
    alias.setUpdatedAt("u");
    alias.setHitCount(4);
    alias.setLastUsedAt("l");

    EXPECT_EQ(alias.value(), "beta");
    EXPECT_EQ(alias.kind(), "contract");
    EXPECT_EQ(alias.source(), "mail");
    EXPECT_EQ(alias.createdAt(), "c");
    EXPECT_EQ(alias.updatedAt(), "u");
    EXPECT_EQ(alias.hitCount(), 4);
    EXPECT_EQ(alias.lastUsedAt(), "l");
}

} // namespace core::domain
