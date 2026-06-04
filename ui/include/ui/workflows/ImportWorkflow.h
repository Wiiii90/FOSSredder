/**
 * @file ui/include/ui/workflows/ImportWorkflow.h
 * @brief Coordinates asynchronous statement imports and in-memory draft
 * sessions.
 */

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <map>
#include <memory>
#include <vector>

#include "core/errors/IErrorReporter.h"
#include "core/ports/usecases/import/IImportRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {
class ImportAdapter;
}

namespace ui {

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

class WorkspaceFacade;

namespace importing {

class ImportWorkflowState {
public:
  ImportWorkflowState() = default;

  bool isRunning() const noexcept {
    return isRunning_;
  }
  bool isPaused() const noexcept {
    return paused_;
  }
  double progress() const noexcept {
    return progress_;
  }
  const QString& phase() const noexcept {
    return phase_;
  }
  const QString& error() const noexcept {
    return error_;
  }
  const QString& selectedFile() const noexcept {
    return selectedFile_;
  }
  const QStringList& queuedFiles() const noexcept {
    return queuedFiles_;
  }
  bool hasDraft() const noexcept {
    return hasDraft_;
  }
  core::ports::importing::draft::StatementDraft* draft() noexcept {
    return hasDraft_ ? &draft_ : nullptr;
  }
  const core::ports::importing::draft::StatementDraft* draft() const noexcept {
    return hasDraft_ ? &draft_ : nullptr;
  }
  const core::ports::workspace::WorkspaceSnapshot&
  catalogSnapshot() const noexcept {
    return catalogSnapshot_;
  }
  int currentTransactionIndex() const noexcept {
    return currentTransactionIndex_;
  }
  void setCurrentTransactionIndex(int index);
  int artifactCount() const noexcept {
    return artifactCount_;
  }
  bool cancelRequested() const noexcept {
    return canceled_;
  }

  bool setSelectedFile(const QString& path);
  bool addFiles(const QStringList& paths);
  bool resetStatus();
  bool clearDraft();

  QString currentImportFile() const;
  QString takeSelectedFileForStart();
  bool takeNextQueuedFile(QString& nextFile);

  void beginImport(const QString& path);
  void rejectStart(const QString& errorMessage);
  void beginCancel(bool clearQueue);
  bool setPaused(bool paused);
  void recordCanceled();
  void recordFailed(const QString& errorMessage);
  void recordFinished();
  bool
  populateDraft(const core::ports::importing::draft::StatementDraft& draft,
                const core::ports::workspace::WorkspaceSnapshot& state,
                const std::map<std::string, std::vector<uint8_t>>& artifacts,
                int currentTransactionIndex);
  bool restoreDraft(const core::ports::importing::draft::StatementDraft& draft,
                    const core::ports::workspace::WorkspaceSnapshot& state,
                    const QString& draftId, int currentTransactionIndex);
  void updateProgress(double progress, const QString& phase);

private:
  void clearDraftState();
  void clearTransientImportState();
  void resetCancellationState();

  bool isRunning_ = false;
  double progress_ = 0.0;
  QString phase_;
  QString error_;
  QString selectedFile_;
  QStringList queuedFiles_;
  core::ports::importing::draft::StatementDraft draft_;
  bool hasDraft_ = false;
  int currentTransactionIndex_ = 0;
  core::ports::workspace::WorkspaceSnapshot catalogSnapshot_;
  int artifactCount_ = 0;
  bool canceled_ = false;
  bool paused_ = false;
  bool cancelClearsQueue_ = false;
  QString currentImportFile_;
};

} // namespace importing

/**
 * @brief Runs PDF imports and holds the active in-memory statement draft
 * session.
 */
class ImportWorkflow : public QObject {
  Q_OBJECT

public:
  explicit ImportWorkflow(
      std::shared_ptr<ui::adapters::ImportAdapter> importAdapter,
      std::shared_ptr<core::errors::IErrorReporter> errorReporter,
      WorkspaceFacade* workspace = nullptr, QObject* parent = nullptr);

  void setWorkspace(WorkspaceFacade* workspace);

  bool isRunning() const noexcept {
    return state_.isRunning();
  }
  bool isPaused() const noexcept {
    return state_.isPaused();
  }
  double progress() const noexcept {
    return state_.progress();
  }
  QString phase() const {
    return state_.phase();
  }
  QString error() const {
    return state_.error();
  }
  QString selectedFile() const;
  void setSelectedFile(const QString& path);
  int queuedCount() const noexcept {
    return static_cast<int>(state_.queuedFiles().size());
  }
  QStringList queuedFiles() const;
  bool hasDraft() const noexcept {
    return state_.hasDraft();
  }
  QString currentDraftId() const;
  int currentTransactionIndex() const noexcept {
    return state_.currentTransactionIndex();
  }
  void setCurrentTransactionIndex(int index);
  int transactionCount() const noexcept;

