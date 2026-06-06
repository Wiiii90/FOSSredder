/**
 * @file ui/tests/unit/viewmodels/TestImportViewModel.cpp
 * @brief Tests for the UI ImportViewModel boundary.
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

TEST(ImportViewModelTest,
     VM_IMPORT_001_AddSelectedImportFilesQueuesOnlySupportedPdfFiles) {
  tests::support::WorkspaceHarness harness;
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);

  viewModel.setManualPathText(QStringLiteral("C:/tmp/statement.pdf"));
  viewModel.addSelectedImportFiles();

  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("C:/tmp/statement.pdf"));
  EXPECT_EQ(viewModel.queuedCount(), 0);
  EXPECT_TRUE(viewModel.canStart());
  EXPECT_TRUE(viewModel.manualPathText().isEmpty());
}

TEST(ImportViewModelTest,
     VM_IMPORT_002_OpenDraftLogSwitchesToDraftContentAndNavigation) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  Navigation navigation;
  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspaceRoles(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());
  viewModel.setNavigation(&navigation);

  viewModel.openImportLog(QStringLiteral("draft-2"), true, {}, {});

  EXPECT_TRUE(viewModel.hasDraft());
  EXPECT_EQ(viewModel.selectedDraftId(), QStringLiteral("draft-2"));
  EXPECT_EQ(viewModel.contentIndex(), 1);
  EXPECT_EQ(navigation.section(), Navigation::Section::Import);
}

TEST(ImportViewModelTest,
     VM_IMPORT_003_RenameAndNavigateCurrentTransactionDraft) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));

  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspaceRoles(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());
  viewModel.setStatementName(QStringLiteral("Renamed Draft"));
  viewModel.selectNextTransactionDraft();

  EXPECT_EQ(viewModel.statementName(), QStringLiteral("Renamed Draft"));
  EXPECT_EQ(viewModel.currentTransactionNumber(), 2);
  EXPECT_TRUE(viewModel.canSelectPreviousTransactionDraft());
  EXPECT_FALSE(viewModel.canSelectNextTransactionDraft());
}

TEST(ImportViewModelTest,
     VM_IMPORT_004_AddAndDeleteTransactionDraftViaAdapter) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));

  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.addTransactionAfterCurrent();
  EXPECT_EQ(viewModel.transactionCount(), 3);

  viewModel.deleteCurrentTransaction();
  EXPECT_EQ(viewModel.transactionCount(), 2);
}

TEST(ImportViewModelTest, VM_IMPORT_005_CommitsEditableTextToCurrentDraft) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));

  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.openImportLog(QStringLiteral("draft-1"), true, {}, {});
  viewModel.setNameText(QStringLiteral("Committed Name"));
  viewModel.setBookingDateText(QStringLiteral("2026-03-01"));
  viewModel.setAmountText(QStringLiteral("99.50"));
  viewModel.commitNameText();
  viewModel.commitBookingDateText();
  viewModel.commitAmountText();

  EXPECT_EQ(viewModel.nameText(), QStringLiteral("Committed Name"));
  EXPECT_EQ(viewModel.bookingDateText(), QStringLiteral("2026-03-01"));
  EXPECT_EQ(viewModel.amountText(), QStringLiteral("99.50"));
}

TEST(ImportViewModelTest,
     VM_IMPORT_006_PropertySelectionUpdatesCurrentDraftState) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeStateWithDraftStack());
  const auto runner = std::make_shared<tests::support::ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(runner);
  const auto reporter = tests::support::noopErrorReporter();
  ImportWorkflow workflow(
      adapter, reporter,
      [&]() {
        return harness.store->snapshot();
      },
      harness.commands.get(), harness.selectors.get());
  ASSERT_TRUE(workflow.openStoredDraft(QStringLiteral("draft-1")));

  ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspaceRoles(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());
  viewModel.openImportLog(QStringLiteral("draft-1"), true, {}, {});
  viewModel.setPropertySelected(QStringLiteral("property-1"), true);

  EXPECT_TRUE(viewModel.isPropertySelected(QStringLiteral("property-1")));
}

} // namespace ui
