/**
 * @file ui/include/ui/viewmodels/ImportViewModel.h
 * @brief Declares the QML API for import overview and draft review.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "ui/shell/AppActions.h"
#include "ui/shell/NavigationState.h"
#include "ui/shell/Settings.h"
#include "ui/shell/StatusState.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui {

class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;

/**
 * @brief Exposes import overview, statement draft, and transaction draft state to QML.
 */
class ImportViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ImportViewModel)

  Q_PROPERTY(int contentIndex READ contentIndex NOTIFY changed)
  Q_PROPERTY(bool hasImportWorkflow READ hasImportWorkflow NOTIFY changed)
  Q_PROPERTY(bool hasDraft READ hasDraft NOTIFY changed)
  Q_PROPERTY(bool hasDraftNavigation READ hasDraftNavigation NOTIFY changed)
  Q_PROPERTY(bool canClearImport READ canClearImport NOTIFY changed)
  Q_PROPERTY(bool canCancel READ canCancel NOTIFY changed)
  Q_PROPERTY(bool canPause READ canPause NOTIFY changed)
  Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
  Q_PROPERTY(bool importRunning READ importRunning NOTIFY changed)
  Q_PROPERTY(bool importPaused READ importPaused NOTIFY changed)
  Q_PROPERTY(QString pauseText READ pauseText NOTIFY changed)
  Q_PROPERTY(QString progressText READ progressText NOTIFY changed)
  Q_PROPERTY(bool progressHasError READ progressHasError NOTIFY changed)
  Q_PROPERTY(double progressValue READ progressValue NOTIFY changed)
  Q_PROPERTY(QString manualPathText READ manualPathText WRITE setManualPathText
                 NOTIFY changed)
  Q_PROPERTY(QStringList importFiles READ importFiles NOTIFY changed)
  Q_PROPERTY(QString importFileSummary READ importFileSummary NOTIFY changed)
  Q_PROPERTY(ui::NavigationState* navigation READ navigation NOTIFY changed)
  Q_PROPERTY(int queuedCount READ queuedCount NOTIFY changed)
  Q_PROPERTY(QVariantList importLogs READ importLogs NOTIFY changed)
  Q_PROPERTY(QString selectedDraftId READ selectedDraftId NOTIFY changed)
  Q_PROPERTY(QStringList importSourceLabels READ importSourceLabels CONSTANT)
  Q_PROPERTY(
      QStringList statementStrategyLabels READ statementStrategyLabels CONSTANT)

  Q_PROPERTY(QString statementName READ statementName WRITE setStatementName
                 NOTIFY changed)
  Q_PROPERTY(int currentTransactionNumber READ currentTransactionNumber NOTIFY
                 changed)
  Q_PROPERTY(int transactionCount READ transactionCount NOTIFY changed)
  Q_PROPERTY(bool canDeleteTransaction READ canDeleteTransaction NOTIFY changed)
  Q_PROPERTY(bool canSelectPreviousTransactionDraft READ
                 canSelectPreviousTransactionDraft NOTIFY changed)
  Q_PROPERTY(bool canSelectNextTransactionDraft READ
                 canSelectNextTransactionDraft NOTIFY changed)

  Q_PROPERTY(QString nameText READ nameText WRITE setNameText NOTIFY changed)
  Q_PROPERTY(QString bookingDateText READ bookingDateText WRITE
                 setBookingDateText NOTIFY changed)
  Q_PROPERTY(
      QString valutaText READ valutaText WRITE setValutaText NOTIFY changed)
  Q_PROPERTY(
      QString amountText READ amountText WRITE setAmountText NOTIFY changed)
  Q_PROPERTY(QVariantList statusOptions READ statusOptions NOTIFY changed)
  Q_PROPERTY(int statusIndex READ statusIndex NOTIFY changed)
  Q_PROPERTY(QString metadataText READ metadataText NOTIFY changed)
  Q_PROPERTY(QString proofSource READ proofSource NOTIFY changed)
  Q_PROPERTY(bool effectiveAllocatable READ effectiveAllocatable NOTIFY changed)
  Q_PROPERTY(QVariantList actorOptions READ actorOptions NOTIFY changed)
  Q_PROPERTY(int selectedActorOptionIndex READ selectedActorOptionIndex NOTIFY
                 changed)
  Q_PROPERTY(QString actorName READ actorName WRITE setActorName NOTIFY changed)
  Q_PROPERTY(bool canAddActor READ canAddActor NOTIFY changed)
  Q_PROPERTY(QVariantList contractOptions READ contractOptions NOTIFY changed)
  Q_PROPERTY(
      int selectedContractOptionIndex READ selectedContractOptionIndex NOTIFY changed)
  Q_PROPERTY(QString contractName READ contractName WRITE setContractName NOTIFY
                 changed)
  Q_PROPERTY(QString contractType READ contractType WRITE setContractType NOTIFY
                 changed)
  Q_PROPERTY(QString contractNamePlaceholder READ contractNamePlaceholder NOTIFY
                 changed)
  Q_PROPERTY(QVariantList contractAllocatableModes READ contractAllocatableModes
                 CONSTANT)
  Q_PROPERTY(int contractAllocatableModeIndex READ contractAllocatableModeIndex
                 WRITE setContractAllocatableModeIndex NOTIFY changed)
  Q_PROPERTY(
      QString selectedContractType READ selectedContractType NOTIFY changed)
  Q_PROPERTY(bool canAddContract READ canAddContract NOTIFY changed)
  Q_PROPERTY(QVariantList propertyOptions READ propertyOptions NOTIFY changed)
  Q_PROPERTY(QString propertyName READ propertyName WRITE setPropertyName NOTIFY
                 changed)
  Q_PROPERTY(bool canAddProperty READ canAddProperty NOTIFY changed)
  Q_PROPERTY(double actorSuggestionConfidence READ actorSuggestionConfidence
                 NOTIFY changed)
  Q_PROPERTY(
      QString actorSuggestionSummary READ actorSuggestionSummary NOTIFY changed)
  Q_PROPERTY(double propertySuggestionConfidence READ
                 propertySuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString propertySuggestionSummary READ propertySuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double contractSuggestionConfidence READ
                 contractSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString contractSuggestionSummary READ contractSuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double allocatableSuggestionConfidence READ
                 allocatableSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString allocatableSuggestionText READ allocatableSuggestionText
                 NOTIFY changed)

