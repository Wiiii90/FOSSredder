/**
 * @file ui/include/ui/workspace/WorkspaceFacade.h
 * @brief Declares the aggregated workspace and selection facade exposed to QML.
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/WorkspaceCache.h"
#include "ui/workspace/WorkspaceSelection.h"

namespace ui {

class ImportWorkflow;

/**
 * @brief Aggregates workspace data and current selection state for the UI.
 */
class WorkspaceFacade : public QObject {
  Q_OBJECT

public:
  /** @brief Creates the facade and its owned workspace state objects. */
  explicit WorkspaceFacade(QObject* parent = nullptr);
  /** @brief Creates the facade and binds it to workspace read/write ports. */
  explicit WorkspaceFacade(
      core::ports::workspace::IWorkspaceWriter* workspaceWriter,
      core::ports::workspace::IWorkspaceReader* workspaceReader,
      QObject* parent = nullptr);

  /** @brief Returns the workspace cache that owns UI model collections and
   * metrics. */
  WorkspaceCache* cache() noexcept;
  /** @brief Returns the selection state synchronized with the current cache
   * models. */
  WorkspaceSelection* selection() noexcept;

  /** @brief Loads the UI cache from the supplied application state snapshot.
   */
  void loadFromState(const core::ports::workspace::WorkspaceSnapshot& state);
  /** @brief Binds the facade to workspace ports used for writes and snapshots.
   */
  void
  setWorkspacePorts(core::ports::workspace::IWorkspaceWriter* workspaceWriter,
                    core::ports::workspace::IWorkspaceReader* workspaceReader);

