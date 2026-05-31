/**
 * @file ui/include/ui/state/export/ExportState.h
 * @brief Declares the UI state adapter for the Export view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace ui {

class Actions;
class ExportRunList;
class ExportWorkflow;
class FileSystemBrowser;
class SettingsViewModel;
class WorkspaceFacade;

/**
 * @brief Owns Export view state and delegates export execution to ExportWorkflow.
 */
class ExportState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ExportState)

  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(ui::ExportWorkflow *exportWorkflow READ exportWorkflow WRITE
                 setExportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::Actions *actions READ actions WRITE setActions NOTIFY changed)
  Q_PROPERTY(ui::FileSystemBrowser *fileSystemBrowser READ fileSystemBrowser
                 WRITE setFileSystemBrowser NOTIFY changed)
  Q_PROPERTY(ui::SettingsViewModel *settings READ settings WRITE setSettings
                 NOTIFY changed)
  Q_PROPERTY(QString targetDirectory READ targetDirectory WRITE
                 setTargetDirectory NOTIFY changed)
  Q_PROPERTY(int packageFormatIndex READ packageFormatIndex WRITE
                 setPackageFormatIndex NOTIFY changed)
  Q_PROPERTY(QString addMode READ addMode WRITE setAddMode NOTIFY changed)
  Q_PROPERTY(QVariantList annualRows READ annualRows NOTIFY changed)
  Q_PROPERTY(QVariantList analysisRows READ analysisRows NOTIFY changed)
  Q_PROPERTY(QVariantList addRows READ addRows NOTIFY changed)
  Q_PROPERTY(QString addTextRole READ addTextRole NOTIFY changed)
  Q_PROPERTY(int pendingIndex READ pendingIndex NOTIFY changed)
  Q_PROPERTY(bool canAddEntry READ canAddEntry NOTIFY changed)
  Q_PROPERTY(QVariantList exportEntries READ exportEntries NOTIFY changed)
  Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
  Q_PROPERTY(bool showClear READ showClear NOTIFY changed)
  Q_PROPERTY(bool showCancel READ showCancel NOTIFY changed)
  Q_PROPERTY(bool showPause READ showPause NOTIFY changed)
  Q_PROPERTY(QString pauseText READ pauseText NOTIFY changed)
  Q_PROPERTY(double progress READ progress NOTIFY changed)
  Q_PROPERTY(QString statusText READ statusText NOTIFY changed)
  Q_PROPERTY(bool hasError READ hasError NOTIFY changed)
  Q_PROPERTY(ui::ExportRunList *runs READ runs NOTIFY changed)

public:
  explicit ExportState(QObject *parent = nullptr);

  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  ExportWorkflow *exportWorkflow() const noexcept { return exportWorkflow_; }
  Actions *actions() const noexcept { return actions_; }
  FileSystemBrowser *fileSystemBrowser() const noexcept {
    return fileSystemBrowser_;
  }
  SettingsViewModel *settings() const noexcept { return settings_; }

  void setWorkspace(WorkspaceFacade *value);
  void setExportWorkflow(ExportWorkflow *value);
  void setActions(Actions *value);
  void setFileSystemBrowser(FileSystemBrowser *value);
  void setSettings(SettingsViewModel *value);

  QString targetDirectory() const { return targetDirectory_; }
  void setTargetDirectory(const QString &value);
  int packageFormatIndex() const noexcept { return packageFormatIndex_; }
  void setPackageFormatIndex(int value);
  QString addMode() const { return addMode_; }
  void setAddMode(const QString &value);

  QVariantList annualRows() const;
  QVariantList analysisRows() const;
  QVariantList addRows() const;
  QString addTextRole() const;
  int pendingIndex() const;
  bool canAddEntry() const;
  QVariantList exportEntries() const { return exportEntries_; }
  bool canStart() const;
  bool showClear() const;
  bool showCancel() const;
  bool showPause() const;
  QString pauseText() const;
  double progress() const;
  QString statusText() const;
  bool hasError() const;
  ExportRunList *runs() const;

  Q_INVOKABLE void refreshFromWorkspace();
  Q_INVOKABLE void browseDirectory();
  Q_INVOKABLE void clearForm();
  Q_INVOKABLE void startExport();
  Q_INVOKABLE void cancelExport();
  Q_INVOKABLE void togglePause();
  Q_INVOKABLE void refreshRuns();
  Q_INVOKABLE void openRunLocationAt(int index);
  Q_INVOKABLE void removeRunAt(int index);
  Q_INVOKABLE void selectPendingRow(int index);
  Q_INVOKABLE void addPendingEntry();
  Q_INVOKABLE void removeEntry(int index);
  Q_INVOKABLE void updateAnnualEntryAtIndex(int entryIndex, int annualIndex);
  Q_INVOKABLE void updateAnnualCollapsed(int entryIndex, bool collapsed);
  Q_INVOKABLE void updateStandaloneAnalysisAtIndex(int entryIndex,
                                                   int analysisIndex);
  Q_INVOKABLE void updateStandaloneAnalysisExportType(int entryIndex,
                                                      const QString &exportType);
  Q_INVOKABLE void updateAnnualAnalysisExportType(int entryIndex,
                                                  int analysisIndex,
                                                  const QString &exportType);
  Q_INVOKABLE QVariantList exportItems() const;
  Q_INVOKABLE void clearAll();
  Q_INVOKABLE void loadItems(const QVariantList &items);

signals:
  void changed();

private:
  bool isAnnualMode() const;
  QVariantMap annualRowById(const QString &id) const;
  QVariantMap analysisRowById(const QString &id) const;
  QString pendingObjectId() const;
  int indexForId(const QVariantList &rows, const QString &id) const;
  void ensurePendingSelection();
  void refreshEntriesFromWorkspace();
  QVariantMap createAnnualEntry(const QString &id, const QString &name,
                                const QVariantList &analyses) const;
  QVariantMap createAnalysisEntry(const QString &id, const QString &name,
                                  const QString &type,
                                  const QString &exportType) const;
  QVariantList analysesForAnnual(const QString &annualId,
                                 const QVariantList &currentAnalyses) const;
  QVariantList exportOptionsForAnalysisType(const QString &type) const;
  QString normalizedExportType(const QString &exportType,
                               const QString &type) const;
  QString defaultExportType(const QString &type) const;
  QString analysisTypeById(const QString &id) const;
  QString defaultTargetDirectory() const;
  QString payloadJson() const;
  QString defaultLocale() const;
  int workflowMode() const;
  void bindWorkspace(WorkspaceFacade *value);
  void bindActions(Actions *value);
  void bindSettings(SettingsViewModel *value);
  void bindWorkflow(ExportWorkflow *value);
  void emitChanged();

  WorkspaceFacade *workspace_ = nullptr;
  ExportWorkflow *exportWorkflow_ = nullptr;
  Actions *actions_ = nullptr;
  FileSystemBrowser *fileSystemBrowser_ = nullptr;
  SettingsViewModel *settings_ = nullptr;
  QString targetDirectory_;
  QString appliedDefaultTargetDirectory_;
  int packageFormatIndex_ = 0;
  QString addMode_ = QStringLiteral("annual");
  QString pendingAnnualId_;
  QString pendingAnalysisId_;
  QVariantList exportEntries_;
};

} // namespace ui
