/**
 * @file core/include/core/ports/import/IImportRunner.h
 * @brief Import runner port consumed by UI workflows.
 */

#pragma once

#include "core/ports/import/ImportResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace core::ports::importing {

enum class StatementImportState {
  Pending,
  Running,
  Paused,
  Finished,
  Failed,
  Canceled,
};

struct StatementImportStartRequest {
  std::string sourcePath;
};

struct StatementImportEvent {
  std::string importId;
  StatementImportState state = StatementImportState::Pending;
  double progress = 0.0;
  std::string message;
};

struct StatementImportHandle {
  std::string importId;
  std::uint64_t subscriptionId = 0;
};

using StatementImportEventCallback =
    std::function<void(const StatementImportEvent &)>;

class IImportRunner {
public:
  virtual ~IImportRunner() = default;

  [[nodiscard]] virtual StatementImportHandle
  startStatementImport(const StatementImportStartRequest &request,
                       StatementImportEventCallback callback) = 0;
  virtual void unsubscribe(const StatementImportHandle &handle) = 0;
  virtual void cancel(const StatementImportHandle &handle) = 0;
  virtual void pause(const StatementImportHandle &handle) = 0;
  virtual void resume(const StatementImportHandle &handle) = 0;

  [[nodiscard]] virtual ImportResult
  importResult(const StatementImportHandle &handle) = 0;

  [[nodiscard]] virtual draft::DraftImportSuggestions
  buildImportSuggestions(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const draft::TransactionDraft &transaction) const = 0;

  [[nodiscard]] virtual draft::DraftTextSignals
  buildDraftTextSignals(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const draft::TransactionDraft &transaction) const = 0;

  [[nodiscard]] virtual draft::DraftDerivedState
  buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const draft::DraftLinkSelection &selection) const = 0;

  virtual bool applyDerivedSelections(
      draft::TransactionDraft &draft,
      const draft::DraftDerivedState &derived,
      draft::DraftAutoSelectionMode mode) const = 0;

  virtual bool applyActorSelection(draft::TransactionDraft &draft,
                                   const std::string &actorId) const = 0;

  virtual bool clearActorSelection(draft::TransactionDraft &draft) const = 0;

  virtual bool applyPropertySelection(draft::TransactionDraft &draft,
                                      const std::string &propertyId) const = 0;

  virtual bool setPropertySelected(draft::TransactionDraft &draft,
                                   const std::string &propertyId,
                                   bool selected) const = 0;

  virtual bool applyContractSelection(
      draft::TransactionDraft &draft,
      const draft::DraftChoiceRow &contract) const = 0;

  virtual bool applyContractSelection(
      draft::TransactionDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &contractId) const = 0;

  virtual bool clearContractSelection(draft::TransactionDraft &draft) const = 0;

  virtual bool applyTransactionPatch(
      draft::TransactionDraft &draft,
      const draft::TransactionDraftPatch &patch) const = 0;

  virtual int insertTransactionAfter(draft::StatementDraft &draft,
                                     int currentIndex) const = 0;

  virtual int removeTransactionAt(draft::StatementDraft &draft,
                                  int index) const = 0;

  virtual bool renameStatementDraft(draft::StatementDraft &draft,
                                    const std::string &name) const = 0;

  [[nodiscard]] virtual draft::StatementDraft
  buildStatementDraft(
      const std::string &sourceFile,
      const core::ports::workspace::StatementSnapshot &statement,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::vector<draft::TransactionDraft> &transactions,
      const std::string &draftId) const = 0;

  [[nodiscard]] virtual core::ports::workspace::StatementDraftSnapshot
  buildImportedStatementDraftSnapshot(
      const std::string &sourceFile,
      const std::string &draftId,
      const core::ports::workspace::StatementSnapshot &statement,
      const std::vector<draft::TransactionDraft> &transactions) const = 0;

  [[nodiscard]] virtual draft::StatementDraft
  restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft) const = 0;

  [[nodiscard]] virtual core::ports::workspace::StatementDraftSnapshot
  buildStatementDraftSnapshot(
      const draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state) const = 0;

  [[nodiscard]] virtual std::string
  resolveActorId(const core::ports::workspace::WorkspaceSnapshot &state,
                 const std::string &text) const = 0;

  [[nodiscard]] virtual std::string
  resolveContractId(const core::ports::workspace::WorkspaceSnapshot &state,
                    const std::string &text) const = 0;

  [[nodiscard]] virtual bool
  contractIsFullyAllocatable(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &contractId) const = 0;

  [[nodiscard]] virtual core::ports::workspace::WorkspaceSnapshot
  mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &secondary) const = 0;

  [[nodiscard]] virtual std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &metadata) const = 0;
};

} // namespace core::ports::importing
