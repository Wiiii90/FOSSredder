/**
 * @file core/include/core/application/import/StatementImportRunner.h
 * @brief Declares the application-level asynchronous statement import runner.
 */

#pragma once

#include "core/ports/usecases/import/IImportRunner.h"

#include <memory>
#include <string>
#include <vector>

namespace core::application::importing {
class IImportStatement;
}
namespace core::ports::diagnostics {
class IErrorReporter;
}
namespace core::jobs {
class JobSystem;
}

namespace core::application::importing {

class StatementImportRunner final
    : public core::ports::importing::IImportRunner {
public:
  explicit StatementImportRunner(
      std::shared_ptr<IImportStatement> importService,
      std::string runBasePath,
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);
  explicit StatementImportRunner(
      std::shared_ptr<core::jobs::JobSystem> jobSystem,
      std::string runBasePath,
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);
  ~StatementImportRunner() override;

  StatementImportRunner(const StatementImportRunner &) = delete;
  StatementImportRunner &operator=(const StatementImportRunner &) = delete;

  [[nodiscard]] core::ports::importing::StatementImportHandle
  startStatementImport(
      const core::ports::importing::ImportRequest &request,
      core::ports::importing::StatementImportEventCallback callback) override;
  void unsubscribe(
      const core::ports::importing::StatementImportHandle &handle) override;
  void cancel(
      const core::ports::importing::StatementImportHandle &handle) override;
  void pause(
      const core::ports::importing::StatementImportHandle &handle) override;
  void resume(
      const core::ports::importing::StatementImportHandle &handle) override;

  [[nodiscard]] core::ports::importing::ImportResult
  importResult(
      const core::ports::importing::StatementImportHandle &handle) override;

  [[nodiscard]] core::ports::importing::draft::DraftDerivedState
  buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::DraftLinkSelection &selection) const
      override;
  bool updateTransactionDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraftEdit &edit)
      const override;
  core::ports::importing::draft::StatementDraftEditResult
  updateStatementDraft(
      core::ports::importing::draft::StatementDraft &draft,
      const core::ports::importing::draft::StatementDraftEdit &edit)
      const override;
  [[nodiscard]] core::ports::importing::draft::StatementDraft
  buildStatementDraft(
      const std::string &sourceFile,
      const core::ports::workspace::StatementSnapshot &statement,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions,
      const std::string &draftId) const override;
  [[nodiscard]] core::ports::importing::draft::StatementDraft
  restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft) const
      override;
  [[nodiscard]] core::ports::workspace::StatementDraftSnapshot
  buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state) const override;
  [[nodiscard]] core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &secondary) const override;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace core::application::importing
