/**
 * @file ui/include/ui/viewmodels/ExportViewModel.h
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
class ExportWorkflow;
class FileSystemBrowser;
class Settings;
class WorkspaceFacade;

/**
 * @brief Owns Export view state and delegates export execution to
 * ExportWorkflow.
 */
class ExportViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ExportViewModel)

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
  Q_PROPERTY(QVariantList exportLogs READ exportLogs NOTIFY changed)

public:
  /**
   * @brief Creates the export view model.
   * @param parent Optional QObject parent.
   */
  explicit ExportViewModel(QObject *parent = nullptr);

  /** @brief Binds the workspace facade used for catalog rows and export logs. */
  void setWorkspace(WorkspaceFacade *value);
  /** @brief Binds the workflow used to execute exports. */
  void setExportWorkflow(ExportWorkflow *value);
  /** @brief Binds shell actions such as the export directory picker. */
  void setActions(Actions *value);
  /** @brief Binds filesystem defaults used by the export form. */
  void setFileSystemBrowser(FileSystemBrowser *value);
  /** @brief Binds export settings used by defaults and package options. */
  void setSettings(Settings *value);

  /** @brief Returns the selected export target directory. */
  QString targetDirectory() const { return targetDirectory_; }
  /** @brief Updates the selected export target directory. */
  void setTargetDirectory(const QString &value);
  /** @brief Returns the selected package format index. */
  int packageFormatIndex() const noexcept { return packageFormatIndex_; }
  /** @brief Updates the selected package format index. */
  void setPackageFormatIndex(int value);
  /** @brief Returns whether the add combobox currently shows annuals or analyses. */
  QString addMode() const { return addMode_; }
  /** @brief Switches the add combobox between annual and analysis mode. */
  void setAddMode(const QString &value);

  /** @brief Returns annual rows available for export selection. */
  QVariantList annualRows() const;
  /** @brief Returns analysis rows available for export selection. */
  QVariantList analysisRows() const;
  /** @brief Returns the rows shown by the current add combobox mode. */
  QVariantList addRows() const;
  /** @brief Returns the display role used by the current add combobox mode. */
  QString addTextRole() const;
  /** @brief Returns the selected index in the current add combobox mode. */
  int pendingIndex() const;
  /** @brief Returns whether the selected row can be added to the export package. */
  bool canAddEntry() const;
  /** @brief Returns the currently configured export package entries. */
  QVariantList exportEntries() const { return exportEntries_; }
  /** @brief Returns whether the export can be started. */
  bool canStart() const;
  /** @brief Returns whether the clear button should be visible. */
  bool showClear() const;
  /** @brief Returns whether the cancel button should be visible. */
  bool showCancel() const;
  /** @brief Returns whether the pause/resume button should be visible. */
  bool showPause() const;
  /** @brief Returns the current pause/resume button text. */
  QString pauseText() const;
  /** @brief Returns the active export progress. */
  double progress() const;
  /** @brief Returns the current export status text. */
  QString statusText() const;
  /** @brief Returns whether the workflow currently reports an error. */
  bool hasError() const;
  /** @brief Returns export log rows for the sidebar. */
  QVariantList exportLogs() const;

  /** @brief Refreshes export form rows from the current workspace state. */
  Q_INVOKABLE void refreshFromWorkspace();
  /** @brief Opens the export directory picker through shell actions. */
  Q_INVOKABLE void browseDirectory();
  /** @brief Clears the export form back to defaults. */
  Q_INVOKABLE void clearForm();
  /** @brief Starts the export workflow with the current package configuration. */
  Q_INVOKABLE void startExport();
  /** @brief Cancels the active export workflow. */
  Q_INVOKABLE void cancelExport();
  /** @brief Toggles pause/resume on the active export workflow. */
  Q_INVOKABLE void togglePause();
  /** @brief Refreshes export logs from workspace state. */
  Q_INVOKABLE void refreshExportLogs();
  /**
   * @brief Opens the target folder for an export log.
   * @param index Fallback row index when no log id is provided.
   * @param logId Stable export log id.
   */
  Q_INVOKABLE void openExportLogLocation(int index, const QString &logId = {});
  /**
   * @brief Deletes an export log from the workspace.
   * @param index Fallback row index when no log id is provided.
   * @param logId Stable export log id.
   */
  Q_INVOKABLE void deleteExportLog(int index, const QString &logId = {});
  /** @brief Selects a row in the current add combobox. */
  Q_INVOKABLE void selectPendingRow(int index);
  /** @brief Adds the selected annual or analysis to the export package. */
  Q_INVOKABLE void addPendingEntry();
  /** @brief Removes an export package entry. */
  Q_INVOKABLE void removeEntry(int index);
  /** @brief Replaces an annual package entry with another annual row. */
  Q_INVOKABLE void updateAnnualEntryAtIndex(int entryIndex, int annualIndex);
  /** @brief Updates collapsed state for an annual package entry. */
  Q_INVOKABLE void updateAnnualCollapsed(int entryIndex, bool collapsed);
  /** @brief Replaces a standalone analysis package entry. */
  Q_INVOKABLE void updateStandaloneAnalysisAtIndex(int entryIndex,
                                                   int analysisIndex);
  /** @brief Updates export type for a standalone analysis entry. */
  Q_INVOKABLE void
  updateStandaloneAnalysisExportType(int entryIndex, const QString &exportType);
  /** @brief Updates export type for an analysis inside an annual entry. */
  Q_INVOKABLE void updateAnnualAnalysisExportType(int entryIndex,
                                                  int analysisIndex,
                                                  const QString &exportType);
  /** @brief Returns the flattened package items consumed by ExportWorkflow. */
  Q_INVOKABLE QVariantList exportItems() const;
  /** @brief Clears package entries without changing form defaults. */
  Q_INVOKABLE void clearAll();
  /** @brief Loads package entries from flattened item rows. */
  Q_INVOKABLE void loadItems(const QVariantList &items);

signals:
  void changed();

private:
  bool isAnnualMode() const;
  QVariantMap annualRowById(const QString &id) const;
  QVariantMap analysisRowById(const QString &id) const;
  QString pendingObjectId() const;
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
  void bindSettings(Settings *value);
  void bindWorkflow(ExportWorkflow *value);
  void configureExportLogSink();
  void emitChanged();

  WorkspaceFacade *workspace_ = nullptr;
  ExportWorkflow *exportWorkflow_ = nullptr;
  Actions *actions_ = nullptr;
  FileSystemBrowser *fileSystemBrowser_ = nullptr;
  Settings *settings_ = nullptr;
  QString targetDirectory_;
  QString appliedDefaultTargetDirectory_;
  int packageFormatIndex_ = 0;
  QString addMode_ = QStringLiteral("annual");
  QString pendingAnnualId_;
  QString pendingAnalysisId_;
  QVariantList exportEntries_;
};

} // namespace ui
