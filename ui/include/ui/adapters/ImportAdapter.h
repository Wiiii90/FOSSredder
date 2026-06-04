/**
 * @file ui/include/ui/adapters/ImportAdapter.h
 * @brief Declares the import adapter used by UI import workflows.
 */

#pragma once

#include <memory>

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "core/ports/usecases/import/IImportRunner.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui::adapters {

class ImportAdapter final {
public:
  explicit ImportAdapter(
      std::shared_ptr<core::ports::importing::IImportRunner> runner);

  core::ports::importing::StatementImportHandle startStatementImport(
      const core::ports::importing::ImportRequest& request,
      core::ports::importing::StatementImportEventCallback callback);
  void unsubscribe(
      const core::ports::importing::StatementImportHandle& handle);
  void
  cancel(const core::ports::importing::StatementImportHandle& handle);
  void
  pause(const core::ports::importing::StatementImportHandle& handle);
  void
  resume(const core::ports::importing::StatementImportHandle& handle);
  core::ports::importing::ImportResult importResult(
      const core::ports::importing::StatementImportHandle& handle);

  core::ports::importing::draft::DraftDerivedState buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot& state,
      const core::ports::importing::draft::DraftLinkSelection& selection)
      const;
  bool updateTransactionDraft(
      core::ports::importing::draft::TransactionDraft& draft,
      const core::ports::workspace::WorkspaceSnapshot& state,
      const core::ports::importing::draft::TransactionDraftEdit& edit) const;
  core::ports::importing::draft::StatementDraftEditResult updateStatementDraft(
      core::ports::importing::draft::StatementDraft& draft,
      const core::ports::importing::draft::StatementDraftEdit& edit) const;
  core::ports::importing::draft::StatementDraft buildStatementDraft(
      const std::string& sourceFile,
      const core::ports::workspace::StatementSnapshot& statement,
      const core::ports::workspace::WorkspaceSnapshot& state,
      const std::vector<core::ports::importing::draft::TransactionDraft>&
          transactions,
      const std::string& draftId) const;
  core::ports::importing::draft::StatementDraft restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot& draft)
      const;
  core::ports::workspace::StatementDraftSnapshot buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft& draft,
      const core::ports::workspace::WorkspaceSnapshot& state) const;
  core::ports::workspace::WorkspaceSnapshot
  mergeWorkspaceState(core::ports::workspace::WorkspaceSnapshot primary,
                      const core::ports::workspace::WorkspaceSnapshot&
                          secondary) const;

  static core::ports::importing::draft::DraftLinkSelection
  toCoreSelection(const core::ports::importing::draft::TransactionDraft& draft);
  static QVariantMap
  toViewState(const core::ports::importing::draft::DraftDerivedState& derived);
  static TransactionDraftView toTransactionDraftView(
      const core::ports::importing::draft::TransactionDraft& draft,
      const QVariantMap& viewState);

private:
  std::shared_ptr<core::ports::importing::IImportRunner> runner_;
};

} // namespace ui::adapters
