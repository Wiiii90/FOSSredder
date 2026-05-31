/**
 * @file ui/include/ui/state/import/ImportState.h
 * @brief Declares the UI state adapter for the import overview page.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <qqmlintegration.h>

#include "ui/shell/AppActions.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/state/status/StatusState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

class ImportState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ImportState)

  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::SettingsViewModel *settingsViewModel READ settingsViewModel
                 WRITE setSettingsViewModel NOTIFY changed)
  Q_PROPERTY(ui::Actions *actions READ actions WRITE setActions NOTIFY changed)
  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY changed)
  Q_PROPERTY(ui::StatusState *status READ status WRITE setStatus NOTIFY changed)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(bool pageActivated READ pageActivated WRITE setPageActivated NOTIFY
                 changed)
  Q_PROPERTY(int contentIndex READ contentIndex NOTIFY changed)
  Q_PROPERTY(bool hasImportWorkflow READ hasImportWorkflow NOTIFY changed)
  Q_PROPERTY(bool hasDraft READ hasDraft NOTIFY changed)
  Q_PROPERTY(bool hasDraftNavigation READ hasDraftNavigation NOTIFY changed)
  Q_PROPERTY(bool canOpenPreviousDraft READ canOpenPreviousDraft NOTIFY changed)
  Q_PROPERTY(bool canOpenNextDraft READ canOpenNextDraft NOTIFY changed)
  Q_PROPERTY(bool canClear READ canClear NOTIFY changed)
  Q_PROPERTY(bool canCancel READ canCancel NOTIFY changed)
  Q_PROPERTY(bool canCancelAll READ canCancelAll NOTIFY changed)
  Q_PROPERTY(bool canPause READ canPause NOTIFY changed)
  Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
  Q_PROPERTY(bool importRunning READ importRunning NOTIFY changed)
  Q_PROPERTY(QString pauseText READ pauseText NOTIFY changed)
  Q_PROPERTY(QString progressText READ progressText NOTIFY changed)
  Q_PROPERTY(bool progressHasError READ progressHasError NOTIFY changed)
  Q_PROPERTY(double progressValue READ progressValue NOTIFY changed)
  Q_PROPERTY(QString manualPathText READ manualPathText WRITE setManualPathText
                 NOTIFY changed)
  Q_PROPERTY(QStringList importFiles READ importFiles NOTIFY changed)
  Q_PROPERTY(int queuedCount READ queuedCount NOTIFY changed)
  Q_PROPERTY(QObject *runModel READ runModel NOTIFY changed)
  Q_PROPERTY(QString selectedRunLogId READ selectedRunLogId NOTIFY changed)
  Q_PROPERTY(ui::StatementDraft *currentDraft READ currentDraft NOTIFY changed)

public:
  explicit ImportState(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }
  SettingsViewModel *settingsViewModel() const noexcept {
    return settingsViewModel_;
  }
  Actions *actions() const noexcept { return actions_; }
  NavigationState *navigation() const noexcept { return navigation_; }
  StatusState *status() const noexcept { return status_; }
  WorkspaceFacade *workspace() const noexcept { return workspace_; }

  void setImportWorkflow(ImportWorkflow *value);
  void setSettingsViewModel(SettingsViewModel *value);
  void setActions(Actions *value);
  void setNavigation(NavigationState *value);
  void setStatus(StatusState *value);
  void setWorkspace(WorkspaceFacade *value);

  bool pageActivated() const noexcept { return pageActivated_; }
  void setPageActivated(bool value);

  int contentIndex() const noexcept;
  bool hasImportWorkflow() const noexcept;
  bool hasDraft() const noexcept;
  bool hasDraftNavigation() const noexcept;
  bool canOpenPreviousDraft() const noexcept;
  bool canOpenNextDraft() const noexcept;
  bool canClear() const noexcept;
  bool canCancel() const noexcept;
  bool canCancelAll() const noexcept;
  bool canPause() const noexcept;
  bool canStart() const noexcept;
  bool importRunning() const noexcept;
  QString pauseText() const;
  QString progressText() const;
  bool progressHasError() const noexcept;
  double progressValue() const noexcept;
  QString manualPathText() const { return manualPathText_; }
  void setManualPathText(const QString &value);
  QStringList importFiles() const;
  int queuedCount() const noexcept;
  QObject *runModel() const noexcept;
  QString selectedRunLogId() const;
  StatementDraft *currentDraft() const noexcept;

  Q_INVOKABLE void activatePage();
  Q_INVOKABLE void applyDefaultImportSelection();
  Q_INVOKABLE void browseImportPdf();
  Q_INVOKABLE void commitManualImportFiles();
  Q_INVOKABLE void addImportFiles(const QStringList &paths);
  Q_INVOKABLE void resetStatus();
  Q_INVOKABLE void cancelImport();
  Q_INVOKABLE void cancelAllImports();
  Q_INVOKABLE void togglePause();
  Q_INVOKABLE void startImport();
  Q_INVOKABLE void openPreviousDraft();
  Q_INVOKABLE void openNextDraft();
  Q_INVOKABLE void refreshFromWorkspace();
  Q_INVOKABLE void activateRun(int index, const QString &logId,
                               bool draftAttached,
                               const QString &statementId,
                               const QString &draftId);
  Q_INVOKABLE void deleteRun(int index, bool draftAttached,
                             const QString &draftId);

signals:
  void changed();

private:
  void bindImportWorkflow(ImportWorkflow *value);
  void bindSettings(SettingsViewModel *value);
  void bindActions(Actions *value);
  void bindWorkspace(WorkspaceFacade *value);
  void updateManualPathFromWorkflow();
  void setPendingFiles(const QStringList &paths);
  void setStatusText(const QString &text);
  QStringList supportedImportFiles(const QStringList &paths) const;

  ImportWorkflow *importWorkflow_ = nullptr;
  SettingsViewModel *settingsViewModel_ = nullptr;
  Actions *actions_ = nullptr;
  NavigationState *navigation_ = nullptr;
  StatusState *status_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
  bool pageActivated_ = false;
  QString manualPathText_;
  QStringList pendingFiles_;
  QString appliedDefaultImportPath_;
};

} // namespace ui
