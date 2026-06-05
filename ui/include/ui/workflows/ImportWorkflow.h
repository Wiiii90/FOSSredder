/**
 * @file ui/include/ui/workflows/ImportWorkflow.h
 * @brief Coordinates asynchronous statement imports and in-memory draft
 * sessions.
 */

#pragma once

#include <functional>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

#include "core/errors/IErrorReporter.h"
#include "core/ports/usecases/import/IImportRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui {

/**
 * @brief Qt-friendly projection of the currently edited import transaction.
 */
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

class WorkspaceCommands;
class WorkspaceSelectors;

namespace adapters {
class ImportAdapter;
}

namespace importing {
class ImportWorkflowState;

} // namespace importing

/**
 * @brief Runs PDF imports and holds the active in-memory statement draft
 * session.
 */
class ImportWorkflow : public QObject {
  Q_OBJECT

public:
  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;

  /**
   * @brief Creates an import workflow.
   * @param importAdapter Adapter used to invoke the core import runner.
   * @param errorReporter Error reporter used for workflow exceptions.
   * @param stateSnapshotProvider Provider for the current workspace snapshot.
   * @param commands Workspace commands used for draft/log mutations.
   * @param selectors Workspace selectors used for log/draft lookups.
   * @param parent Optional Qt parent.
   */
  explicit ImportWorkflow(
      std::shared_ptr<ui::adapters::ImportAdapter> importAdapter,
      std::shared_ptr<core::errors::IErrorReporter> errorReporter,
      StateSnapshotProvider stateSnapshotProvider = {},
      WorkspaceCommands *commands = nullptr,
      WorkspaceSelectors *selectors = nullptr, QObject *parent = nullptr);
  /**
   * @brief Destroys the workflow and its internal state.
   */
  ~ImportWorkflow() override;

  /**
   * @brief Rebinds workspace roles used by the workflow.
   * @param commands Workspace commands.
   * @param selectors Workspace selectors.
   */
  void setWorkspaceRoles(WorkspaceCommands *commands,
                         WorkspaceSelectors *selectors);

  /** @brief Reports whether an import is running. @return True while running. */
  bool isRunning() const noexcept;
  /** @brief Reports whether the running import is paused. @return True while paused. */
  bool isPaused() const noexcept;
  /** @brief Returns import progress. @return Progress in the range 0..1. */
  double progress() const noexcept;
  /** @brief Returns the current import phase text. @return Phase text. */
  QString phase() const;
  /** @brief Returns the current import error text. @return Error text or empty. */
  QString error() const;
  /** @brief Returns the selected import file. @return Selected file path. */
  QString selectedFile() const;
  /**
   * @brief Sets the selected import file.
   * @param path Import file path.
   */
  void setSelectedFile(const QString& path);
  /** @brief Returns queued import file count. @return Queue size. */
  int queuedCount() const noexcept;
  /** @brief Returns queued import files. @return Queued file paths. */
  QStringList queuedFiles() const;
  /** @brief Reports whether an active statement draft exists. @return True when a draft is loaded. */
  bool hasDraft() const noexcept;
  /** @brief Returns the active draft id. @return Draft id or empty. */
  QString currentDraftId() const;
  /** @brief Returns the selected transaction draft index. @return Zero-based transaction index. */
  int currentTransactionIndex() const noexcept;
  /**
   * @brief Selects the current transaction draft by index.
   * @param index Zero-based transaction index.
   */
  void setCurrentTransactionIndex(int index);
  /** @brief Returns transaction count in the active draft. @return Transaction count. */
  int transactionCount() const noexcept;

  /**
   * @brief Returns the editable name of the active statement draft.
   * @return Statement draft name or empty.
   */
  QString currentStatementName() const;

