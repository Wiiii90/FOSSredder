/**
 * @file ui/tests/unit/TestImportWorkflow.cpp
 * @brief Tests for import workflow draft session behavior.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/errors/IErrorReporter.h"
#include "core/ports/import/IImportRunner.h"
#include "core/ports/import/ImportDraft.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "support/WorkspacePortFakes.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace {

using core::ports::importing::draft::StatementDraft;
using core::ports::importing::draft::TransactionDraft;

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
      core::ports::importing::draft::TransactionDraft &draft,
      const std::string &propertyId,
      bool selected) const override {
    if (selected) {
      draft.propertyIds.push_back(QString::fromStdString(propertyId));
    } else {
      draft.propertyIds.removeAll(QString::fromStdString(propertyId));
    }
    if (!draft.propertyIds.isEmpty()) {
      draft.contractId.clear();
      draft.contractSelected = false;
    }
    return true;
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
      core::ports::importing::draft::StatementDraft &draft,
      int currentIndex) const override {
    if (draft.transactions.empty()) {
      return -1;
    }
    TransactionDraft row;
    row.id = QStringLiteral("inserted");
    draft.transactions.insert(
        draft.transactions.begin() +
            static_cast<std::ptrdiff_t>(currentIndex + 1),
        row);
    return currentIndex + 1;
  }

  int removeTransactionAt(core::ports::importing::draft::StatementDraft &draft,
                          int index) const override {
    if (draft.transactions.size() <= 1 || index < 0 ||
        static_cast<std::size_t>(index) >= draft.transactions.size()) {
      return -1;
    }
    draft.transactions.erase(
        draft.transactions.begin() + static_cast<std::ptrdiff_t>(index));
    return std::min(index, static_cast<int>(draft.transactions.size()) - 1);
  }

  bool renameStatementDraft(
      core::ports::importing::draft::StatementDraft &draft,
      const std::string &name) const override {
    draft.name = name;
    return true;
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
      const core::ports::workspace::StatementDraftSnapshot &snapshot) const override {
    core::ports::importing::draft::StatementDraft draft;
    draft.id = snapshot.id;
    draft.name = snapshot.name;
    draft.transactionIds = snapshot.transactionIds;
    draft.createdAt = snapshot.createdAt;
    draft.updatedAt = snapshot.updatedAt;
    draft.transactions.reserve(snapshot.transactions.size());
    for (const auto &tx : snapshot.transactions) {
      core::ports::importing::draft::TransactionDraft transaction;
      transaction.id = tx.id;
      transaction.statementDraftId = tx.statementDraftId;
      transaction.name = tx.name;
      transaction.bookingDate = tx.bookingDate;
      transaction.valuta = tx.valuta;
      transaction.amount = tx.amount;
      transaction.actorId = tx.actorId;
      transaction.contractId = tx.contractId;
      transaction.propertyIds = tx.propertyIds;
      transaction.status = tx.status;
      transaction.allocatable = tx.allocatable;
      transaction.position = tx.position;
      transaction.metadata = tx.metadata;
      transaction.proofImageData = tx.proofImageData;
      draft.transactions.push_back(std::move(transaction));
    }
    std::sort(draft.transactions.begin(), draft.transactions.end(),
              [](const auto &lhs, const auto &rhs) {
                return lhs.position < rhs.position;
              });
    return draft;
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

core::ports::workspace::WorkspaceSnapshot makeStateWithDraftStack() {
  core::ports::workspace::WorkspaceSnapshot state;
  for (int i = 1; i <= 3; ++i) {
    const auto id = std::string("draft-") + std::to_string(i);

    core::ports::workspace::StatementDraftSnapshot statementDraft;
    statementDraft.id = id;
    statementDraft.name = std::string("Draft ") + std::to_string(i);

    for (int position = 0; position < 2; ++position) {
      core::ports::workspace::TransactionDraftSnapshot transactionDraft;
      transactionDraft.id = std::string("tx-") + std::to_string(i) + "-" +
                            std::to_string(position);
      transactionDraft.statementDraftId = id;
      transactionDraft.position = position;
      transactionDraft.name = std::string("Tx ") + std::to_string(i);
      transactionDraft.metadata = std::string("metadata-") + std::to_string(i);
      transactionDraft.proofImageData = {static_cast<std::uint8_t>(i)};
      statementDraft.transactions.push_back(std::move(transactionDraft));
    }
    state.statementDrafts.push_back(std::move(statementDraft));

    core::ports::workspace::ImportLogSnapshot log;
    log.id = id;
    log.time = "2026-05-15 10:00:00";
    log.type = "statement";
    log.file = "/tmp/import.pdf";
    log.status = "Draft";
    log.draftAttached = true;
    log.draftId = id;
    state.importLogs.push_back(std::move(log));
  }
  return state;
}

} // namespace

namespace ui {

TEST(ImportWorkflowTest, DraftStackNavigationCyclesThroughImportHome) {
  tests::support::InMemoryWorkspace workspace(makeStateWithDraftStack());
  ui::WorkspaceFacade facade(&workspace, &workspace);
  const auto reporter = std::make_shared<NoopErrorReporter>();
  const auto runner = std::make_shared<ImportRunnerStub>();
  const auto adapter =
      std::make_shared<ui::adapters::ImportAdapter>(&facade, runner);
  ui::ImportWorkflow workflow(adapter, reporter, &facade);
  ui::ImportViewModel viewModel;
  viewModel.setImportWorkflow(&workflow);
  viewModel.setWorkspace(&facade);

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

TEST(ImportWorkflowTest, ReopenedDraftRestoresRememberedTransactionIndex) {
  tests::support::InMemoryWorkspace workspace(makeStateWithDraftStack());
  ui::WorkspaceFacade facade(&workspace, &workspace);
  const auto reporter = std::make_shared<NoopErrorReporter>();
  const auto runner = std::make_shared<ImportRunnerStub>();
  const auto adapter =
      std::make_shared<ui::adapters::ImportAdapter>(&facade, runner);
  ui::ImportWorkflow workflow(adapter, reporter, &facade);

  ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  workflow.setCurrentTransactionIndex(1);

  workflow.clearDraft();
  ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
  ASSERT_TRUE(workflow.hasDraft());
  EXPECT_EQ(workflow.currentTransactionIndex(), 1);
}

TEST(ImportAdapterTest, PropertySelectionClearsSelectedContract) {
  tests::support::InMemoryWorkspace workspace;
  ui::WorkspaceFacade facade(&workspace, &workspace);
  const auto adapter = std::make_shared<ui::adapters::ImportAdapter>(
      &facade, std::make_shared<ImportRunnerStub>());

  TransactionDraft transaction;
  transaction.contractId = QStringLiteral("contract-1");
  transaction.contractSelected = true;

  ASSERT_TRUE(adapter->setPropertySelected(transaction, "property-1", true));
  EXPECT_TRUE(transaction.contractId.isEmpty());
  EXPECT_FALSE(transaction.contractSelected);
}

TEST(StatementDraftViewModelTest, TransactionNavigationWrapsAtBothEdges) {
  StatementDraft draft;
  std::vector<TransactionDraft> rows(3);
  draft.setDrafts(std::move(rows));

  draft.setCurrentIndex(2);
  draft.next();
  EXPECT_EQ(draft.currentIndex(), 0);

  draft.prev();
  EXPECT_EQ(draft.currentIndex(), 2);
}

} // namespace ui
