/**
 * @file ui/tests/unit/adapters/TestImportAdapter.cpp
 * @brief Tests for the UI ImportAdapter boundary.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "support/ImportRunnerStub.h"
#include "support/ViewModelTestHarness.h"
#include "ui/adapters/ImportAdapter.h"

namespace ui {

TEST(
    ImportAdapterTest,
    ADP_IMPORT_001_PropertySelectionDelegatesToImportRunnerAndNormalizesDraft) {
  tests::support::WorkspaceHarness harness;
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      std::make_shared<tests::support::ImportRunnerStub>());

  core::ports::importing::draft::TransactionDraft transaction;
  transaction.contractId = "contract-1";
  transaction.contractSelected = true;

  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind = core::ports::importing::draft::TransactionDraftEditKind::
      SetPropertySelected;
  edit.id = "property-1";
  edit.selected = true;

  ASSERT_TRUE(adapter->updateTransactionDraft(transaction,
                                              harness.store->snapshot(), edit));

  EXPECT_TRUE(transaction.contractId.empty());
  EXPECT_FALSE(transaction.contractSelected);
  ASSERT_EQ(transaction.propertyIds.size(), 1);
  EXPECT_EQ(transaction.propertyIds.front(), std::string("property-1"));
}

} // namespace ui
