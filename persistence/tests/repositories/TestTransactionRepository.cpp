/**
 * @file persistence/tests/repositories/TestTransactionRepository.cpp
 * @brief Tests for the SQLite-backed transaction repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(TransactionRepositoryTest, AddsUpdatesRemovesAndClearsTransactions)
{
    TempDatabase dbFile("transaction-repository");
    SqliteActorRepository actors(dbFile.string());
    SqlitePropertyRepository properties(dbFile.string());
    SqliteContractRepository contracts(dbFile.string());
    SqliteTransactionRepository repo(dbFile.string());

    actors.addActor(makeActor());
    properties.addProperty(makeProperty());
    auto property2 = makeProperty("property-2");
    property2->rename("Secondary Property");
    properties.addProperty(property2);
    contracts.addContract(makeContract());
    contracts.addContract(makeContract("contract-2"));

    repo.addTransaction(makeTransaction("tx-1", "", "property-1"));
    repo.addTransaction(makeTransaction("tx-2", "", "property-1"));

    ASSERT_EQ(repo.getTransactions().size(), 2);
    ASSERT_TRUE(repo.getTransactionById("tx-1").has_value());
    EXPECT_EQ(repo.getTransactionById("tx-1").value()->name(), "Rent");
    EXPECT_EQ(repo.getTransactionById("tx-1").value()->contractId(), "contract-1");
    EXPECT_EQ(repo.getTransactionsForContract("contract-1").size(), 2);

    auto updated = makeTransaction("tx-1", "", "property-1");
    updated->setName("Updated Rent");
    updated->setAmount(2222.5);
    updated->setPropertyIds({ "property-1", "property-2" });
    repo.updateTransaction(updated);

    ASSERT_TRUE(repo.getTransactionById("tx-1").has_value());
    EXPECT_EQ(repo.getTransactionById("tx-1").value()->name(), "Updated Rent");
    EXPECT_DOUBLE_EQ(repo.getTransactionById("tx-1").value()->amount(), 2222.5);
    EXPECT_EQ(repo.getTransactionById("tx-1").value()->propertyIds(),
              std::vector<std::string>({"property-1", "property-2"}));

    repo.assignTransactionsToContract("contract-2", { "tx-1" });
    ASSERT_TRUE(repo.getTransactionById("tx-1").has_value());
    EXPECT_EQ(repo.getTransactionById("tx-1").value()->contractId(), "contract-2");
    EXPECT_EQ(repo.getTransactionsForContract("contract-2").size(), 1);

    repo.removeTransaction("tx-1");
    EXPECT_FALSE(repo.getTransactionById("tx-1").has_value());

    repo.clearTransactions();
    EXPECT_TRUE(repo.getTransactions().empty());
}

} // namespace persistence::tests
