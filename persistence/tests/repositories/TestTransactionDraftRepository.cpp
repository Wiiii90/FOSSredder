/**
 * @file persistence/tests/repositories/TestTransactionDraftRepository.cpp
 * @brief Tests for the SQLite-backed transaction draft repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteStatementDraftRepository.h"
#include "persistence/repositories/SqliteTransactionDraftRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(TransactionDraftRepositoryTest, AddsUpdatesRemovesAndClearsTransactionDrafts)
{
    TempDatabase dbFile("transaction-draft-repository");
    SqliteStatementDraftRepository statementDrafts(dbFile.string());
    SqliteTransactionDraftRepository repo(dbFile.string());

    auto statementDraft = makeStatementDraft();
    statementDraft.transactionIds.clear();
    statementDrafts.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(statementDraft));

    auto draft = makeTransactionDraft("draft-tx-1");
    draft.statementDraftId = "statement-draft-1";
    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(draft));

    ASSERT_EQ(repo.getTransactionDrafts().size(), 1);
    ASSERT_TRUE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->name, "Draft Rent");
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->propertyIds,
              std::vector<std::string>({"property-1"}));

    auto updated = makeTransactionDraft("draft-tx-1");
    updated.statementDraftId = "statement-draft-1";
    updated.name = "Updated Draft Rent";
    updated.propertyIds = { "property-1", "property-2" };
    repo.updateTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(updated));

    ASSERT_TRUE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->name, "Updated Draft Rent");
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->propertyIds,
              std::vector<std::string>({"property-1", "property-2"}));

    repo.removeTransactionDraft("draft-tx-1");
    EXPECT_FALSE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_TRUE(repo.getTransactionDrafts().empty());

    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(makeTransactionDraft("draft-tx-2")));
    repo.clearTransactionDrafts();
    EXPECT_TRUE(repo.getTransactionDrafts().empty());
}

TEST(TransactionDraftRepositoryTest, PreservesExplicitZeroBasedPositions)
{
    TempDatabase dbFile("transaction-draft-repository-positions");
    SqliteStatementDraftRepository statementDrafts(dbFile.string());
    SqliteTransactionDraftRepository repo(dbFile.string());

    auto statementDraft = makeStatementDraft();
    statementDraft.transactionIds.clear();
    statementDrafts.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(statementDraft));

    auto first = makeTransactionDraft("draft-tx-1");
    first.statementDraftId = "statement-draft-1";
    first.position = 0;
    auto second = makeTransactionDraft("draft-tx-2");
    second.statementDraftId = "statement-draft-1";
    second.position = 1;

    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(first));
    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(second));

    const auto rows = repo.getTransactionDrafts();
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0]->id, "draft-tx-1");
    EXPECT_EQ(rows[0]->position, 0);
    EXPECT_EQ(rows[1]->id, "draft-tx-2");
    EXPECT_EQ(rows[1]->position, 1);
}

} // namespace persistence::tests