  QString currentPath() const;
  core::ports::workspace::WorkspaceSnapshot workspaceSnapshot() const;
  std::optional<core::ports::workspace::StatementDraftSnapshot>
  statementDraftSnapshot(const QString& draftId = {}) const;
  QString finalizeStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot& draft);
  void saveStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot& draft);
  void clearStatementDraft(const QString& draftId = {});
  void saveImportLog(const core::ports::workspace::ImportLogSnapshot& log);
  void upsertImportLog(const QString& logId, const QString& status,
                       const QString& message, bool draftAttached,
                       const QString& draftId = {},
                       const QString& statementId = {},
                       const QString& importFile = {});
  void deleteImportLog(const QString& id);
  void clearImportLogs();
  /** @brief Returns import logs projected as QML sidebar rows. */
  QVariantList importLogRows() const;
  /** @brief Returns draft ids for import logs that still have an attached
   * draft.
   */
  QStringList attachedImportDraftIds() const;
  void saveExportLog(const core::ports::workspace::ExportLogSnapshot& log);
  void deleteExportLog(const QString& id);
  void deleteExportLogAt(int index);
  void clearExportLogs();
  /** @brief Returns export logs projected as QML sidebar rows. */
  QVariantList exportLogRows() const;
  QString exportLogTargetPath(const QString& id, int fallbackIndex = -1) const;
  QVariantMap actorIdentityByName(const QString& name) const;
  QVariantMap propertyIdentityByName(const QString& name) const;
  QVariantMap contractIdentityBySignature(const QString& name,
                                          const QString& type,
                                          const QStringList& actorIds,
                                          const QStringList& propertyIds) const;
  QString nextContractName() const;
  QString selectedActorId() const;
  QString selectedPropertyId() const;
  QString selectedContractId() const;
  QString selectedStatementId() const;
  QString selectedTransactionId() const;
  QString selectedAnalysisId() const;
  QString selectedAnnualId() const;
  void selectActor(const QString& id);
  void selectProperty(const QString& id);
  void selectContract(const QString& id);
  void selectStatement(const QString& id);
  void selectTransaction(const QString& statementId, const QString& id);
  void selectAnalysis(const QString& id);
  void selectAnnual(const QString& id);
  QVariantList actorRows() const;
  QVariantList propertyRows() const;
  QVariantList contractRows() const;
  QVariantList analysisRows() const;
  QVariantList annualRows() const;
  QVariantList statementRows() const;
  QVariantList statementRowsWithTransactions() const;
  QVariantList statementTransactionRows(const QString& statementId) const;
  QVariantMap transactionRowById(const QString& id) const;
  QVariantList actorDropdownRows() const;
  QVariantList propertyDropdownRows() const;
  QVariantList contractDropdownRows() const;
  /**
   * @brief Applies actor, contract, and property dropdown changes to a
   * transaction form map.
   * @param formData Current transaction form map.
   * @param changes Changed catalog selection fields.
   * @return Updated transaction form map with dependent catalog fields adjusted.
   */
  QVariantMap transactionFormWithCatalogSelection(
      const QVariantMap& formData, const QVariantMap& changes) const;
  double amountForTransactionCommit(const QVariant& rawAmount,
                                    const QString& transactionId,
                                    double fallbackAmount) const;
  void setTransactionPropertyIdsImmediate(const QString& transactionId,
                                          const QStringList& propertyIds);
  QVariantMap validateActor(const QString& id, const QString& name,
                            const QStringList& aliases = {},
                            const QStringList& contractIds = {}) const;
  QVariantMap validateProperty(const QString& id, const QString& name,
                               const QStringList& aliases = {},
                               const QStringList& contractIds = {}) const;
  QVariantMap validateContract(
      const QString& id, const QString& name, const QString& type,
      const QStringList& actorIds = {}, const QStringList& propertyIds = {},
      const QStringList& aliases = {},
      const QString& allocatableMode = QStringLiteral("mixed")) const;
  QVariantMap validateStatement(const QString& id, const QString& name) const;
  QVariantMap validateTransaction(const QString& id, const QString& name,
                                  const QString& bookingDate,
                                  const QString& valuta, double amount,
                                  const QString& statementId, int status,
                                  const QString& actorId,
                                  const QString& contractId,
                                  bool allocatable,
                                  const QStringList& propertyIds) const;
  void newFile(const QString& path);
  void openFile(const QString& path);
  void saveFile();
  void saveFileAs(const QString& path);
  void setImportWorkflowForSave(ImportWorkflow* workflow);

  QString saveActor(const QString& id, const QString& name,
                    const QStringList& aliases = {},
                    const QStringList& contractIds = {});
  void deleteActor(const QString& id);

  QString saveProperty(const QString& id, const QString& name,
                       const QStringList& aliases = {},
                       const QStringList& contractIds = {});
  void deleteProperty(const QString& id);

  QString
  saveContract(const QString& id, const QString& name, const QString& type,
               const QStringList& actorIds = {},
               const QStringList& propertyIds = {},
               const QStringList& aliases = {},
               const QString& allocatableMode = QStringLiteral("mixed"));
  void deleteContract(const QString& id);

  QString addStatement(const QString& name);
  void updateStatement(const QString& id, const QString& name);
  void deleteStatement(const QString& id);

  QString addTransaction(const QString& name, const QString& bookingDate,
                         const QString& valuta, double amount,
                         const QString& statementId, int status = 0,
                         const QString& actorId = QString(),
                         const QString& contractId = QString(),
                         bool allocatable = false,
                         const QStringList& propertyIds = {});
  QString insertTransactionAfter(
      const QString& afterTransactionId, const QString& name,
      const QString& bookingDate, const QString& valuta, double amount,
      const QString& statementId, int status = 0,
      const QString& actorId = QString(), const QString& contractId = QString(),
      bool allocatable = false, const QStringList& propertyIds = {});
  void updateTransaction(const QString& id, const QString& name,
                         const QString& bookingDate, const QString& valuta,
                         double amount, const QString& statementId, int status,
                         const QString& actorId, const QString& contractId,
                         bool allocatable, const QStringList& propertyIds);
  void deleteTransaction(const QString& id);

  QString addAnalysis(const QString& name, const QString& type,
                      const QString& configJson, const QString& filterSpec,
                      const QString& exportFormat, bool includeCalcAdjustments,
                      const QString& exportStateJson,
                      const QString& snapshotTransactionsJson,
                      const QString& adjustmentsJson = QStringLiteral("{}"));
  QVariantMap validateAnalysis(
      const QString& id, const QString& name, const QString& type,
      const QString& configJson, const QString& filterSpec,
      const QString& exportFormat, bool includeCalcAdjustments,
      const QString& exportStateJson, const QString& snapshotTransactionsJson,
      const QString& adjustmentsJson = QStringLiteral("{}")) const;
  void updateAnalysis(const QString& id, const QString& name,
                      const QString& type, const QString& configJson,
                      const QString& filterSpec, const QString& exportFormat,
                      bool includeCalcAdjustments,
                      const QString& exportStateJson,
                      const QString& snapshotTransactionsJson,
                      const QString& adjustmentsJson = QStringLiteral("{}"));
  void deleteAnalysis(const QString& id);

  QString addAnnual(const QString& name, int year,
                    const QStringList& analysisIds = {});
  QVariantMap validateAnnual(const QString& id, const QString& name, int year,
                             const QStringList& analysisIds = {}) const;
  void updateAnnual(const QString& id, const QString& name, int year,
                    const QStringList& analysisIds = {});
  void deleteAnnual(const QString& id);
  void updateAnalysisExportFormat(const QString& analysisId,
                                  const QString& exportFormat);

  /** @brief Applies deletion side effects from the domain layer to UI state. */
  void
  applyDeletionImpact(const core::ports::workspace::DeletionImpact& impact);

  int dataRevision() const noexcept {
    return dataRevision_;
  }

signals:
  void dataRevisionChanged();
  void selectedActorIdChanged();
  void selectedPropertyIdChanged();
  void selectedContractIdChanged();
  void selectedStatementIdChanged();
  void selectedTransactionIdChanged();
  void selectedAnalysisIdChanged();
  void selectedAnnualIdChanged();
  void operationSucceeded(const QString& operation);
  void operationFailed(const QString& operation, const QString& error);

private:
  void bumpDataRevision();
  void runStorageOperation(const QString& operation,
                           const std::function<void()>& action);
  QVariantMap validationResultToMap(
      const core::ports::workspace::ValidationResult& result) const;
  bool rejectInvalidCommand(
      const char* origin,
      const core::ports::workspace::ValidationResult& result) const;

  std::unique_ptr<WorkspaceCache> cache_;
  std::unique_ptr<WorkspaceSelection> selection_;
  core::ports::workspace::IWorkspaceWriter* workspaceWriter_ = nullptr;
  core::ports::workspace::IWorkspaceReader* workspaceReader_ = nullptr;
  ImportWorkflow* importWorkflowForSave_ = nullptr;
  int dataRevision_ = 0;
};

} // namespace ui
