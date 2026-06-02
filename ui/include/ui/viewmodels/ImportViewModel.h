/**
 * @file ui/include/ui/viewmodels/ImportViewModel.h
 * @brief Declares the UI state adapter for the import overview page.
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
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

/**
 * @brief Exposes import overview state and commands to QML.
 *
 * ImportViewModel is the API surface for the import start page, import
 * controls, and import log sidebar. Draft detail editing belongs to
 * StatementDraftViewModel and TransactionDraftViewModel.
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
  Q_PROPERTY(
      ui::ImportWorkflow *importWorkflow READ importWorkflow NOTIFY changed)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace NOTIFY changed)
  Q_PROPERTY(ui::NavigationState *navigation READ navigation NOTIFY changed)
  Q_PROPERTY(int queuedCount READ queuedCount NOTIFY changed)
  Q_PROPERTY(QVariantList importLogs READ importLogs NOTIFY changed)
  Q_PROPERTY(QString selectedDraftId READ selectedDraftId NOTIFY changed)
  Q_PROPERTY(QStringList importSourceLabels READ importSourceLabels CONSTANT)
  Q_PROPERTY(QStringList statementStrategyLabels READ statementStrategyLabels
                 CONSTANT)

public:
  /**
   * @brief Creates an import overview view model.
   * @param parent Optional QObject parent.
   */
  explicit ImportViewModel(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }

  /**
   * @brief Returns settings used for import defaults.
   * @return Bound settings, or nullptr when not wired.
   */
  Settings *settings() const noexcept { return settings_; }

  /**
   * @brief Returns the application actions bridge used for file dialogs.
   * @return Bound action object, or nullptr when not wired.
   */
  Actions *actions() const noexcept { return actions_; }

  /**
   * @brief Returns navigation state used when opening imported statements.
   * @return Bound navigation state, or nullptr when not wired.
   */
  NavigationState *navigation() const noexcept { return navigation_; }

  /**
   * @brief Returns status state used for import status messages.
   * @return Bound status state, or nullptr when not wired.
   */
  StatusState *status() const noexcept { return status_; }

  /**
   * @brief Returns workspace facade used for selecting imported statements.
   * @return Bound workspace facade, or nullptr when not wired.
   */
  WorkspaceFacade *workspace() const noexcept { return workspace_; }

  /**
   * @brief Binds the import workflow and listens for workflow state changes.
   * @param value Workflow to expose to QML; may be nullptr.
   */
  void setImportWorkflow(ImportWorkflow *value);

  /**
   * @brief Binds settings and listens for import default path changes.
   * @param value Settings to read from; may be nullptr.
   */
  void setSettings(Settings *value);

  /**
   * @brief Binds application actions for file picking and drag events.
   * @param value Actions object to use; may be nullptr.
   */
  void setActions(Actions *value);

  /**
   * @brief Binds navigation state for view changes triggered by import logs.
   * @param value Navigation state to update; may be nullptr.
   */
  void setNavigation(NavigationState *value);

  /**
   * @brief Binds status state for visible import messages.
   * @param value Status state to update; may be nullptr.
   */
  void setStatus(StatusState *value);

  /**
   * @brief Binds workspace facade and refreshes import state from workspace.
   * @param value Workspace facade to use; may be nullptr.
   */
  void setWorkspace(WorkspaceFacade *value);

  /**
   * @brief Returns the visible content index for the import overview stack.
   * @return 1 when a statement draft is open, otherwise 0.
   */
  int contentIndex() const noexcept;

  /**
   * @brief Reports whether an import workflow is available.
   * @return True when importWorkflow() is not nullptr.
   */
  bool hasImportWorkflow() const noexcept;

  /**
   * @brief Reports whether the workflow currently exposes a statement draft.
   * @return True when a draft is active.
   */
  bool hasDraft() const noexcept;

  /**
   * @brief Reports whether multiple statement drafts can be navigated.
   * @return True when the workflow has draft navigation state.
   */
  bool hasDraftNavigation() const noexcept;

  /**
   * @brief Reports whether the import overview can be cleared.
   * @return True when a workflow is bound and no import is running.
   */
  bool canClearImport() const noexcept;

  /**
   * @brief Reports whether the current import can be canceled.
   * @return True while an import is running.
   */
  bool canCancel() const noexcept;

  /**
   * @brief Reports whether the current import can be paused or resumed.
   * @return True while an import is running.
   */
  bool canPause() const noexcept;

  /**
   * @brief Reports whether an import can be started.
   * @return True when a selected or queued file exists and no import is
   * running.
   */
  bool canStart() const noexcept;

  /**
   * @brief Reports whether an import job is currently running.
   * @return True while the workflow is running.
   */
  bool importRunning() const noexcept;

  /**
   * @brief Reports whether the running import is paused.
   * @return True when the workflow is paused.
   */
  bool importPaused() const noexcept;

  /**
   * @brief Returns the label for the pause/resume button.
   * @return Translated "Pause" or "Resume".
   */
  QString pauseText() const;

  /**
   * @brief Returns the visible import progress text.
   * @return Error text, workflow phase, or a translated ready text.
   */
  QString progressText() const;

  /**
   * @brief Reports whether the progress text represents an error.
   * @return True when the workflow has a non-empty error.
   */
  bool progressHasError() const noexcept;

  /**
   * @brief Returns the numeric import progress value.
   * @return Workflow progress in the range provided by the workflow.
   */
  double progressValue() const noexcept;

  /**
   * @brief Returns the manual import path text field value.
   * @return Current manual path text.
   */
  QString manualPathText() const { return manualPathText_; }

  /**
   * @brief Updates the manual import path text and clears pending picked files.
   * @param value New text field value.
   */
  void setManualPathText(const QString &value);

  /**
   * @brief Returns selected and queued import files for display.
   * @return File paths currently known to the import workflow.
   */
  QStringList importFiles() const;

  /**
   * @brief Returns a human-readable summary of selected import files.
   * @return Translated summary text, or an empty string when no file is queued.
   */
  QString importFileSummary() const;

  /**
   * @brief Returns the number of queued import files.
   * @return Workflow queued file count, or 0 without a workflow.
   */
  int queuedCount() const noexcept;

  /**
   * @brief Returns import logs for the sidebar.
   * @return QML-friendly import log entries from the workspace.
   */
  QVariantList importLogs() const;

  /**
   * @brief Returns the currently open statement draft id.
   * @return Current draft id, or an empty string without a workflow.
   */
  QString selectedDraftId() const;
  QStringList importSourceLabels() const;
  QStringList statementStrategyLabels() const;

  /**
   * @brief Initializes the import view and applies the default import path.
   */
  Q_INVOKABLE void initializeImportView();

  /**
   * @brief Opens the import PDF file picker.
   */
  Q_INVOKABLE void browseImportPdf();

  /**
   * @brief Adds the manually entered or picked files to the import queue.
   */
  Q_INVOKABLE void addSelectedImportFiles();

  /**
   * @brief Clears import overview status through the workflow.
   */
  Q_INVOKABLE void clearImport();

  /**
   * @brief Cancels the currently running import.
   */
  Q_INVOKABLE void cancelCurrentImport();

  /**
   * @brief Cancels the current import and all queued import files.
   */
  Q_INVOKABLE void cancelAllImports();

  /**
   * @brief Pauses the current import when it is running and not paused.
   */
  Q_INVOKABLE void pauseImport();

  /**
   * @brief Resumes the current import when it is running and paused.
   */
  Q_INVOKABLE void resumeImport();

  /**
   * @brief Starts import execution for the selected or queued files.
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
   * @brief Opens a sidebar import log.
   * @param logId Import log id carried by the row.
   * @param draftAttached True when the log points to a persisted draft.
   * @param statementId Statement id to open for finalized logs.
   * @param draftId Draft id to open for draft logs.
   */
  Q_INVOKABLE void openImportLog(const QString &logId, bool draftAttached,
                                 const QString &statementId,
                                 const QString &draftId);

  /**
   * @brief Deletes a sidebar import log and its draft when attached.
   * @param logId Import log id carried by the row.
   * @param draftAttached True when a persisted draft should also be deleted.
   * @param draftId Draft id to delete when draftAttached is true.
   */
  Q_INVOKABLE void deleteImportLog(const QString &logId, bool draftAttached,
                                   const QString &draftId);