  /**
   * @brief Returns the editable name of the active statement draft.
   */
  QString currentStatementName() const;

  core::ports::importing::draft::TransactionDraft* currentTransactionDraft();
  const core::ports::importing::draft::TransactionDraft*
  currentTransactionDraft() const;
  core::ports::workspace::WorkspaceSnapshot catalogSnapshotForDraft() const;
  void notifyDraftChanged();
  void flushSessionToWorkspace();
  bool renameCurrentStatementDraft(const QString& name);
  int insertTransactionAfterCurrent();
  int removeCurrentTransaction();
  bool commitCurrentTransactionName(const QString& name);
  bool commitCurrentTransactionBookingDate(const QString& bookingDate);
  bool commitCurrentTransactionValuta(const QString& valuta);
  bool setCurrentTransactionStatus(int status);
  bool setCurrentTransactionAllocatable(bool allocatable);
  bool applyCurrentTransactionAmountText(const QString& amountText);
  bool selectCurrentTransactionActor(const QString& actorId);
  bool clearCurrentTransactionActor();
  bool setCurrentTransactionPropertySelected(const QString& propertyId,
                                             bool selected);
  bool selectCurrentTransactionContract(const QString& contractId);
  bool clearCurrentTransactionContract();
  bool createActorForCurrentTransaction(const QString& actorName);
  bool createPropertyForCurrentTransaction(const QString& propertyName);
  bool
  createOrSelectContractForCurrentTransaction(const QString& contractName,
                                              const QString& contractType,
                                              const QString& allocatableMode);
  TransactionDraftView currentTransactionView() const;
  /**
   * @brief Persists the active draft and finalizes it into a workspace
   * statement.
   * @return Created statement id, or empty when finalization failed.
   */
  QString finalizeCurrentStatementDraft();
  void pauseActiveDraft();
  void discardActiveDraft();
  void finalizeActiveDraft();
  void removeAttachedImportLog(const QString& logId, bool draftAttached,
                               const QString& draftId);

  void startStatementImport();
  void addFiles(const QStringList& paths);
  void resetStatus();
  void clearDraft();
  bool openPersistedDraft(const QString& draftId = {});
  void rememberCurrentDraftTransactionIndex();
  void cancelImport();
  void cancelQueuedImports();
  void pauseImport();
  void resumeImport();

signals:
  void stateChanged();
  void importFinished();
  void importCanceled();
  void importFailed(const QString& error);

private slots:
  void updateProgress(double progress, const QString& phase);
  void onJobTerminal(core::ports::importing::StatementImportState state,
                     const QString& message);

private:
  bool hasActiveImportHandle() const noexcept;
  void clearActiveImportSubscription();
  void rejectImportStart(const QString& errorMessage, const char* traceMessage);
  void requestImportCancellation(bool clearQueue, const char* origin,
                                 const char* traceMessage);
  void setImportPaused(bool paused);
  void
  handleImportEvent(const core::ports::importing::StatementImportEvent& event);
  void handleImportCanceled();
  void handleImportFailed(const QString& errorMessage,
                          const char* traceMessage);
  bool populateDraftFromResult();
  void startNextQueuedImport();
  void startImportForFile(const QString& path);
  void reportException(const char* origin, std::exception_ptr exception) const;
  void persistImportLog(const QString& logId, const QString& status,
                        const QString& message, bool draftAttached = false,
                        const QString& draftId = {},
                        const QString& statementId = {},
                        const QString& importFile = {});
  void clearPersistedDraft(const QString& draftId);
  bool applyCurrentTransactionPatch(
      const core::ports::importing::draft::TransactionDraftPatch& patch);
  core::ports::workspace::StatementDraftSnapshot
  currentStatementDraftSnapshot() const;
  core::ports::importing::draft::StatementDraft* statementDraft() noexcept;
  const core::ports::importing::draft::StatementDraft*
  statementDraft() const noexcept;
  bool restoreDraftFromState(
      const core::ports::workspace::WorkspaceSnapshot& snapshot);
  QString resolveDraftContextId() const;
  int rememberedDraftTransactionIndex(const QString& draftId) const;

  importing::ImportWorkflowState state_;
  std::shared_ptr<ui::adapters::ImportAdapter> importAdapter_;
  WorkspaceFacade* workspace_ = nullptr;
  std::shared_ptr<core::errors::IErrorReporter> errorReporter_;

  QString activeDraftId_;
  QHash<QString, int> draftTransactionIndexByDraftId_;
  QString activeImportLogId_;
  bool activeImportTerminalHandled_ = false;
  bool hasPendingTerminalEvent_ = false;
  bool hasActiveImportHandle_ = false;
  core::ports::importing::StatementImportHandle activeImportHandle_;
  core::ports::importing::StatementImportState pendingTerminalState_ =
      core::ports::importing::StatementImportState::Pending;
  QString pendingTerminalMessage_;
};

} // namespace ui
