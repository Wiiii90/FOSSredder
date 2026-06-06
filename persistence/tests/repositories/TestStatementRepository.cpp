/**
 * @file persistence/tests/repositories/TestStatementRepository.cpp
 * @brief Tests for the SQLite-backed statement repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(StatementRepositoryTest, AddsUpdatesRemovesAndClearsStatements)
{
    TempDatabase dbFile("statement-repository");
    SqliteTransactionRepository transactions(dbFile.string());
    SqliteStatementRepository repo(dbFile.string());

    auto tx1 = makeTransaction("tx-1", "", "");
    tx1->setContractId("");
    tx1->setActorId("");
    tx1->setPropertyIds({});
    auto tx2 = makeTransaction("tx-2", "", "");
    tx2->setContractId("");
    tx2->setActorId("");
    tx2->setPropertyIds({});
    transactions.addTransaction(tx1);
    transactions.addTransaction(tx2);

    auto statement = makeStatement();
    statement->setTransactionIds({ "tx-1", "tx-2" });
    repo.addStatement(statement);

    ASSERT_EQ(repo.getStatements().size(), 1);
    ASSERT_TRUE(repo.getStatementById("statement-1").has_value());
    EXPECT_EQ(repo.getStatementById("statement-1").value()->name(), "January Statement");
    EXPECT_EQ(repo.getStatementById("statement-1").value()->transactionIds(),
              std::vector<std::string>({"tx-1", "tx-2"}));

    auto updated = makeStatement();
    updated->rename("Updated Statement");
    updated->setTransactionIds({ "tx-2" });
    repo.updateStatement(updated);

    ASSERT_TRUE(repo.getStatementById("statement-1").has_value());
    EXPECT_EQ(repo.getStatementById("statement-1").value()->name(), "Updated Statement");
    EXPECT_EQ(repo.getStatementById("statement-1").value()->transactionIds(),
              std::vector<std::string>({"tx-2"}));

    repo.removeStatement("statement-1");
    EXPECT_FALSE(repo.getStatementById("statement-1").has_value());
    EXPECT_TRUE(repo.getStatements().empty());

    repo.addStatement(makeStatement("statement-2"));
    repo.clearStatements();
    EXPECT_TRUE(repo.getStatements().empty());
}

} // namespace persistence::tests