  /**
   * @brief Saves the active in-memory draft into workspace draft state.
   */
  void flushActiveDraftToWorkspace();
  /**
   * @brief Renames the active statement draft.
   * @param name New statement draft name.
   * @return True when the draft changed.
   */
  bool renameCurrentStatementDraft(const QString& name);
  /**
   * @brief Inserts a transaction draft after the current one.
   * @return New selected transaction index or -1.
   */
  int insertTransactionAfterCurrent();
  /**
   * @brief Removes the current transaction draft.
   * @return New selected transaction index or -1.
   */
  int removeCurrentTransaction();
  /**
   * @brief Updates the current transaction draft name.
   * @param name Transaction name.
   * @return True when the draft changed.
   */
  bool commitCurrentTransactionName(const QString& name);
  /**
   * @brief Updates the current transaction draft booking date.
   * @param bookingDate Booking date text.
   * @return True when the draft changed.
   */
  bool commitCurrentTransactionBookingDate(const QString& bookingDate);
  /**
   * @brief Updates the current transaction draft valuta date.
   * @param valuta Valuta date text.
   * @return True when the draft changed.
   */
  bool commitCurrentTransactionValuta(const QString& valuta);
  /**
   * @brief Updates the current transaction draft status.
   * @param status Transaction status.
   * @return True when the draft changed.
   */
  bool setCurrentTransactionStatus(int status);
  /**
   * @brief Updates the current transaction draft allocatable flag.
   * @param allocatable Allocatable flag.
   * @return True when the draft changed.
   */
  bool setCurrentTransactionAllocatable(bool allocatable);
  /**
   * @brief Applies user-entered amount text to the current transaction draft.
   * @param amountText Amount text.
   * @return True when the draft changed.
   */
  bool applyCurrentTransactionAmountText(const QString& amountText);
  /**
   * @brief Selects an actor for the current transaction draft.
   * @param actorId Actor id.
   * @return True when the draft changed.
   */
  bool selectCurrentTransactionActor(const QString& actorId);
  /**
   * @brief Clears the actor selection for the current transaction draft.
   * @return True when the draft changed.
   */
  bool clearCurrentTransactionActor();
  /**
   * @brief Selects or deselects a property for the current transaction draft.
   * @param propertyId Property id.
   * @param selected Whether the property should be selected.
   * @return True when the draft changed.
   */
  bool setCurrentTransactionPropertySelected(const QString& propertyId,
                                             bool selected);
  /**
   * @brief Selects a contract for the current transaction draft.
   * @param contractId Contract id.
   * @return True when the draft changed.
   */
  bool selectCurrentTransactionContract(const QString& contractId);
  /**
   * @brief Clears the contract selection for the current transaction draft.
   * @return True when the draft changed.
   */
  bool clearCurrentTransactionContract();
  /**
   * @brief Creates or reuses an actor and selects it for the current draft.
   * @param actorName Actor name.
   * @return True when an actor was selected.
   */
  bool createActorForCurrentTransaction(const QString& actorName);
  /**
   * @brief Creates or reuses a property and selects it for the current draft.
   * @param propertyName Property name.
   * @return True when a property was selected.
   */
  bool createPropertyForCurrentTransaction(const QString& propertyName);
  /**
   * @brief Creates or reuses a contract and selects it for the current draft.
   * @param contractName Contract name.
   * @param contractType Contract type.
   * @param allocatableMode Allocatable mode.
   * @return True when a contract was selected.
   */
  bool
  createOrSelectContractForCurrentTransaction(const QString& contractName,
                                              const QString& contractType,
                                              const QString& allocatableMode);
  /**
   * @brief Builds the QML-facing view of the current transaction draft.
   * @return Transaction draft view payload.
   */
  TransactionDraftView currentTransactionView() const;
  /**
   * @brief Saves the active draft and returns to the import overview.
   */
  void pauseActiveDraft();
  /**
   * @brief Discards the active draft and removes its workspace draft state.
   */
  void discardActiveDraft();
  /**
   * @brief Finalizes the active draft into workspace statement data.
   */
  void finalizeActiveDraft();
  /**
   * @brief Removes an import log and optionally its attached draft.
   * @param logId Import log id.
   * @param draftAttached Whether the log points to a draft.
   * @param draftId Attached draft id.
   */
  void removeAttachedImportLog(const QString& logId, bool draftAttached,
                               const QString& draftId);