public:
  /**
   * @brief Creates the import view model.
   * @param parent Optional Qt parent object.
   */
  explicit ImportViewModel(QObject* parent = nullptr);

  /**
   * @brief Sets the import workflow used for import runs and draft review.
   * @param value Import workflow or nullptr.
   */
  void setImportWorkflow(ImportWorkflow* value);

  /**
   * @brief Sets settings used for import defaults.
   * @param value Settings object or nullptr.
   */
  void setSettings(Settings* value);

  /**
   * @brief Sets shell actions used for browsing files.
   * @param value Actions object or nullptr.
   */
  void setActions(Actions* value);

  /**
   * @brief Sets shell navigation used by import transitions.
   * @param value Navigation state or nullptr.
   */
  void setNavigation(NavigationState* value);

  /**
   * @brief Returns shell navigation used by import transitions.
   * @return Navigation state or nullptr.
   */
  NavigationState* navigation() const noexcept { return navigation_; }

  /**
   * @brief Sets shell status output used by import actions.
   * @param value Status state or nullptr.
   */
  void setStatus(StatusState* value);

  /**
   * @brief Sets workspace roles used for import logs, catalog and selection.
   * @param store Workspace store or nullptr.
   * @param commands Workspace command API or nullptr.
   * @param selection Workspace selection API or nullptr.
   * @param selectors Workspace selector API or nullptr.
   */
  void setWorkspaceRoles(WorkspaceStore *store, WorkspaceCommands *commands,
                         WorkspaceSelection *selection,
                         WorkspaceSelectors *selectors);

  /**
   * @brief Returns the active import content index.
   * @return Content stack index.
   */
  int contentIndex() const noexcept;

  /**
   * @brief Returns whether an import workflow is attached.
   * @return True when workflow commands can run.
   */
  bool hasImportWorkflow() const noexcept;

  /**
   * @brief Returns whether a statement draft is active.
   * @return True when draft review content is available.
   */
  bool hasDraft() const noexcept;

  /**
   * @brief Returns whether draft navigation is available.
   * @return True when previous/next draft actions can be shown.
   */
  bool hasDraftNavigation() const noexcept;

  /**
   * @brief Returns whether the import form can be cleared.
   * @return True when local import selection can be cleared.
   */
  bool canClearImport() const noexcept;

  /**
   * @brief Returns whether the current import can be canceled.
   * @return True when cancel is available.
   */
  bool canCancel() const noexcept;

  /**
   * @brief Returns whether the current import can be paused.
   * @return True when pause is available.
   */
  bool canPause() const noexcept;

  /**
   * @brief Returns whether import can be started.
   * @return True when selected files can be queued.
   */
  bool canStart() const noexcept;

  /**
   * @brief Returns whether an import is running.
   * @return True while import work is active.
   */
  bool importRunning() const noexcept;

  /**
   * @brief Returns whether the import workflow is paused.
   * @return True when paused.
   */
  bool importPaused() const noexcept;

  /**
   * @brief Returns the pause/resume button text.
   * @return Localized pause state text.
   */
  QString pauseText() const;

  /**
   * @brief Returns the import progress text.
   * @return Localized progress text.
   */
  QString progressText() const;

  /**
   * @brief Returns whether progress currently represents an error.
   * @return True when the progress state has an error.
   */
  bool progressHasError() const noexcept;

  /**
   * @brief Returns the import progress value.
   * @return Progress value between 0 and 1 when available.
   */
  double progressValue() const noexcept;

  /**
   * @brief Returns the manual import path field.
   * @return Manual path text.
   */
  QString manualPathText() const { return manualPathText_; }

  /**
   * @brief Updates the manual import path field.
   * @param value Manual path text.
   */
  void setManualPathText(const QString& value);

  /**
   * @brief Returns selected import files.
   * @return File path list.
   */
  QStringList importFiles() const;

  /**
   * @brief Returns a short summary of selected import files.
   * @return Localized file summary text.
   */
  QString importFileSummary() const;

  /**
   * @brief Returns queued import count.
   * @return Number of queued import files.
   */
  int queuedCount() const noexcept;

  /**
   * @brief Returns import logs for the sidebar.
   * @return Import log rows.
   */
  QVariantList importLogs() const;

  /**
   * @brief Returns the active draft id.
   * @return Draft id or an empty string.
   */
  QString selectedDraftId() const;

  /**
   * @brief Returns import source labels.
   * @return Source labels.
   */
  QStringList importSourceLabels() const;

  /**
   * @brief Returns statement strategy labels.
   * @return Strategy labels.
   */
  QStringList statementStrategyLabels() const;

  /**
   * @brief Returns the active statement draft name field.
   * @return Statement name.
   */
  QString statementName() const;

  /**
   * @brief Updates the active statement draft name field.
   * @param value Statement name.
   */
  void setStatementName(const QString& value);

  /**
   * @brief Returns the current transaction draft number.
   * @return One-based transaction number.
   */
  int currentTransactionNumber() const noexcept;

  /**
   * @brief Returns the transaction draft count.
   * @return Transaction count.
   */
  int transactionCount() const noexcept;

  /**
   * @brief Returns whether the current transaction draft can be deleted.
   * @return True when deletion is available.
   */
  bool canDeleteTransaction() const noexcept;

  /**
   * @brief Returns whether the previous transaction draft can be selected.
   * @return True when previous draft navigation is available.
   */
  bool canSelectPreviousTransactionDraft() const noexcept;

  /**
   * @brief Returns whether the next transaction draft can be selected.
   * @return True when next draft navigation is available.
   */
  bool canSelectNextTransactionDraft() const noexcept;

  /**
   * @brief Returns the transaction name field.
   * @return Transaction name text.
   */
  QString nameText() const { return nameText_; }

  /**
   * @brief Updates the transaction name field.
   * @param value Transaction name text.
   */
  void setNameText(const QString& value);

  /**
   * @brief Returns the booking date field.
   * @return Booking date text.
   */
  QString bookingDateText() const { return bookingDateText_; }

  /**
   * @brief Updates the booking date field.
   * @param value Booking date text.
   */
  void setBookingDateText(const QString& value);

  /**
   * @brief Returns the valuta field.
   * @return Valuta text.
   */
  QString valutaText() const { return valutaText_; }

  /**
   * @brief Updates the valuta field.
   * @param value Valuta text.
   */
  void setValutaText(const QString& value);

  /**
   * @brief Returns the amount field.
   * @return Amount text.
   */
  QString amountText() const { return amountText_; }

  /**
   * @brief Updates the amount field.
   * @param value Amount text.
   */
  void setAmountText(const QString& value);

  /**
   * @brief Returns transaction status options.
   * @return Status option rows.
   */
  QVariantList statusOptions() const;

  /**
   * @brief Returns selected transaction status index.
   * @return Index into statusOptions().
   */
  int statusIndex() const;

  /**
   * @brief Returns metadata text for the current transaction draft.
   * @return Metadata text.
   */
  QString metadataText() const;

  /**
   * @brief Returns proof image source for the current draft.
   * @return QML image source string.
   */
  QString proofSource() const;

  /**
   * @brief Returns the effective allocatable state.
   * @return True when the transaction draft is allocatable.
   */
  bool effectiveAllocatable() const;

  /**
   * @brief Returns actor dropdown options.
   * @return Actor option rows.
   */
  QVariantList actorOptions() const;

  /**
   * @brief Returns selected actor option index.
   * @return Index into actorOptions().
   */
  int selectedActorOptionIndex() const;

  /**
   * @brief Returns the quick-create actor name field.
   * @return Actor name text.
   */
  QString actorName() const { return actorName_; }

  /**
   * @brief Updates the quick-create actor name field.
   * @param value Actor name text.
   */
  void setActorName(const QString& value);

  /**
   * @brief Returns whether quick-create actor is available.
   * @return True when actor can be added.
   */
  bool canAddActor() const;

  /**
   * @brief Returns contract dropdown options.
   * @return Contract option rows.
   */
  QVariantList contractOptions() const;

  /**
   * @brief Returns selected contract option index.
   * @return Index into contractOptions().
   */
  int selectedContractOptionIndex() const;

  /**
   * @brief Returns the quick-create contract name field.
   * @return Contract name text.
   */
  QString contractName() const { return contractName_; }

  /**
   * @brief Updates the quick-create contract name field.
   * @param value Contract name text.
   */
  void setContractName(const QString& value);

  /**
   * @brief Returns the quick-create contract type field.
   * @return Contract type text.
   */
  QString contractType() const { return contractType_; }

  /**
   * @brief Updates the quick-create contract type field.
   * @param value Contract type text.
   */
  void setContractType(const QString& value);

  /**
   * @brief Returns the quick-create contract name placeholder.
   * @return Placeholder text.
   */
  QString contractNamePlaceholder() const;

  /**
   * @brief Returns contract allocatable mode options.
   * @return Allocatable mode rows.
   */
  QVariantList contractAllocatableModes() const;

  /**
   * @brief Returns selected contract allocatable mode index.
   * @return Index into contractAllocatableModes().
   */
  int contractAllocatableModeIndex() const;

  /**
   * @brief Updates selected contract allocatable mode index.
   * @param index Index into contractAllocatableModes().
   */
  void setContractAllocatableModeIndex(int index);

  /**
   * @brief Returns the selected contract type.
   * @return Contract type key.
   */
  QString selectedContractType() const;

  /**
   * @brief Returns whether quick-create contract is available.
   * @return True when contract can be added.
   */
  bool canAddContract() const;

  /**
   * @brief Returns property checkbox options.
   * @return Property option rows.
   */
  QVariantList propertyOptions() const;

  /**
   * @brief Returns the quick-create property name field.
   * @return Property name text.
   */
  QString propertyName() const { return propertyName_; }

  /**
   * @brief Updates the quick-create property name field.
   * @param value Property name text.
   */
  void setPropertyName(const QString& value);

  /**
   * @brief Returns whether quick-create property is available.
   * @return True when property can be added.
   */
  bool canAddProperty() const;

  /**
   * @brief Initializes import view state when the view is opened.
   */
  Q_INVOKABLE void initializeImportView();

  /**
   * @brief Opens a file browser for import PDFs.
   */
  Q_INVOKABLE void browseImportPdf();

  /**
   * @brief Adds the manual path field to the selected import files.
   */
  Q_INVOKABLE void addSelectedImportFiles();

  /**
   * @brief Clears the local import selection and progress display.
   */
  Q_INVOKABLE void clearImport();

  /**
   * @brief Cancels the current import job.
   */
  Q_INVOKABLE void cancelCurrentImport();

  /**
   * @brief Cancels queued import jobs.
   */
  Q_INVOKABLE void cancelQueuedImports();

  /**
   * @brief Pauses the current import job.
   */
  Q_INVOKABLE void pauseImport();

  /**
   * @brief Resumes a paused import job.
   */
  Q_INVOKABLE void resumeImport();

  /**
   * @brief Starts importing the selected files.
   */
  Q_INVOKABLE void startImport();

  /**
   * @brief Selects the previous statement draft.
   */
  Q_INVOKABLE void selectPreviousDraft();

  /**
   * @brief Selects the next statement draft.
   */
  Q_INVOKABLE void selectNextDraft();

  /**
   * @brief Opens an import log from the sidebar.
   * @param logId Import log id.
   * @param draftAttached True when a draft is attached.
   * @param statementId Statement id associated with finished logs.
   * @param draftId Draft id associated with draft logs.
   */
  Q_INVOKABLE void openImportLog(const QString& logId, bool draftAttached,
                                 const QString& statementId,
                                 const QString& draftId);

  /**
   * @brief Deletes an import log and attached draft state when requested.
   * @param logId Import log id.
   * @param draftAttached True when a draft is attached.
   * @param draftId Draft id associated with the log.
   */
  Q_INVOKABLE void deleteImportLog(const QString& logId, bool draftAttached,
                                   const QString& draftId);

  /**
   * @brief Returns from draft review to the import overview.
   */
  Q_INVOKABLE void returnToImport();

  /**
   * @brief Discards the active statement draft.
   */
  Q_INVOKABLE void discard();

  /**
   * @brief Adds a transaction draft after the current one.
   */
  Q_INVOKABLE void addTransactionAfterCurrent();

  /**
   * @brief Deletes the current transaction draft.
   */
  Q_INVOKABLE void deleteCurrentTransaction();

  /**
   * @brief Finalizes the active statement draft through the import workflow.
   */
  Q_INVOKABLE void finalize();

  /**
   * @brief Selects the previous transaction draft.
   */
  Q_INVOKABLE void selectPreviousTransactionDraft();

  /**
   * @brief Selects the next transaction draft.
   */
  Q_INVOKABLE void selectNextTransactionDraft();

  /**
   * @brief Commits the transaction name field to the workflow draft.
   */
  Q_INVOKABLE void commitNameText();

  /**
   * @brief Commits the booking date field to the workflow draft.
   */
  Q_INVOKABLE void commitBookingDateText();

  /**
   * @brief Commits the valuta field to the workflow draft.
   */
  Q_INVOKABLE void commitValutaText();

  /**
   * @brief Commits the amount field to the workflow draft.
   */
  Q_INVOKABLE void commitAmountText();

  /**
   * @brief Selects a transaction status option.
   * @param index Index into statusOptions().
   */
  Q_INVOKABLE void selectStatusAtIndex(int index);

  /**
   * @brief Maps suggestion confidence to a presentation tone.
   * @param confidence Suggestion confidence.
   * @return Tone index for QML styling.
   */
  Q_INVOKABLE int suggestionTone(double confidence) const;

  /**
   * @brief Selects an actor option.
   * @param index Index into actorOptions().
   */
  Q_INVOKABLE void selectActorAtIndex(int index);

  /**
   * @brief Creates and selects an actor for the current draft.
   */
  Q_INVOKABLE void addActor();

  /**
   * @brief Selects a contract option.
   * @param index Index into contractOptions().
   */
  Q_INVOKABLE void selectContractAtIndex(int index);

  /**
   * @brief Creates and selects a contract for the current draft.
   */
  Q_INVOKABLE void addContract();

  /**
   * @brief Returns whether a property is selected for the current draft.
   * @param propertyId Property id.
   * @return True when selected.
   */
  Q_INVOKABLE bool isPropertySelected(const QString& propertyId) const;

  /**
   * @brief Updates whether a property is selected for the current draft.
   * @param propertyId Property id.
   * @param selected True to select, false to remove.
   */
  Q_INVOKABLE void setPropertySelected(const QString& propertyId, bool selected);

  /**
   * @brief Creates and selects a property for the current draft.
   */
  Q_INVOKABLE void addProperty();

  /**
   * @brief Toggles the current draft allocatable state.
   */
  Q_INVOKABLE void toggleAllocatable();

  /**
   * @brief Returns actor suggestion confidence.
   * @return Confidence between 0 and 1.
   */
  double actorSuggestionConfidence() const;

  /**
   * @brief Returns actor suggestion summary text.
   * @return Summary text.
   */
  QString actorSuggestionSummary() const;

  /**
   * @brief Returns property suggestion confidence.
   * @return Confidence between 0 and 1.
   */
  double propertySuggestionConfidence() const;

  /**
   * @brief Returns property suggestion summary text.
   * @return Summary text.
   */
  QString propertySuggestionSummary() const;

  /**
   * @brief Returns contract suggestion confidence.
   * @return Confidence between 0 and 1.
   */
  double contractSuggestionConfidence() const;

  /**
   * @brief Returns contract suggestion summary text.
   * @return Summary text.
   */
  QString contractSuggestionSummary() const;

  /**
   * @brief Returns allocatable suggestion confidence.
   * @return Confidence between 0 and 1.
   */
  double allocatableSuggestionConfidence() const;

  /**
   * @brief Returns allocatable suggestion text.
   * @return Suggestion text.
   */
  QString allocatableSuggestionText() const;

