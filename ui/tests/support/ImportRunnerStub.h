/**
 * @file ui/tests/support/ImportRunnerStub.h
 * @brief Shared import runner stub for UI import adapter and workflow tests.
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <QString>

#include "core/ports/usecases/import/IImportRunner.h"
#include "core/ports/usecases/import/ImportResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::tests::support {

class ImportRunnerStub final : public core::ports::importing::IImportRunner {
public:
  core::ports::importing::StatementImportHandle startStatementImport(
      const core::ports::importing::ImportRequest &,
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

  core::ports::importing::draft::DraftDerivedState buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot &,
      const core::ports::importing::draft::DraftLinkSelection &) const override {
    return {};
  }

  bool updateTransactionDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &,
      const core::ports::importing::draft::TransactionDraftEdit &edit)
      const override {
    using Kind = core::ports::importing::draft::TransactionDraftEditKind;
    switch (edit.kind) {
    case Kind::Patch:
      if (edit.patch.hasName) {
        draft.name = edit.patch.name;
      }
      if (edit.patch.hasBookingDate) {
        draft.bookingDate = edit.patch.bookingDate;
      }
      if (edit.patch.hasValuta) {
        draft.valuta = edit.patch.valuta;
      }
      if (edit.patch.hasAmount) {
        draft.amount = edit.patch.amount;
      }
      if (edit.patch.hasStatus) {
        draft.status = edit.patch.status;
      }
      if (edit.patch.hasAllocatable) {
        draft.allocatable = edit.patch.allocatable;
      }
      return true;
    case Kind::SetPropertySelected:
      if (edit.selected) {
        draft.propertyIds.push_back(edit.id);
      } else {
        draft.propertyIds.erase(
            std::remove(draft.propertyIds.begin(), draft.propertyIds.end(),
                        edit.id),
            draft.propertyIds.end());
      }
      return true;
    default:
      return false;
    }
  }

  core::ports::importing::draft::StatementDraftEditResult updateStatementDraft(
      core::ports::importing::draft::StatementDraft &draft,
      const core::ports::importing::draft::StatementDraftEdit &edit)
      const override {
    using Kind = core::ports::importing::draft::StatementDraftEditKind;
    core::ports::importing::draft::StatementDraftEditResult result;
    result.selectedTransactionIndex = edit.index;
    switch (edit.kind) {
    case Kind::Rename:
      draft.name = edit.text;
      result.changed = true;
      break;
    case Kind::InsertTransactionAfter:
      if (!draft.transactions.empty()) {
        core::ports::importing::draft::TransactionDraft row;
        row.id = "inserted";
        draft.transactions.insert(
            draft.transactions.begin() +
                static_cast<std::ptrdiff_t>(edit.index + 1),
            row);
        result.selectedTransactionIndex = edit.index + 1;
        result.changed = true;
      }
      break;
    case Kind::RemoveTransactionAt:
      if (draft.transactions.size() > 1 && edit.index >= 0 &&
          static_cast<std::size_t>(edit.index) < draft.transactions.size()) {
        draft.transactions.erase(draft.transactions.begin() +
                                 static_cast<std::ptrdiff_t>(edit.index));
        result.selectedTransactionIndex =
            std::min(edit.index, static_cast<int>(draft.transactions.size()) - 1);
        result.changed = true;
      }
      break;
    }
    return result;
  }

  core::ports::importing::draft::StatementDraft buildStatementDraft(
      const std::string &,
      const core::ports::workspace::StatementSnapshot &,
      const core::ports::workspace::WorkspaceSnapshot &,
      const std::vector<core::ports::importing::draft::TransactionDraft> &,
      const std::string &) const override {
    return {};
  }

  core::ports::importing::draft::StatementDraft restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &snapshot)
      const override {
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
      const core::ports::importing::draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &) const override {
    core::ports::workspace::StatementDraftSnapshot snapshot;
    snapshot.id = draft.id;
    snapshot.name = draft.name;
    snapshot.transactionIds = draft.transactionIds;
    for (std::size_t i = 0; i < draft.transactions.size(); ++i) {
      const auto &tx = draft.transactions[i];
      core::ports::workspace::TransactionDraftSnapshot row;
      row.id = tx.id;
      row.statementDraftId = tx.statementDraftId;
      row.name = tx.name;
      row.bookingDate = tx.bookingDate;
      row.valuta = tx.valuta;
      row.amount = tx.amount;
      row.position = static_cast<int>(i);
      snapshot.transactions.push_back(std::move(row));
    }
    return snapshot;
  }

  core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &) const override {
    return primary;
  }
};

inline core::ports::workspace::WorkspaceSnapshot makeStateWithDraftStack() {
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

} // namespace ui::tests::support
