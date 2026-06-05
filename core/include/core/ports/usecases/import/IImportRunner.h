/**
 * @file core/include/core/ports/usecases/import/IImportRunner.h
 * @brief Import runner port consumed by UI workflows.
 */

#pragma once

#include "core/ports/usecases/import/ImportRequest.h"
#include "core/ports/usecases/import/ImportResult.h"
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
  /**
   * @brief Destroys the import runner port.
   */
  virtual ~IImportRunner() = default;

  /**
   * @brief Starts an asynchronous statement import.
   * @param request Import source and configuration.
   * @param callback Callback receiving import progress events.
   * @return Handle used to control and query the started import.
   */
  [[nodiscard]] virtual StatementImportHandle
  startStatementImport(const ImportRequest &request,
                       StatementImportEventCallback callback) = 0;

  /**
   * @brief Removes a progress subscription for an import.
   * @param handle Import handle containing the subscription to remove.
   */
  virtual void unsubscribe(const StatementImportHandle &handle) = 0;

  /**
   * @brief Requests cancellation of an import.
   * @param handle Import handle to cancel.
   */
  virtual void cancel(const StatementImportHandle &handle) = 0;

  /**
   * @brief Pauses a running import.
   * @param handle Import handle to pause.
   */
  virtual void pause(const StatementImportHandle &handle) = 0;

  /**
   * @brief Resumes a paused import.
   * @param handle Import handle to resume.
   */
  virtual void resume(const StatementImportHandle &handle) = 0;

  /**
   * @brief Returns the completed import result for a handle.
   * @param handle Import handle to query.
   * @return Import result for the handle, or an empty result when unavailable.
   */
  [[nodiscard]] virtual ImportResult
  importResult(const StatementImportHandle &handle) = 0;

  /**
   * @brief Builds derived selection state for a draft link selection.
   * @param state Workspace snapshot used as catalog context.
   * @param selection Current draft link selection.
   * @return Derived selection state for UI presentation and auto-selection.
   */
  [[nodiscard]] virtual core::ports::importing::draft::DraftDerivedState
  buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::DraftLinkSelection &selection) const = 0;

  /**
   * @brief Applies one edit command to a transaction draft.
   * @param draft Draft to mutate.
   * @param state Workspace snapshot used when catalog context is required.
   * @param edit Edit command to apply.
   * @return True when the draft changed.
   */
  virtual bool updateTransactionDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraftEdit &edit)
      const = 0;

  /**
   * @brief Applies one edit command to a statement draft.
   * @param draft Draft to mutate.
   * @param edit Edit command to apply.
   * @return Edit result including the next selected transaction index.
   */
  virtual core::ports::importing::draft::StatementDraftEditResult
  updateStatementDraft(
      core::ports::importing::draft::StatementDraft &draft,
      const core::ports::importing::draft::StatementDraftEdit &edit)
      const = 0;

  /**
   * @brief Builds a statement draft from imported statement data.
   * @param sourceFile Imported source file path.
   * @param statement Imported statement snapshot.
   * @param state Workspace snapshot used as catalog context.
   * @param transactions Imported transaction drafts.
   * @param draftId Draft identifier to assign.
   * @return Built statement draft.
   */
  [[nodiscard]] virtual core::ports::importing::draft::StatementDraft
  buildStatementDraft(
      const std::string &sourceFile,
      const core::ports::workspace::StatementSnapshot &statement,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions,
      const std::string &draftId) const = 0;

  /**
   * @brief Restores a runtime statement draft from a workspace draft snapshot.
   * @param draft Workspace statement draft snapshot.
   * @return Restored statement draft.
   */
  [[nodiscard]] virtual core::ports::importing::draft::StatementDraft
  restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft) const = 0;

  /**
   * @brief Converts a runtime statement draft to a workspace draft snapshot.
   * @param draft Statement draft to convert.
   * @param state Workspace snapshot used as catalog context.
   * @return Workspace statement draft snapshot.
   */
  [[nodiscard]] virtual core::ports::workspace::StatementDraftSnapshot
  buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state) const = 0;

  /**
   * @brief Merges two workspace snapshots for import matching context.
   * @param primary Primary workspace snapshot.
   * @param secondary Secondary workspace snapshot.
   * @return Merged workspace snapshot.
   */
  [[nodiscard]] virtual core::ports::workspace::WorkspaceSnapshot
  mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &secondary) const = 0;

};

} // namespace core::ports::importing
