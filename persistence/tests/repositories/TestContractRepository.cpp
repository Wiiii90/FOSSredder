/**
 * @file persistence/tests/repositories/TestContractRepository.cpp
 * @brief Tests for the SQLite-backed contract repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(ContractRepositoryTest, AddsUpdatesRemovesAndClearsContracts)
{
    TempDatabase dbFile("contract-repository");
    SqliteActorRepository actors(dbFile.string());
    SqlitePropertyRepository properties(dbFile.string());
    SqliteContractRepository repo(dbFile.string());

    actors.addActor(makeActor());
    auto actor2 = makeActor("actor-2");
    actor2->rename("Secondary Actor");
    actors.addActor(actor2);
    properties.addProperty(makeProperty());
    auto property2 = makeProperty("property-2");
    property2->rename("Secondary Property");
    properties.addProperty(property2);

    const auto contract = makeContract();
    repo.addContract(contract);

    ASSERT_EQ(repo.getContracts().size(), 1);
    ASSERT_TRUE(repo.getContractById("contract-1").has_value());
    EXPECT_EQ(repo.getContractById("contract-1").value()->name(), "Lease Contract");
    EXPECT_EQ(repo.getContractById("contract-1").value()->type(), "lease");
    EXPECT_EQ(repo.getContractById("contract-1").value()->aliases().size(), 1);
    EXPECT_EQ(repo.getContractById("contract-1").value()->aliases().at(0).value(), "Lease");
    EXPECT_EQ(repo.getContractsForActor("actor-1").size(), 1);
    EXPECT_EQ(repo.getContractsForProperty("property-1").size(), 1);
    EXPECT_EQ(repo.getActorIdsForContract("contract-1"), std::vector<std::string>{"actor-1"});
    EXPECT_EQ(repo.getPropertyIdsForContract("contract-1"), std::vector<std::string>{"property-1"});

    auto updated = makeContract();
    updated->rename("Updated Contract");
    updated->setType("rental");
    updated->setActorIds({ "actor-1", "actor-2" });
    updated->setPropertyIds({ "property-1", "property-2" });
    repo.updateContract(updated);

    ASSERT_TRUE(repo.getContractById("contract-1").has_value());
    EXPECT_EQ(repo.getContractById("contract-1").value()->name(), "Updated Contract");
    EXPECT_EQ(repo.getContractById("contract-1").value()->type(), "rental");
    EXPECT_EQ(repo.getContractById("contract-1").value()->aliases().size(), 1);
    EXPECT_EQ(repo.getContractById("contract-1").value()->aliases().at(0).value(), "Lease");
    EXPECT_EQ(repo.getActorIdsForContract("contract-1"),
              std::vector<std::string>({"actor-1", "actor-2"}));
    EXPECT_EQ(repo.getPropertyIdsForContract("contract-1"),
              std::vector<std::string>({"property-1", "property-2"}));

    repo.removeContract("contract-1");
    EXPECT_FALSE(repo.getContractById("contract-1").has_value());
    EXPECT_TRUE(repo.getContracts().empty());

    repo.addContract(makeContract("contract-2"));
    repo.clearContracts();
    EXPECT_TRUE(repo.getContracts().empty());
}

} // namespace persistence::tests
