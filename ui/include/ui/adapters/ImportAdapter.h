/**
 * @file ui/include/ui/adapters/ImportAdapter.h
 * @brief Declares the import adapter used by UI import workflows.
 */

#pragma once

#include <memory>

#include "core/ports/usecases/import/IImportRunner.h"

namespace ui {
struct TransactionDraftView;
} // namespace ui

namespace ui::adapters {

class ImportAdapter final {
public:
  /**
   * @brief Creates an adapter backed by the core import runner port.
   * @param runner Core import runner used for jobs and draft operations.
   */
  explicit ImportAdapter(
      std::shared_ptr<core::ports::importing::IImportRunner> runner);

  /**
   * @brief Starts an asynchronous statement import through the runner.
   * @param request Import request built by the workflow.
   * @param callback Event callback receiving progress and terminal events.
   * @return Import handle used for pause, cancel, result, and unsubscribe.
   */
  core::ports::importing::StatementImportHandle startStatementImport(
      const core::ports::importing::ImportRequest& request,
      core::ports::importing::StatementImportEventCallback callback);
  /**
   * @brief Removes an import event subscription.
   * @param handle Import handle returned by startStatementImport().
   */
  void unsubscribe(const core::ports::importing::StatementImportHandle& handle);
  /**
   * @brief Requests cancellation of a running import.
   * @param handle Import handle returned by startStatementImport().
   */
  void cancel(const core::ports::importing::StatementImportHandle& handle);
  /**
   * @brief Requests pausing of a running import.
   * @param handle Import handle returned by startStatementImport().
   */
  void pause(const core::ports::importing::StatementImportHandle& handle);
  /**
   * @brief Requests resuming of a paused import.
   * @param handle Import handle returned by startStatementImport().
   */
  void resume(const core::ports::importing::StatementImportHandle& handle);
  /**
   * @brief Reads the final import result for a handle.
   * @param handle Import handle returned by startStatementImport().
   * @return Import result or an empty result when no runner is available.
   */
  core::ports::importing::ImportResult
  importResult(const core::ports::importing::StatementImportHandle& handle);

  /**
   * @brief Builds derived draft suggestion state through the runner.
   * @param state Workspace snapshot used for catalog matching.
   * @param selection Current draft link selection.
   * @return Derived draft state.
   */
  core::ports::importing::draft::DraftDerivedState buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot& state,
      const core::ports::importing::draft::DraftLinkSelection& selection) const;
  /**
   * @brief Applies a transaction draft edit through the runner.
   * @param draft Transaction draft to mutate.
   * @param state Workspace snapshot used for catalog-aware edits.
   * @param edit Transaction draft edit command.
   * @return True when the draft changed.
   */
  bool updateTransactionDraft(
      core::ports::importing::draft::TransactionDraft& draft,
      const core::ports::workspace::WorkspaceSnapshot& state,
      const core::ports::importing::draft::TransactionDraftEdit& edit) const;
  /**
   * @brief Applies a statement draft edit through the runner.
   * @param draft Statement draft to mutate.
   * @param edit Statement draft edit command.
   * @return Edit result including selected transaction index.
   */
  core::ports::importing::draft::StatementDraftEditResult updateStatementDraft(
      core::ports::importing::draft::StatementDraft& draft,
      const core::ports::importing::draft::StatementDraftEdit& edit) const;
  /**
   * @brief Builds a statement draft from an imported statement result.
   * @param sourceFile Source import file.
   * @param statement Imported statement snapshot.
   * @param state Workspace snapshot used for catalog matching.
   * @param transactions Imported transaction drafts.
   * @param draftId Draft id to assign.
   * @return Statement draft.
   */
  core::ports::importing::draft::StatementDraft buildStatementDraft(
      const std::string& sourceFile,
      const core::ports::workspace::StatementSnapshot& statement,
      const core::ports::workspace::WorkspaceSnapshot& state,
      const std::vector<core::ports::importing::draft::TransactionDraft>&
          transactions,
      const std::string& draftId) const;
  /**
   * @brief Restores a statement draft from workspace draft state.
   * @param draft Workspace draft snapshot.
   * @return Statement draft.
   */
  core::ports::importing::draft::StatementDraft restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot& draft) const;
  /**
   * @brief Builds workspace draft state from the active statement draft.
   * @param draft Statement draft.
   * @param state Workspace snapshot used for catalog projection.
   * @return Workspace draft snapshot.
   */
  core::ports::workspace::StatementDraftSnapshot buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft& draft,
      const core::ports::workspace::WorkspaceSnapshot& state) const;
  /**
   * @brief Merges stored draft catalog state with live workspace state.
   * @param primary Primary workspace snapshot.
   * @param secondary Secondary workspace snapshot.
   * @return Merged workspace snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot& secondary) const;

  /**
   * @brief Builds the QML transaction draft view through core-derived state.
   * @param draft Transaction draft.
   * @param state Workspace snapshot used for catalog matching.
   * @return QML-friendly transaction draft view.
   */
  TransactionDraftView transactionDraftView(
      const core::ports::importing::draft::TransactionDraft& draft,
      const core::ports::workspace::WorkspaceSnapshot& state) const;

private:
  /**
   * @brief Converts a transaction draft to a core link selection.
   * @param draft Transaction draft.
   * @return Core link selection.
   */
  static core::ports::importing::draft::DraftLinkSelection
  toCoreSelection(const core::ports::importing::draft::TransactionDraft& draft);
  /**
   * @brief Builds a QML transaction draft view from draft and derived state.
   * @param draft Transaction draft.
   * @param derived Core-derived draft state.
   * @return Transaction draft view.
   */
  static TransactionDraftView toTransactionDraftView(
      const core::ports::importing::draft::TransactionDraft& draft,
      const core::ports::importing::draft::DraftDerivedState& derived);

  std::shared_ptr<core::ports::importing::IImportRunner> runner_;
};

} // namespace ui::adapters
