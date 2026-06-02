/**
 * @file ui/tests/interaction/TestImportState.cpp
 * @brief Smoke tests for the import overview view model.
 */

#include <gtest/gtest.h>

#include "core/errors/IErrorReporter.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/shell/Settings.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"

namespace ui {

namespace {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
  void report(const core::errors::ErrorEvent &) override {}
};

class ImportRunnerStub final : public core::ports::importing::IImportRunner {
public:
  core::ports::importing::StatementImportHandle startStatementImport(
      const core::ports::importing::StatementImportStartRequest &,
      core::ports::importing::StatementImportEventCallback) override {
    return {};
  }
  void unsubscribe(
      const core::ports::importing::StatementImportHandle &) override {}
  void cancel(const core::ports::importing::StatementImportHandle &) override {}
  void pause(const core::ports::importing::StatementImportHandle &) override {}
  void resume(const core::ports::importing::StatementImportHandle &) override {}
  core::ports::importing::ImportResult importResult(
      const core::ports::importing::StatementImportHandle &) override {
    return {};
  }
  core::ports::importing::draft::DraftImportSuggestions buildImportSuggestions(
      const core::ports::workspace::WorkspaceSnapshot &,
      const core::ports::importing::draft::TransactionDraft &) const override {
    return {};
  }
  core::ports::importing::draft::DraftTextSignals buildDraftTextSignals(
      const core::ports::workspace::WorkspaceSnapshot &,
      const core::ports::importing::draft::TransactionDraft &) const override {
    return {};
  }
  core::ports::importing::draft::DraftDerivedState
  buildDraftDerivedState(const core::ports::workspace::WorkspaceSnapshot &,
                         const core::ports::importing::draft::DraftLinkSelection
                             &) const override {
    return {};
  }
  bool applyDerivedSelections(
      core::ports::importing::draft::TransactionDraft &,
      const core::ports::importing::draft::DraftDerivedState &,
      core::ports::importing::draft::DraftAutoSelectionMode) const override {
    return false;
  }
  bool applyActorSelection(
      core::ports::importing::draft::TransactionDraft &,
      const std::string &) const override {
    return false;
  }
  bool clearActorSelection(
      core::ports::importing::draft::TransactionDraft &) const override {
    return false;
  }
  bool applyPropertySelection(
      core::ports::importing::draft::TransactionDraft &,
      const std::string &) const override {
    return false;
  }
  bool setPropertySelected(
      core::ports::importing::draft::TransactionDraft &,
      const std::string &,
      bool) const override {
    return false;
  }
  bool applyContractSelection(
      core::ports::importing::draft::TransactionDraft &,
      const core::ports::importing::draft::DraftChoiceRow &) const override {
    return false;
  }
  bool applyContractSelection(
      core::ports::importing::draft::TransactionDraft &,
      const core::ports::workspace::WorkspaceSnapshot &,
      const std::string &) const override {
    return false;
  }
  bool clearContractSelection(
      core::ports::importing::draft::TransactionDraft &) const override {
    return false;
  }
  bool applyTransactionPatch(
      core::ports::importing::draft::TransactionDraft &,
      const core::ports::importing::draft::TransactionDraftPatch &) const override {
    return false;
  }
  int insertTransactionAfter(
      core::ports::importing::draft::StatementDraft &,
      int) const override {
    return -1;
  }
  int removeTransactionAt(core::ports::importing::draft::StatementDraft &,
                          int) const override {
    return -1;
  }
  bool renameStatementDraft(
      core::ports::importing::draft::StatementDraft &,
      const std::string &) const override {
    return false;
  }
  core::ports::importing::draft::StatementDraft buildStatementDraft(
      const std::string &,
      const core::ports::workspace::StatementSnapshot &,
      const core::ports::workspace::WorkspaceSnapshot &,
      const std::vector<core::ports::importing::draft::TransactionDraft> &,
      const std::string &) const override {
    return {};
  }
  core::ports::workspace::StatementDraftSnapshot
  buildImportedStatementDraftSnapshot(
      const std::string &,
      const std::string &,
      const core::ports::workspace::StatementSnapshot &,
      const std::vector<core::ports::importing::draft::TransactionDraft> &)
      const override {
    return {};
  }
  core::ports::importing::draft::StatementDraft restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &) const override {
    return {};
  }
  core::ports::workspace::StatementDraftSnapshot buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft &,
      const core::ports::workspace::WorkspaceSnapshot &) const override {
    return {};
  }
  std::string resolveActorId(const core::ports::workspace::WorkspaceSnapshot &,
                             const std::string &) const override {
    return {};
  }
  std::string
  resolveContractId(const core::ports::workspace::WorkspaceSnapshot &,
                    const std::string &) const override {
    return {};
  }
  bool
  contractIsFullyAllocatable(const core::ports::workspace::WorkspaceSnapshot &,
                             const std::string &) const override {
    return false;
  }
  core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &) const override {
    return primary;
  }
  std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &) const override {
    return {};
  }
};

} // namespace

TEST(ImportStateTest, HeaderIsUsable) { SUCCEED(); }

TEST(ImportStateTest, OverviewStateAppliesDefaultPathAndFiltersManualFiles) {
  const auto reporter = std::make_shared<NoopErrorReporter>();
  const auto runner = std::make_shared<ImportRunnerStub>();
  const auto adapter = std::make_shared<adapters::ImportAdapter>(nullptr, runner);
  ImportWorkflow workflow(adapter, reporter);
  Settings settings;
  ImportViewModel state;

  settings.setImportDefaultPath(QStringLiteral("P:/imports/default.pdf"));
  state.setSettings(&settings);
  state.setImportWorkflow(&workflow);

  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/default.pdf"));
  EXPECT_TRUE(state.canStart());

  settings.setImportDefaultPath(QStringLiteral("P:/imports/updated.pdf"));
  EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/updated.pdf"));

  settings.setImportDefaultPath({});
  workflow.setSelectedFile({});
  state.setManualPathText(QStringLiteral("P:/imports/readme.txt"));
  state.addSelectedImportFiles();

  EXPECT_TRUE(workflow.selectedFile().isEmpty());
  EXPECT_TRUE(workflow.queuedFiles().isEmpty());

  state.setManualPathText(QStringLiteral("P:/imports/statement.PDF"));
  state.addSelectedImportFiles();

  EXPECT_EQ(workflow.selectedFile(),
            QStringLiteral("P:/imports/statement.PDF"));
  EXPECT_EQ(state.importFileSummary(),
            QStringLiteral("Selected: statement.PDF"));
  EXPECT_TRUE(state.manualPathText().isEmpty());

  settings.setImportDefaultPath(QStringLiteral("P:/imports/later-default.pdf"));
  EXPECT_EQ(workflow.selectedFile(),
            QStringLiteral("P:/imports/statement.PDF"));
}

TEST(ImportStateTest, PauseGatesProgressUpdates) {
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
