/**
 * @file ui/include/ui/workspace/WorkspaceCommands.h
 * @brief Declares UI-side workspace command submission.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include <functional>
#include <optional>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

/**
 * @brief Maps UI intent to core workspace writer commands.
 */
class WorkspaceCommands : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Creates workspace commands for a store.
   * @param store Workspace store owning ports and snapshot refresh state.
   * @param parent Optional Qt parent.
   */
  explicit WorkspaceCommands(WorkspaceStore &store, QObject *parent = nullptr);

  /**
   * @brief Reads a statement draft snapshot from workspace state.
   * @param draftId Draft id; empty allows the workspace reader default.
   * @return Draft snapshot when available.
   */
  std::optional<core::ports::workspace::StatementDraftSnapshot>
  statementDraftSnapshot(const QString &draftId = {}) const;
  /**
   * @brief Finalizes a statement draft through the workspace writer.
   * @param draft Draft snapshot to finalize.
   * @return Created statement id or an empty string.
   */
  QString finalizeStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft);
  /**
   * @brief Saves a statement draft in workspace state.
   * @param draft Draft snapshot to save.
   */
  void saveStatementDraft(
      const core::ports::workspace::StatementDraftSnapshot &draft);
  /**
   * @brief Removes a statement draft from workspace state.
   * @param draftId Draft id; empty allows the writer default.
   */
  void clearStatementDraft(const QString &draftId = {});

  /**
   * @brief Creates or updates an import log row.
   * @param logId Existing log id or empty for a generated id.
   * @param status User-visible import status.
   * @param message User-visible import message.
   * @param draftAttached Whether the log points to a review draft.
   * @param draftId Attached draft id.
   * @param statementId Finalized statement id.
   * @param importFile Source import file.
   */
  void upsertImportLog(const QString &logId, const QString &status,
                       const QString &message, bool draftAttached,
                       const QString &draftId = {},
                       const QString &statementId = {},
                       const QString &importFile = {});
  /**
   * @brief Deletes an import log.
   * @param id Import log id.
   */
  void deleteImportLog(const QString &id);

  /**
   * @brief Saves an export log in workspace state.
   * @param log Export log snapshot.
   */
  void saveExportLog(const core::ports::workspace::ExportLogSnapshot &log);
  /**
   * @brief Deletes an export log.
   * @param id Export log id.
   */
  void deleteExportLog(const QString &id);

  /**
   * @brief Creates a new workspace file.
   * @param path Target file path.
   */
  void newFile(const QString &path);
  /**
   * @brief Opens an existing workspace file.
   * @param path Source file path.
   */
  void openFile(const QString &path);
  /**
   * @brief Saves the current workspace file.
   */
  void saveFile();
  /**
   * @brief Saves the current workspace file to a new path.
   * @param path Target file path.
   */
  void saveFileAs(const QString &path);
  /**
   * @brief Registers an optional hook executed immediately before storage save.
   * @param callback Callback used by composition to flush active workflow
   * state.
   */
  void setBeforeStorageSaveCallback(std::function<void()> callback);

  /**
   * @brief Creates or updates an actor.
   * @param id Existing actor id or empty to create.
   * @param name Actor name.
   * @param aliases Actor aliases.
   * @param contractIds Linked contract ids.
   * @return Saved actor id or an empty string.
   */
  QString saveActor(const QString &id, const QString &name,
                    const QStringList &aliases = {},
                    const QStringList &contractIds = {});
  /**
   * @brief Deletes an actor.
   * @param id Actor id.
   */
  void deleteActor(const QString &id);

  /**
   * @brief Creates or updates a property.
   * @param id Existing property id or empty to create.
   * @param name Property name.
   * @param aliases Property aliases.
   * @param contractIds Linked contract ids.
   * @return Saved property id or an empty string.
   */
  QString saveProperty(const QString &id, const QString &name,
                       const QStringList &aliases = {},
                       const QStringList &contractIds = {});
  /**
   * @brief Deletes a property.
   * @param id Property id.
   */
  void deleteProperty(const QString &id);

  /**
   * @brief Creates or updates a contract.
   * @param id Existing contract id or empty to create.
   * @param name Contract name.
   * @param type Contract type.
   * @param actorIds Linked actor ids.
   * @param propertyIds Linked property ids.
   * @param aliases Contract aliases.
   * @param allocatableMode Contract allocatable mode.
   * @return Saved contract id or an empty string.
   */
  QString saveContract(const QString &id, const QString &name,
                       const QString &type,
                       const QStringList &actorIds = {},
                       const QStringList &propertyIds = {},
                       const QStringList &aliases = {},
                       const QString &allocatableMode =
                           QStringLiteral("mixed"));
  /**
   * @brief Deletes a contract.
   * @param id Contract id.
   */
  void deleteContract(const QString &id);

  /**
   * @brief Creates a statement.
   * @param name Statement name.
   * @return Created statement id or an empty string.
   */
  QString addStatement(const QString &name);
  /**
   * @brief Creates a statement and its transactions in one workspace mutation.
   * @param name Statement name.
   * @param transactionStates Transaction form state rows.
   * @return Created statement id or an empty string.
   */
  QString addStatementWithTransactions(const QString &name,
                                       const QVariantList &transactionStates);
  /**
   * @brief Updates a statement.
   * @param id Statement id.
   * @param name Statement name.
   */
  void updateStatement(const QString &id, const QString &name);
  /**
   * @brief Deletes a statement.
   * @param id Statement id.
   */
  void deleteStatement(const QString &id);

  /**
   * @brief Creates a transaction.
   * @param name Transaction name.
   * @param bookingDate Booking date text.
   * @param valuta Valuta date text.
   * @param amount Raw UI amount value.
   * @param statementId Owning statement id.
   * @param status Transaction status.
   * @param actorId Linked actor id.
   * @param contractId Linked contract id.
   * @param allocatable Allocatable flag.
   * @param propertyIds Linked property ids.
   * @return Created transaction id or an empty string.
   */
  QString addTransaction(const QString &name, const QString &bookingDate,
                         const QString &valuta, const QVariant &amount,
                         const QString &statementId, int status = 0,
                         const QString &actorId = QString(),
                         const QString &contractId = QString(),
                         bool allocatable = false,
                         const QStringList &propertyIds = {});
  /**
   * @brief Creates a transaction after another transaction.
   * @param afterTransactionId Transaction id used as insertion anchor.
   * @param name Transaction name.
   * @param bookingDate Booking date text.
   * @param valuta Valuta date text.
   * @param amount Raw UI amount value.
   * @param statementId Owning statement id.
   * @param status Transaction status.
   * @param actorId Linked actor id.
   * @param contractId Linked contract id.
   * @param allocatable Allocatable flag.
   * @param propertyIds Linked property ids.
   * @return Created transaction id or an empty string.
   */
  QString insertTransactionAfter(
      const QString &afterTransactionId, const QString &name,
      const QString &bookingDate, const QString &valuta,
      const QVariant &amount,
      const QString &statementId, int status = 0,
      const QString &actorId = QString(), const QString &contractId = QString(),
      bool allocatable = false, const QStringList &propertyIds = {});
  /**
   * @brief Updates a transaction.
   * @param id Transaction id.
   * @param name Transaction name.
   * @param bookingDate Booking date text.
   * @param valuta Valuta date text.
   * @param amount Raw UI amount value.
   * @param statementId Owning statement id.
   * @param status Transaction status.
   * @param actorId Linked actor id.
   * @param contractId Linked contract id.
   * @param allocatable Allocatable flag.
   * @param propertyIds Linked property ids.
   */
  void updateTransaction(const QString &id, const QString &name,
                         const QString &bookingDate, const QString &valuta,
                         const QVariant &amount, const QString &statementId,
                         int status,
                         const QString &actorId, const QString &contractId,
                         bool allocatable, const QStringList &propertyIds);
  /**
   * @brief Deletes a transaction.
   * @param id Transaction id.
   */
  void deleteTransaction(const QString &id);

  /**
   * @brief Creates an analysis.
   * @param name Analysis name.
   * @param type Analysis type.
   * @param config Analysis configuration payload.
   * @param filter Analysis filter payload.
   * @param exportFormat Export format.
   * @param includeCalcAdjustments Whether calculation adjustments are included.
   * @param snapshotTransactions Frozen transaction snapshot rows.
   * @param adjustments Adjustment amount map.
   * @return Created analysis id or an empty string.
   */
  QString addAnalysis(const QString &name, const QString &type,
                      const QVariantMap &config, const QVariantMap &filter,
                      const QString &exportFormat, bool includeCalcAdjustments,
                      const QVariantList &snapshotTransactions,
                      const QVariantMap &adjustments = {});
  /**
   * @brief Updates an analysis.
   * @param id Analysis id.
   * @param name Analysis name.
   * @param type Analysis type.
   * @param config Analysis configuration payload.
   * @param filter Analysis filter payload.
   * @param exportFormat Export format.
   * @param includeCalcAdjustments Whether calculation adjustments are included.
   * @param snapshotTransactions Frozen transaction snapshot rows.
   * @param adjustments Adjustment amount map.
   */
  void updateAnalysis(const QString &id, const QString &name,
                      const QString &type, const QVariantMap &config,
                      const QVariantMap &filter, const QString &exportFormat,
                      bool includeCalcAdjustments,
                      const QVariantList &snapshotTransactions,
                      const QVariantMap &adjustments = {});
  /**
   * @brief Deletes an analysis.
   * @param id Analysis id.
   */
  void deleteAnalysis(const QString &id);
  /**
   * @brief Updates only the stored export format of an analysis.
   * @param analysisId Analysis id.
   * @param exportFormat Export format.
   */
  void updateAnalysisExportFormat(const QString &analysisId,
                                  const QString &exportFormat);

  /**
   * @brief Creates an annual.
   * @param name Annual name.
   * @param year Annual year.
   * @param analysisIds Assigned analysis ids.
   * @return Created annual id or an empty string.
   */
  QString addAnnual(const QString &name, int year,
                    const QStringList &analysisIds = {});
  /**
   * @brief Updates an annual.
   * @param id Annual id.
   * @param name Annual name.
   * @param year Annual year.
   * @param analysisIds Assigned analysis ids.
   */
  void updateAnnual(const QString &id, const QString &name, int year,
                    const QStringList &analysisIds = {});
  /**
   * @brief Deletes an annual.
   * @param id Annual id.
   */
  void deleteAnnual(const QString &id);

