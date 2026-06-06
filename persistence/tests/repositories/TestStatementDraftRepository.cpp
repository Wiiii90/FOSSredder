/**
 * @file persistence/tests/repositories/TestStatementDraftRepository.cpp
 * @brief Tests for the SQLite-backed statement draft repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteStatementDraftRepository.h"
#include "persistence/repositories/SqliteTransactionDraftRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(StatementDraftRepositoryTest, AddsUpdatesRemovesAndClearsStatementDrafts)
{
    TempDatabase dbFile("statement-draft-repository");
    SqliteTransactionDraftRepository transactionDrafts(dbFile.string());
    SqliteStatementDraftRepository repo(dbFile.string());

    auto draft = makeStatementDraft();
    draft.transactionIds.clear();
    repo.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(draft));

    auto tx1 = makeTransactionDraft("draft-tx-1");
    tx1.statementDraftId = "statement-draft-1";
    auto tx2 = makeTransactionDraft("draft-tx-2");
    tx2.statementDraftId = "statement-draft-1";
    transactionDrafts.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(tx1));
    transactionDrafts.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(tx2));

    draft.transactionIds = { "draft-tx-1", "draft-tx-2" };
    repo.updateStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(draft));

    ASSERT_EQ(repo.getStatementDrafts().size(), 1);
    ASSERT_TRUE(repo.getStatementDraftById("statement-draft-1").has_value());
    EXPECT_EQ(repo.getStatementDraftById("statement-draft-1").value()->name, "Imported Statement");
    EXPECT_EQ(repo.getStatementDraftById("statement-draft-1").value()->transactionIds,
              std::vector<std::string>({"draft-tx-1", "draft-tx-2"}));

    repo.removeStatementDraft("statement-draft-1");
    EXPECT_FALSE(repo.getStatementDraftById("statement-draft-1").has_value());
    EXPECT_TRUE(repo.getStatementDrafts().empty());

    repo.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(makeStatementDraft("statement-draft-2")));
    repo.clearStatementDrafts();
    EXPECT_TRUE(repo.getStatementDrafts().empty());
}

} // namespace persistence::tests
