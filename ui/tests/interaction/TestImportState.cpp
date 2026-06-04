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
      harness.facade.get(),
      std::make_shared<tests::support::ImportRunnerStub>());
  ImportWorkflow workflow(adapter, tests::support::noopErrorReporter(),
                          harness.facade.get());
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
  importing::ImportWorkflowState state;
  state.beginImport(QStringLiteral("statement.pdf"));

  EXPECT_TRUE(state.setPaused(true));
  EXPECT_TRUE(state.isPaused());
  EXPECT_EQ(state.phase(), QStringLiteral("Paused"));

  state.updateProgress(0.5, QStringLiteral("halfway"));
  EXPECT_DOUBLE_EQ(state.progress(), 0.01);
  EXPECT_EQ(state.phase(), QStringLiteral("Paused"));

  EXPECT_TRUE(state.setPaused(false));
  EXPECT_FALSE(state.isPaused());
  EXPECT_EQ(state.phase(), QStringLiteral("Running import..."));

  state.updateProgress(0.5, QStringLiteral("halfway"));
  EXPECT_DOUBLE_EQ(state.progress(), 0.5);
  EXPECT_EQ(state.phase(), QStringLiteral("halfway"));
}

} // namespace ui
