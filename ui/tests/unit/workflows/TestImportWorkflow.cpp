/**
 * @file ui/tests/unit/workflows/TestImportWorkflow.cpp
 * @brief Tests for import workflow draft session behavior.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/ImportRunnerStub.h"
#include "support/ViewModelTestHarness.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui {

TEST(ImportWorkflowTest,
     WF_IMPORT_001_DraftStackNavigationCyclesThroughImportHome) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      harness.workspace.get(),
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(adapter, tests::support::noopErrorReporter(),
                          [&]() { return harness.store->snapshot(); },
                          harness.commands.get(),
                          harness.selectors.get());
  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspaceRoles(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  EXPECT_TRUE(viewModel.hasDraftNavigation());
  viewModel.selectNextDraft();
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentDraftId(), QStringLiteral("draft-1"));
  workflow.clearDraft();

  ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-3")));
  ASSERT_TRUE(workflow.hasDraft());
  viewModel.selectNextDraft();
  EXPECT_FALSE(workflow.hasDraft());

  viewModel.selectNextDraft();
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentDraftId(), QStringLiteral("draft-1"));

  viewModel.selectPreviousDraft();
  EXPECT_FALSE(workflow.hasDraft());

  viewModel.selectPreviousDraft();
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentDraftId(), QStringLiteral("draft-3"));
}

TEST(ImportWorkflowTest,
     WF_IMPORT_002_ReopenedDraftRestoresRememberedTransactionIndex) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      harness.workspace.get(),
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(adapter, tests::support::noopErrorReporter(),
                          [&]() { return harness.store->snapshot(); },
                          harness.commands.get(),
                          harness.selectors.get());

  ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  workflow.setCurrentTransactionIndex(1);

  workflow.clearDraft();
  ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentTransactionIndex(), 1);
}

} // namespace ui
