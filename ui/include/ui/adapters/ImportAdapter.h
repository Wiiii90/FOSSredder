/**
 * @file ui/include/ui/adapters/ImportAdapter.h
 * @brief Declares the import adapter used by UI import workflows.
 */

#pragma once

#include <memory>

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "core/ports/import/IImportRunner.h"

namespace ui {
class WorkspaceFacade;
}

namespace ui::adapters {

struct TransactionDraftView {
  QString id;
  QString name;
  QString bookingDate;
  QString valuta;
  QString amountText;
  QString metadata;
  QString proofSource;
  QString actorId;
  QString contractId;
  QStringList propertyIds;
  int status = 0;
  bool effectiveAllocatable = false;
  double actorSuggestionConfidence = 0.0;
  QString actorSuggestionSummary;
  double propertySuggestionConfidence = 0.0;
  QString propertySuggestionSummary;
  double contractSuggestionConfidence = 0.0;
  QString contractSuggestionSummary;
  double allocatableSuggestionConfidence = 0.0;
  QString allocatableSuggestionSummary;
};

class ImportAdapter final : public core::ports::importing::IImportRunner {
public:
  ImportAdapter(
      ui::WorkspaceFacade *uiWorkspace,
      std::shared_ptr<core::ports::importing::IImportRunner> runner);

  std::shared_ptr<core::ports::importing::IImportRunner>
  runner() const noexcept {
    return runner_;
  }

  core::ports::importing::StatementImportHandle startStatementImport(
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
  core::ports::importing::ImportResult importResult(
      const core::ports::importing::StatementImportHandle &handle) override;

  core::ports::importing::draft::DraftImportSuggestions buildImportSuggestions(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraft &transaction) const
      override;
  core::ports::importing::draft::DraftTextSignals buildDraftTextSignals(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const core::ports::importing::draft::TransactionDraft &transaction) const
      override;
  core::ports::importing::draft::DraftDerivedState buildDraftDerivedState(
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
  core::ports::importing::draft::StatementDraft buildStatementDraft(
      const std::string &sourceFile,
      const core::ports::workspace::StatementSnapshot &statement,
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions,
      const std::string &draftId) const override;
  core::ports::workspace::StatementDraftSnapshot
  buildImportedStatementDraftSnapshot(
      const std::string &sourceFile,
      const std::string &draftId,
      const core::ports::workspace::StatementSnapshot &statement,
      const std::vector<core::ports::importing::draft::TransactionDraft>
          &transactions) const override;
  core::ports::importing::draft::StatementDraft restoreStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft) const
      override;
  core::ports::workspace::StatementDraftSnapshot buildStatementDraftSnapshot(
      const core::ports::importing::draft::StatementDraft &draft,
      const core::ports::workspace::WorkspaceSnapshot &state) const override;
  std::string resolveActorId(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &text) const override;
  std::string resolveContractId(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &text) const override;
  bool contractIsFullyAllocatable(
      const core::ports::workspace::WorkspaceSnapshot &state,
      const std::string &contractId) const override;
  core::ports::workspace::WorkspaceSnapshot mergeWorkspaceState(
      core::ports::workspace::WorkspaceSnapshot primary,
      const core::ports::workspace::WorkspaceSnapshot &secondary) const
      override;
  std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &metadata) const override;

  QVariantMap createActorForDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const QString &actorName);
  QVariantMap createPropertyForDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const QString &propertyName);
  QVariantMap createOrSelectContractForDraft(
      core::ports::importing::draft::TransactionDraft &draft,
      const QString &contractName, const QString &contractType,
      const QString &allocatableMode);
  static core::ports::importing::draft::DraftLinkSelection toCoreSelection(
      const core::ports::importing::draft::TransactionDraft &draft);
  static QVariantMap toViewState(
      const core::ports::importing::draft::DraftDerivedState &derived);
  static TransactionDraftView toTransactionDraftView(
      const core::ports::importing::draft::TransactionDraft &draft,
      const QVariantMap &viewState);

private:
  QVariantMap actorIdentityByName(const QString &name) const;
  QVariantMap propertyIdentityByName(const QString &name) const;
  QVariantMap contractIdentityBySignature(
      const QString &name, const QString &type, const QStringList &actorIds,
      const QStringList &propertyIds) const;
  QString nextContractName() const;
  QString saveActor(const QString &name);
  QString saveProperty(const QString &name);
  QString saveContract(const QString &name, const QString &type,
                       const QStringList &actorIds,
                       const QStringList &propertyIds,
                       const QString &allocatableMode);

  ui::WorkspaceFacade *uiWorkspace_ = nullptr;
  std::shared_ptr<core::ports::importing::IImportRunner> runner_;
};

} // namespace ui::adapters