signals:
  /**
   * @brief Emitted when import view model state changed.
   */
  void changed();

private:
  /**
   * @brief Connects import workflow signals.
   * @param value Import workflow or nullptr.
   */
  void bindImportWorkflow(ImportWorkflow* value);
  /**
   * @brief Connects settings signals.
   * @param value Settings object or nullptr.
   */
  void bindSettings(Settings* value);
  /**
   * @brief Connects shell action signals.
   * @param value Actions object or nullptr.
   */
  void bindActions(Actions* value);
  /**
   * @brief Connects workspace role signals.
   */
  void bindWorkspaceRoles(WorkspaceStore *store, WorkspaceCommands *commands,
                          WorkspaceSelection *selection,
                          WorkspaceSelectors *selectors);
  /**
   * @brief Applies the default import path when the import view is opened.
   */
  void applyDefaultImportSelection();
  /**
   * @brief Copies the workflow-selected file into the manual path field.
   */
  void updateManualPathFromWorkflow();
  /**
   * @brief Replaces locally selected import files.
   * @param paths Import file paths.
   */
  void setSelectedImportFiles(const QStringList& paths);
  /**
   * @brief Updates the shell status text.
   * @param text Status text.
   */
  void setStatusText(const QString& text);
  /**
   * @brief Queues supported files for the import workflow.
   * @param paths File paths to queue.
   */
  void queueImportFiles(const QStringList& paths);
  /**
   * @brief Filters paths to supported import files.
   * @param paths Candidate paths.
   * @return Supported import file paths.
   */
  QStringList supportedImportFiles(const QStringList& paths) const;
  /**
   * @brief Returns the active statement draft stack index.
   * @return Active draft index.
   */
  int activeDraftStackIndex() const;
  /**
   * @brief Opens a statement draft by stack index.
   * @param index Draft stack index.
   * @return True when a draft was opened.
   */
  bool openDraftAtStackIndex(int index);

  /**
   * @brief Synchronizes transaction draft fields from workflow state.
   */
  void syncTransactionDraftFromWorkflow();
  /**
   * @brief Reloads the current transaction draft presentation payload.
   */
  void reloadCurrentTransactionView();
  /**
   * @brief Clears local transaction draft form fields.
   */
  void resetTransactionDraftFields();
  /**
   * @brief Formats a suggestion summary for display.
   * @param value Suggestion summary text.
   * @return Display-ready suggestion summary.
   */
  QString displaySuggestionSummary(const QString& value) const;
  /**
   * @brief Returns the current transaction draft id.
   * @return Transaction draft id.
   */
  QString currentTransactionId() const;
  /**
   * @brief Returns property ids selected on the current transaction draft.
   * @return Property ids.
   */
  QStringList currentTransactionPropertyIds() const;
  /**
   * @brief Returns the actor id selected on the current transaction draft.
   * @return Actor id.
   */
  QString currentTransactionActorId() const;

  ImportWorkflow* importWorkflow_ = nullptr;
  Settings* settings_ = nullptr;
  Actions* actions_ = nullptr;
  NavigationState* navigation_ = nullptr;
  StatusState* status_ = nullptr;
  WorkspaceStore *store_ = nullptr;
  WorkspaceCommands *commands_ = nullptr;
  WorkspaceSelection *selection_ = nullptr;
  WorkspaceSelectors *selectors_ = nullptr;
  QString manualPathText_;
  QStringList selectedImportFiles_;
  QString appliedDefaultImportPath_;

  TransactionDraftView currentTransactionView_;
  QString lastTransactionId_;
  QString nameText_;
  QString bookingDateText_;
  QString valutaText_;
  QString amountText_;
  QString actorName_;
  QString contractName_;
  QString contractType_;
  QString contractAllocatableMode_;
  QString propertyName_;
};

} // namespace ui
