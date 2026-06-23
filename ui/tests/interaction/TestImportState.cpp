/**
 * @file ui/tests/interaction/TestImportState.cpp
 * @brief Interaction tests for import overview wiring.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/ImportRunnerStub.h"
#include "support/ViewModelTestHarness.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/shell/Settings.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui {

TEST(ImportInteractionTest,
     INTERACTION_IMPORT_001_OverviewAppliesDefaultPathAndFiltersManualFiles) {
  tests::support::WorkspaceHarness harness;
  const auto adapter = std::make_shared<adapters::ImportAdapter>(
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  Settings settings;
  ImportViewModel viewModel;

  settings.setImportDefaultPath(QStringLiteral("P:/imports/default.pdf"));
  viewModel.setSettings(&settings);
  viewModel.setImportWorkflow(&workflow);

  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/default.pdf"));
  EXPECT_TRUE(viewModel.canStart());

  settings.setImportDefaultPath(QStringLiteral("P:/imports/updated.pdf"));
  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/updated.pdf"));

  settings.setImportDefaultPath({});
  workflow.setSelectedFile({});
  viewModel.setManualPathText(QStringLiteral("P:/imports/readme.txt"));
  viewModel.addSelectedImportFiles();

  EXPECT_TRUE(workflow.selectedFile().isEmpty());
  EXPECT_TRUE(workflow.queuedFiles().isEmpty());

  viewModel.setManualPathText(QStringLiteral("P:/imports/statement.PDF"));
  viewModel.addSelectedImportFiles();

  EXPECT_EQ(workflow.selectedFile(),
            QStringLiteral("P:/imports/statement.PDF"));
  EXPECT_EQ(viewModel.importFileSummary(),
            QStringLiteral("Selected: statement.PDF"));
  EXPECT_TRUE(viewModel.manualPathText().isEmpty());

  settings.setImportDefaultPath(QStringLiteral("P:/imports/later-default.pdf"));
  EXPECT_EQ(workflow.selectedFile(),
            QStringLiteral("P:/imports/statement.PDF"));
}

TEST(ImportInteractionTest,
     INTERACTION_IMPORT_002_PauseGatesWorkflowProgressUpdates) {
  tests::support::WorkspaceHarness harness;
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  runner->nextStatementImportHandle.importId = "import-1";
  runner->nextStatementImportHandle.subscriptionId = 1;
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  ImportWorkflow workflow(
      adapter, tests::support::noopErrorReporter(),
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());

  workflow.setSelectedFile(QStringLiteral("statement.pdf"));
  workflow.startStatementImport();

  EXPECT_FALSE(workflow.isPaused());
  EXPECT_EQ(workflow.phase().toStdString(), "Starting import...");

  workflow.pauseImport();
  EXPECT_TRUE(workflow.isPaused());
  EXPECT_EQ(workflow.phase().toStdString(), "Paused");

  workflow.resumeImport();
  EXPECT_FALSE(workflow.isPaused());
  EXPECT_EQ(workflow.phase().toStdString(), "Running import...");
}

TEST(
    ImportInteractionTest,
    INTERACTION_IMPORT_003_ViewModelFinalizesDraftThroughWorkflowAndWorkspace) {
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

  viewModel.openImportLog(QStringLiteral("draft-1"), true,
                          QStringLiteral("draft-1"), {});
  ASSERT_TRUE(viewModel.hasDraft());

  viewModel.finalize();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  EXPECT_FALSE(viewModel.hasDraft());
  EXPECT_EQ(snapshot.statementDrafts.size(), 2U);
  ASSERT_FALSE(snapshot.statements.empty());
  EXPECT_EQ(snapshot.statements.back().name, std::string("Draft 1"));
}

} // namespace ui