  /**
   * @brief Starts importing the selected or next queued statement file.
   */
  void startStatementImport();
  /**
   * @brief Adds files to the import selection or queue.
   * @param paths File paths.
   */
  void addFiles(const QStringList& paths);
  /**
   * @brief Resets import status fields when no import is running.
   */
  void resetStatus();
  /**
   * @brief Clears the active draft and may continue queued imports.
   */
  void clearDraft();
  /**
   * @brief Opens a statement draft stored in workspace state.
   * @param draftId Draft id; empty selects the remembered or first attached draft.
   * @return True when a draft was restored.
   */
  bool openStoredDraft(const QString& draftId = {});
  /**
   * @brief Cancels the current running import.
   */
  void cancelImport();
  /**
   * @brief Cancels the running import and clears queued imports.
   */
  void cancelQueuedImports();
  /**
   * @brief Pauses the current running import.
   */
  void pauseImport();
  /**
   * @brief Resumes the current paused import.
   */
  void resumeImport();

signals:
  /**
   * @brief Emitted when import workflow state changed.
   */
  void stateChanged();
  /**
   * @brief Emitted when an import reached a finished state.
   */
  void importFinished();
  /**
   * @brief Emitted when an import was canceled.
   */
  void importCanceled();
  /**
   * @brief Emitted when an import failed.
   * @param error Error message.
   */
  void importFailed(const QString& error);

private slots:
  /**
   * @brief Updates progress state from a queued import event.
   * @param progress Progress in the range 0..1.
   * @param phase Phase text.
   */
  void updateProgress(double progress, const QString& phase);
  /**
   * @brief Handles terminal import state from the runner.
   * @param state Terminal import state.
   * @param message Runner message.
   */
  void onJobTerminal(core::ports::importing::StatementImportState state,
                     const QString& message);

private:
  /**
   * @brief Reports whether an import handle is active.
   * @return True when a handle has an import id.
   */
  bool hasActiveImportHandle() const noexcept;
  /**
   * @brief Removes the active import event subscription.
   */
  void clearActiveImportSubscription();
  /**
   * @brief Rejects starting an import and updates workflow state.
   * @param errorMessage User-facing error message.
   * @param traceMessage Trace message.
   */
  void rejectImportStart(const QString& errorMessage, const char* traceMessage);
  /**
   * @brief Requests cancellation for the active import.
   * @param clearQueue Whether queued imports should be cleared.
   * @param origin Trace origin.
   * @param traceMessage Trace message.
   */
  void requestImportCancellation(bool clearQueue, const char* origin,
                                 const char* traceMessage);
  /**
   * @brief Sets paused state for the active import.
   * @param paused True to pause, false to resume.
   */
  void setImportPaused(bool paused);
  /**
   * @brief Handles one runner import event.
   * @param event Import event.
   */
  void
  handleImportEvent(const core::ports::importing::StatementImportEvent& event);
  /**
   * @brief Handles a canceled import terminal state.
   */
  void handleImportCanceled();
  /**
   * @brief Handles a failed import terminal state.
   * @param errorMessage Error message.
   * @param traceMessage Trace message.
   */
  void handleImportFailed(const QString& errorMessage,
                          const char* traceMessage);
  /**
   * @brief Builds and stores the active draft from the completed import result.
   * @return True when a draft was populated.
   */
  bool populateDraftFromResult();
  /**
   * @brief Starts the next queued import when available.
   */
  void startNextQueuedImport();
  /**
   * @brief Starts an import for one file.
   * @param path Import file path.
   */
  void startImportForFile(const QString& path);
  /**
   * @brief Reports an exception through the import diagnostics path.
   * @param origin Trace origin.
   * @param exception Exception pointer.
   */
  void reportException(const char* origin, std::exception_ptr exception) const;
  /**
   * @brief Saves the current import log state through workspace commands.
   * @param logId Import log id.
   * @param status Log status.
   * @param message Log message.
   * @param draftAttached Whether a draft is attached.
   * @param draftId Draft id.
   * @param statementId Statement id.
   * @param importFile Source import file path.
   */
  void saveImportLog(const QString& logId, const QString& status,
                     const QString& message, bool draftAttached = false,
                     const QString& draftId = {},
                     const QString& statementId = {},
                     const QString& importFile = {});
  /**
   * @brief Clears a statement draft stored in workspace state by id.
   * @param draftId Draft id.
   */
  void clearStoredDraft(const QString& draftId);
  /**
   * @brief Remembers the current transaction index for draft restoration.
   */
  void rememberCurrentDraftTransactionIndex();
  /**
   * @brief Applies a patch to the current transaction draft.
   * @param patch Draft patch command.
   * @return True when the draft changed.
   */
  bool applyCurrentTransactionPatch(
      const core::ports::importing::draft::TransactionDraftPatch& patch);
  /**
   * @brief Returns the current transaction draft.
   * @return Mutable transaction draft or nullptr.
   */
  core::ports::importing::draft::TransactionDraft* currentTransactionDraft();
  /**
   * @brief Returns the current transaction draft.
   * @return Transaction draft or nullptr.
   */
  const core::ports::importing::draft::TransactionDraft*
  currentTransactionDraft() const;
  /**
   * @brief Returns the current workspace snapshot.
   * @return Workspace snapshot or an empty snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;
  /**
   * @brief Returns the workspace snapshot used for draft catalog matching.
   * @return Merged catalog snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot catalogSnapshotForDraft() const;
  /**
   * @brief Emits state changes after draft mutation.
   */
  void notifyDraftChanged();
  /**
   * @brief Builds the current statement draft workspace snapshot.
   * @return Statement draft snapshot.
   */
  core::ports::workspace::StatementDraftSnapshot
  currentStatementDraftSnapshot() const;
  /**
   * @brief Finalizes the current statement draft through workspace commands.
   * @return Created statement id or empty.
   */
  QString finalizeCurrentStatementDraft();
  /**
   * @brief Returns the active statement draft.
   * @return Mutable statement draft or nullptr.
   */
  core::ports::importing::draft::StatementDraft* statementDraft() noexcept;
  /**
   * @brief Returns the active statement draft.
   * @return Statement draft or nullptr.
   */
  const core::ports::importing::draft::StatementDraft*
  statementDraft() const noexcept;
  /**
   * @brief Restores an attached statement draft from workspace state.
   * @param snapshot Workspace snapshot.
   * @return True when a draft was restored.
   */
  bool restoreDraftFromState(
      const core::ports::workspace::WorkspaceSnapshot& snapshot);
  /**
   * @brief Resolves the active draft context id.
   * @return Draft context id or empty.
   */
  QString resolveDraftContextId() const;
  /**
   * @brief Returns the remembered transaction index for a draft.
   * @param draftId Draft id.
   * @return Remembered transaction index.
   */
  int rememberedDraftTransactionIndex(const QString& draftId) const;

  std::unique_ptr<importing::ImportWorkflowState> state_;
  std::shared_ptr<ui::adapters::ImportAdapter> importAdapter_;
  StateSnapshotProvider stateSnapshotProvider_;
  WorkspaceCommands *commands_ = nullptr;
  WorkspaceSelectors *selectors_ = nullptr;
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
