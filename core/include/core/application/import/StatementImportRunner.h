/**
 * @file core/include/core/application/import/StatementImportRunner.h
 * @brief Declares the application-level asynchronous statement import runner.
 */

#pragma once

#include "core/ports/import/IImportRunner.h"

#include <memory>
#include <string>
#include <vector>

namespace core::application::importing {
class IImportStatement;
}
namespace core::errors {
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
      std::string runBasePath = {},
      std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);
  explicit StatementImportRunner(
      std::shared_ptr<core::jobs::JobSystem> jobSystem,
      std::string runBasePath = {},
      std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);
  ~StatementImportRunner() override;

  StatementImportRunner(const StatementImportRunner &) = delete;
  StatementImportRunner &operator=(const StatementImportRunner &) = delete;

  [[nodiscard]] core::ports::importing::StatementImportHandle
  startStatementImport(
      const core::ports::importing::StatementImportStartRequest &request,
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

  [[nodiscard]] core::ports::importing::draft::DraftImportSuggestions
  buildImportSuggestions(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraft &transaction) const
      override;
  [[nodiscard]] core::ports::importing::draft::DraftTextSignals
  buildDraftTextSignals(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraft &transaction) const
      override;
  [[nodiscard]] core::ports::importing::draft::DraftDerivedState
  buildDraftDerivedState(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::DraftLinkSelection &selection) const
      override;
  bool applyDerivedSelections(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::importing::draft::DraftDerivedState &derived,
      core::ports::importing::draft::DraftAutoSelectionMode mode) const
      override;
  bool applyActorSelection(
      core::ports::importing::draft::TransactionDraft &draft,
      const std::string &actorId) const override;
  bool clearActorSelection(
      core::ports::importing::draft::TransactionDraft &draft) const override;
  bool applyPropertySelection(
      core::ports::importing::draft::TransactionDraft &draft,
      const std::string &propertyId) const override;
  bool setPropertySelected(
      core::ports::importing::draft::TransactionDraft &draft,
      const std::string &propertyId,
      bool selected) const override;
  bool applyContractSelection(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::importing::draft::DraftChoiceRow &contract) const
      override;
  bool applyContractSelection(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &contractId) const override;
  bool clearContractSelection(
      core::ports::importing::draft::TransactionDraft &draft) const override;
  bool applyTransactionPatch(
      core::ports::importing::draft::TransactionDraft &draft,
      const core::ports::importing::draft::TransactionDraftPatch &patch) const
      override;
  int insertTransactionAfter(
      core::ports::importing::draft::StatementDraft &draft,
      int currentIndex) const override;
  int removeTransactionAt(core::ports::importing::draft::StatementDraft &draft,
                          int index) const override;
  bool renameStatementDraft(
      core::ports::importing::draft::StatementDraft &draft,
      const std::string &name) const override;
  [[nodiscard]] core::ports::importing::draft::StatementDraft
  buildStatementDraft(
      const std::string &sourceFile,
      const core::ports::workspace::StatementSnapshot &statement,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions,
      const std::string &draftId) const override;
  [[nodiscard]] core::ports::workspace::StatementDraftSnapshot
  buildImportedStatementDraftSnapshot(
      const std::string &sourceFile,
      const std::string &draftId,
      const core::ports::workspace::StatementSnapshot &statement,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions) const override;
  [[nodiscard]] core::ports::importing::draft::StatementDraft
  restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft) const
      override;
  [[nodiscard]] core::ports::workspace::StatementDraftSnapshot
  buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state) const override;
  [[nodiscard]] std::string resolveActorId(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &text) const override;
  [[nodiscard]] std::string resolveContractId(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &text) const override;
  [[nodiscard]] bool contractIsFullyAllocatable(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &contractId) const override;
  [[nodiscard]] core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &secondary) const override;
  [[nodiscard]] std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &metadata) const override;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace core::application::importing