signals:
  /**
   * @brief Emitted whenever visible import overview state changes.
   */
  void changed();

private:
  /**
   * @brief Replaces the bound import workflow and connects workflow signals.
   * @param value Workflow to bind; may be nullptr.
   */
  void bindImportWorkflow(ImportWorkflow *value);

  /**
   * @brief Replaces the bound settings store and connects settings signals.
   * @param value Settings to bind; may be nullptr.
   */
  void bindSettings(Settings *value);

  /**
   * @brief Replaces the bound actions object and connects file events.
   * @param value Actions object to bind; may be nullptr.
   */
  void bindActions(Actions *value);

  /**
   * @brief Replaces the bound workspace facade and connects revision changes.
   * @param value Workspace facade to bind; may be nullptr.
   */
  void bindWorkspace(WorkspaceFacade *value);

  /**
   * @brief Applies the configured default import path when it can be adopted.
   */
  void applyDefaultImportSelection();

  /**
   * @brief Copies the workflow-selected file into the manual path field.
   */
  void updateManualPathFromWorkflow();

  /**
   * @brief Stores picked files before the user adds them to the import queue.
   * @param paths Picked file paths.
   */
  void setPendingFiles(const QStringList &paths);

  /**
   * @brief Writes visible status text through StatusState.
   * @param text Status text to display.
   */
  void setStatusText(const QString &text);

  /**
   * @brief Filters and queues supported import files in the workflow.
   * @param paths Candidate file paths.
   */
  void queueImportFiles(const QStringList &paths);

  /**
   * @brief Filters a list of paths to supported import files.
   * @param paths Candidate file paths.
   * @return PDF paths accepted for import.
   */
  QStringList supportedImportFiles(const QStringList &paths) const;
  int activeDraftStackIndex() const;
  bool openDraftAtStackIndex(int index);

  ImportWorkflow *importWorkflow_ = nullptr;
  Settings *settings_ = nullptr;
  Actions *actions_ = nullptr;
  NavigationState *navigation_ = nullptr;
  StatusState *status_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
  QString manualPathText_;
  QStringList pendingFiles_;
  QString appliedDefaultImportPath_;
};

} // namespace ui
