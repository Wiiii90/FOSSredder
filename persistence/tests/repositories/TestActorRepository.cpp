/**
 * @file persistence/tests/repositories/TestActorRepository.cpp
 * @brief Tests for the SQLite-backed actor repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteActorRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(ActorRepositoryTest, AddsUpdatesRemovesAndClearsActors)
{
    TempDatabase dbFile("actor-repository");
    SqliteActorRepository repo(dbFile.string());

    const auto actor = makeActor();
    repo.addActor(actor);

    ASSERT_EQ(repo.getActors().size(), 1);
    ASSERT_TRUE(repo.getActorById("actor-1").has_value());
    EXPECT_EQ(repo.getActorById("actor-1").value()->name(), "Main Actor");
    EXPECT_EQ(repo.getActorById("actor-1").value()->aliases().size(), 2);
    EXPECT_EQ(repo.getActorById("actor-1").value()->aliases().at(0).value(), "Primary Actor");
    EXPECT_EQ(repo.getActorById("actor-1").value()->aliases().at(1).value(), "Main Actor");

    auto updated = makeActor();
    updated->rename("Updated Actor");
    updated->setAliases({ makeAlias("Updated Alias") });
    repo.updateActor(updated);

    ASSERT_TRUE(repo.getActorById("actor-1").has_value());
    EXPECT_EQ(repo.getActorById("actor-1").value()->name(), "Updated Actor");
    EXPECT_EQ(repo.getActorById("actor-1").value()->aliases().size(), 1);
    EXPECT_EQ(repo.getActorById("actor-1").value()->aliases().at(0).value(), "Updated Alias");

    repo.removeActor("actor-1");
    EXPECT_FALSE(repo.getActorById("actor-1").has_value());
    EXPECT_TRUE(repo.getActors().empty());

    repo.addActor(makeActor("actor-2"));
    repo.clearActors();
    EXPECT_TRUE(repo.getActors().empty());
}

TEST(ActorRepositoryTest, UpsertsMissingActors)
{
    TempDatabase dbFile("actor-repository-upsert");
    SqliteActorRepository repo(dbFile.string());

    auto actor = makeActor("actor-upsert");
    actor->rename("Upserted Actor");
    repo.upsertActor(actor);

    ASSERT_TRUE(repo.getActorById("actor-upsert").has_value());
    EXPECT_EQ(repo.getActorById("actor-upsert").value()->name(), "Upserted Actor");
}

} // namespace persistence::tests