signals:
  /**
   * @brief Emitted when a storage command completed.
   * @param operation Storage operation name.
   */
  void operationSucceeded(const QString &operation);
  /**
   * @brief Emitted when a storage command failed.
   * @param operation Storage operation name.
   * @param error Error message.
   */
  void operationFailed(const QString &operation, const QString &error);

private:
  /**
   * @brief Validates an actor command through the workspace writer.
   * @param id Actor id.
   * @param name Actor name.
   * @param aliases Actor aliases.
   * @param contractIds Linked contract ids.
   * @return QML validation payload.
   */
  QVariantMap validateActor(const QString &id, const QString &name,
                            const QStringList &aliases = {},
                            const QStringList &contractIds = {}) const;
  /**
   * @brief Validates a property command through the workspace writer.
   * @param id Property id.
   * @param name Property name.
   * @param aliases Property aliases.
   * @param contractIds Linked contract ids.
   * @return QML validation payload.
   */
  QVariantMap validateProperty(const QString &id, const QString &name,
                               const QStringList &aliases = {},
                               const QStringList &contractIds = {}) const;
  /**
   * @brief Validates a contract command through the workspace writer.
   * @param id Contract id.
   * @param name Contract name.
   * @param type Contract type.
   * @param actorIds Linked actor ids.
   * @param propertyIds Linked property ids.
   * @param aliases Contract aliases.
   * @param allocatableMode Allocatable mode.
   * @return QML validation payload.
   */
  QVariantMap validateContract(
      const QString &id, const QString &name, const QString &type,
      const QStringList &actorIds = {}, const QStringList &propertyIds = {},
      const QStringList &aliases = {},
      const QString &allocatableMode = QStringLiteral("mixed")) const;
  /**
   * @brief Validates a statement command through the workspace writer.
   * @param id Statement id.
   * @param name Statement name.
   * @return QML validation payload.
   */
  QVariantMap validateStatement(const QString &id, const QString &name) const;
  /**
   * @brief Validates a transaction command through the workspace writer.
   * @param id Transaction id.
   * @param name Transaction name.
   * @param bookingDate Booking date text.
   * @param valuta Valuta date text.
   * @param amount Amount value or text.
   * @param statementId Owning statement id.
   * @param status Transaction status index.
   * @param actorId Actor id.
   * @param contractId Contract id.
   * @param allocatable Allocatable flag.
   * @param propertyIds Linked property ids.
   * @return QML validation payload.
   */
  QVariantMap validateTransaction(
      const QString &id, const QString &name, const QString &bookingDate,
      const QString &valuta, const QVariant &amount, const QString &statementId,
      int status, const QString &actorId, const QString &contractId,
      bool allocatable, const QStringList &propertyIds) const;
  /**
   * @brief Validates an annual command through the workspace writer.
   * @param id Annual id.
   * @param name Annual name.
   * @param year Annual year.
   * @param analysisIds Assigned analysis ids.
   * @return QML validation payload.
   */
  QVariantMap validateAnnual(const QString &id, const QString &name, int year,
                             const QStringList &analysisIds = {}) const;
  /**
   * @brief Refreshes the UI store from the bound workspace reader.
   */
  void refreshStoreFromReader();
  /**
   * @brief Runs a storage command with error reporting and store refresh.
   * @param operation Storage operation name.
   * @param action Storage action to execute.
   */
  void runStorageOperation(const QString &operation,
                           const std::function<void()> &action);
  /**
   * @brief Saves an import log through the workspace writer.
   * @param log Import log snapshot.
   */
  void saveImportLog(const core::ports::workspace::ImportLogSnapshot &log);
  /**
   * @brief Reports and rejects an invalid workspace command.
   * @param origin Trace origin.
   * @param result Core validation result.
   * @return True when the command must be rejected.
   */
  bool rejectInvalidCommand(
      const char *origin,
      const core::ports::workspace::ValidationResult &result) const;

  WorkspaceStore &store_;
  std::function<void()> beforeStorageSave_;
};

} // namespace ui
