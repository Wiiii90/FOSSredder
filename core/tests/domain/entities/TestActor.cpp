/**
 * @file core/tests/domain/entities/TestActor.cpp
 * @brief Tests for actor behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Actor.h"

namespace core::domain {

namespace {

Alias MakeAlias(const std::string& value, const std::string& source = {})
{
    Alias alias{value, "kind", source.empty() ? value : source, "created", "updated", 1, "last"};
    return alias;
}

} // namespace

TEST(ActorTest, RenamesAndKeepsTechnicalStateStable) {
    Actor actor;
    actor.setId("actor-1");
    actor.setCreatedAt("created");
    actor.setUpdatedAt("updated");

    actor.rename("  Alpha Actor  ");

    EXPECT_EQ(actor.id(), "actor-1");
    EXPECT_EQ(actor.name(), "Alpha Actor");
    EXPECT_EQ(actor.createdAt(), "created");
    EXPECT_EQ(actor.updatedAt(), "updated");
}

TEST(ActorTest, NormalizesAliasesAndDeduplicatesCanonicalValues) {
    Actor actor;

    actor.addAlias(MakeAlias("  alpha  "));
    actor.addAlias(MakeAlias("alpha"));
    actor.addAlias(MakeAlias("beta"));

    EXPECT_EQ(actor.aliasCount(), 2u);
    EXPECT_TRUE(actor.hasAlias("alpha"));
    EXPECT_TRUE(actor.hasAlias(" beta "));
}

TEST(ActorTest, RemovesAndRecordsAliasHitsDeterministically) {
    Actor actor;
    actor.setAliases({MakeAlias("alpha", "source-a"), MakeAlias("beta", "source-b")});

    actor.recordAliasHit("alpha", "2024-01-01T00:00:00Z");
    actor.removeAlias(" beta ");

    ASSERT_EQ(actor.aliasCount(), 1u);
    EXPECT_TRUE(actor.hasAlias("alpha"));
    EXPECT_EQ(actor.aliases().front().hitCount(), 2);
    EXPECT_EQ(actor.aliases().front().lastUsedAt(), "2024-01-01T00:00:00Z");
}

} // namespace core::domain
