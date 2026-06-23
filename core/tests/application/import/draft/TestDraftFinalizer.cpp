/**
 * @file core/tests/application/import/draft/TestDraftFinalizer.cpp
 * @brief Tests for draft finalization behavior.
 */

#include <gtest/gtest.h>

#include "core/application/import/draft/DraftFinalizer.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"

namespace core::application::importing::draft {

TEST(DraftFinalizerTest, FinalizesValidDraftIntoWorkspaceState) {
    core::domain::catalog::WorkspaceCatalog state;

    StatementDraft draft;
    draft.name = "Imported Statement";

    TransactionDraft item;
    item.name = "Rent";
    item.bookingDate = "2024-01-31";
    item.valuta = "02.02";
    item.amount = 10.0;
    item.type = "rent";
    item.allocatable = true;
    draft.transactions.push_back(item);

    const auto statementId = DraftFinalizer::finalize(state, draft);

    EXPECT_FALSE(statementId.empty());
    EXPECT_EQ(state.statements().size(), 1u);
    EXPECT_EQ(state.transactions().size(), 1u);
    EXPECT_EQ(state.contracts().size(), 1u);
    EXPECT_EQ(state.statements().front()->transactionCount(), 1u);
    EXPECT_EQ(state.transactions().front()->statementId(), statementId);
    EXPECT_EQ(state.transactions().front()->valuta(), "02.02");
}

TEST(DraftFinalizerTest, ReturnsEmptyStringForEmptyDraft) {
    core::domain::catalog::WorkspaceCatalog state;
    StatementDraft draft;

    const auto statementId = DraftFinalizer::finalize(state, draft);

    EXPECT_TRUE(statementId.empty());
    EXPECT_TRUE(state.empty());
}

} // namespace core::application::importing::draft
