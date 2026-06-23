/**
 * @file ui/tests/unit/workflows/TestImportWorkflow.cpp
 * @brief Tests for import workflow draft session behavior.
 */

#include <gtest/gtest.h>

#include <algorithm>
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
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspaceRoles(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  EXPECT_TRUE(viewModel.hasDraftNavigation());
  viewModel.selectNextDraft();
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentDraftId(), QStringLiteral("draft-1"));
  workflow.clearDraft();

  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-3")));
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
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());

  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  workflow.setCurrentTransactionIndex(1);

  workflow.clearDraft();
  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentTransactionIndex(), 1);
}

TEST(ImportWorkflowTest,
     WF_IMPORT_003_AddFilesSelectsFirstAndDeduplicatesQueue) {
  tests::support::WorkspaceHarness harness;
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());

  workflow.addFiles(
      {QStringLiteral(" P:/imports/a.pdf "), QStringLiteral("P:/imports/b.pdf"),
       QStringLiteral("P:/imports/a.pdf"), QStringLiteral("P:/imports/b.pdf")});

  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/a.pdf"));
  EXPECT_EQ(workflow.queuedFiles(),
            QStringList{QStringLiteral("P:/imports/b.pdf")});
  EXPECT_EQ(workflow.queuedCount(), 1);
}

TEST(ImportWorkflowTest,
     WF_IMPORT_004_FlushActiveDraftWritesWorkspaceDraftSnapshot) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());

  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.renameCurrentStatementDraft(QStringLiteral("Renamed")));
  workflow.flushActiveDraftToWorkspace();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  const auto it =
      std::find_if(snapshot.statementDrafts.begin(),
                   snapshot.statementDrafts.end(), [](const auto& row) {
                     return row.id == "draft-1";
                   });
  ASSERT_NE(it, snapshot.statementDrafts.end());
  EXPECT_EQ(it->name, std::string("Renamed"));
}

TEST(ImportWorkflowTest,
     WF_IMPORT_005_FinalizeActiveDraftDelegatesToWorkspaceCommands) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());

  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));
  workflow.finalizeActiveDraft();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  EXPECT_EQ(snapshot.statementDrafts.size(), 2U);
  ASSERT_FALSE(snapshot.statements.empty());
  EXPECT_EQ(snapshot.statements.back().name, std::string("Draft 1"));
}

} // namespace ui
