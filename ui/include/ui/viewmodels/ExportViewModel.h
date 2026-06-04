/**
 * @file ui/include/ui/viewmodels/ExportViewModel.h
 * @brief Declares the QML API for the Export view.
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
  Q_PROPERTY(int selectedAddIndex READ selectedAddIndex NOTIFY changed)
  Q_PROPERTY(bool canAddEntry READ canAddEntry NOTIFY changed)
  Q_PROPERTY(QVariantList exportEntries READ exportEntries NOTIFY changed)
  Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
  Q_PROPERTY(bool showClear READ showClear NOTIFY changed)
  Q_PROPERTY(bool showCancel READ showCancel NOTIFY changed)
  Q_PROPERTY(bool showPause READ showPause NOTIFY changed)
  Q_PROPERTY(bool isPaused READ isPaused NOTIFY changed)
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

  /**
   * @brief Binds the workspace facade used for catalog rows and export logs.
   * @param value Workspace facade or nullptr.
   */
  void setWorkspace(WorkspaceFacade *value);
  /**
   * @brief Binds the workflow used to execute exports.
   * @param value Export workflow or nullptr.
   */
  void setExportWorkflow(ExportWorkflow *value);
  /**
   * @brief Binds shell actions such as the export directory picker.
   * @param value Actions object or nullptr.
   */
  void setActions(Actions *value);
  /**
   * @brief Binds filesystem defaults used by the export form.
   * @param value File system browser or nullptr.
   */
  void setFileSystemBrowser(FileSystemBrowser *value);
  /**
   * @brief Binds export settings used by defaults and package options.
   * @param value Settings object or nullptr.
   */
  void setSettings(Settings *value);

  /**
   * @brief Returns the selected export target directory.
   * @return Target directory path.
   */
  QString targetDirectory() const { return targetDirectory_; }
  /**
   * @brief Updates the selected export target directory.
   * @param value Target directory path.
   */
  void setTargetDirectory(const QString &value);
  /**
   * @brief Returns the selected package format index.
   * @return Package format index.
   */
  int packageFormatIndex() const noexcept { return packageFormatIndex_; }
  /**
   * @brief Updates the selected package format index.
   * @param value Package format index.
   */
  void setPackageFormatIndex(int value);
  /**
   * @brief Returns whether the add combobox currently shows annuals or analyses.
   * @return Add mode key.
   */
  QString addMode() const { return addMode_; }
  /**
   * @brief Switches the add combobox between annual and analysis mode.
   * @param value Add mode key.
   */
  void setAddMode(const QString &value);

  /**
   * @brief Returns annual rows available for export selection.
   * @return Annual rows.
   */
  QVariantList annualRows() const;
  /**
   * @brief Returns analysis rows available for export selection.
   * @return Analysis rows.
   */
  QVariantList analysisRows() const;
  /**
   * @brief Returns the rows shown by the current add combobox mode.
   * @return Current add rows.
   */
  QVariantList addRows() const;
  /**
   * @brief Returns the display role used by the current add combobox mode.
   * @return Display role name.
   */
  QString addTextRole() const;
  /**
   * @brief Returns the selected index in the current add combobox mode.
   * @return Selected row index.
   */
  int selectedAddIndex() const;
  /**
   * @brief Returns whether the selected row can be added to the export package.
   * @return True when add is available.
   */
  bool canAddEntry() const;
  /**
   * @brief Returns the currently configured export package entries.
   * @return Export package entries.
   */
  QVariantList exportEntries() const { return exportEntries_; }
  /**
   * @brief Returns whether the export can be started.
   * @return True when export can start.
   */
  bool canStart() const;
  /**
   * @brief Returns whether the clear button should be visible.
   * @return True when clear should be shown.
   */
  bool showClear() const;
  /**
   * @brief Returns whether the cancel button should be visible.
   * @return True when cancel should be shown.
   */
  bool showCancel() const;
  /**
   * @brief Returns whether the pause/resume button should be visible.
   * @return True when pause/resume should be shown.
   */
  bool showPause() const;
  /**
   * @brief Returns whether the active export is paused.
   * @return True when the export workflow is paused.
   */
  bool isPaused() const;
  /**
   * @brief Returns the current pause/resume button text.
   * @return Pause/resume text.
   */
  QString pauseText() const;
  /**
   * @brief Returns the active export progress.
   * @return Progress value.
   */
  double progress() const;
  /**
   * @brief Returns the current export status text.
   * @return Status text.
   */
  QString statusText() const;
  /**
   * @brief Returns whether the workflow currently reports an error.
   * @return True when an error is reported.
   */
  bool hasError() const;
  /**
   * @brief Returns export log rows for the sidebar.
   * @return Export log rows.
   */
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
  /** @brief Pauses the active export workflow. */
  Q_INVOKABLE void pauseExport();
  /** @brief Resumes the active export workflow. */
  Q_INVOKABLE void resumeExport();
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
  /**
   * @brief Selects a row in the current add combobox.
   * @param index Index into addRows().
   */
  Q_INVOKABLE void selectAddRow(int index);
  /** @brief Adds the selected annual or analysis to the export package. */
  Q_INVOKABLE void addSelectedEntry();
  /**
   * @brief Removes an export package entry.
   * @param index Entry index.
   */
  Q_INVOKABLE void removeEntry(int index);
  /**
   * @brief Replaces an annual package entry with another annual row.
   * @param entryIndex Export entry index.
   * @param annualIndex Index into annualRows().
   */
  Q_INVOKABLE void updateAnnualEntryAtIndex(int entryIndex, int annualIndex);
  /**
   * @brief Updates collapsed state for an annual package entry.
   * @param entryIndex Export entry index.
   * @param collapsed True when collapsed.
   */
  Q_INVOKABLE void updateAnnualCollapsed(int entryIndex, bool collapsed);
  /**
   * @brief Replaces a standalone analysis package entry.
   * @param entryIndex Export entry index.
   * @param analysisIndex Index into analysisRows().
   */
  Q_INVOKABLE void updateStandaloneAnalysisAtIndex(int entryIndex,
                                                   int analysisIndex);
  /**
   * @brief Updates export type for a standalone analysis entry.
   * @param entryIndex Export entry index.
   * @param exportType Export type key.
   */
  Q_INVOKABLE void updateStandaloneAnalysisExportType(
      int entryIndex, const QString &exportType);
  /**
   * @brief Updates export type for an analysis inside an annual entry.
   * @param entryIndex Export entry index.
   * @param analysisIndex Analysis index inside the annual entry.
   * @param exportType Export type key.
   */
  Q_INVOKABLE void updateAnnualAnalysisExportType(int entryIndex,
                                                  int analysisIndex,
                                                  const QString &exportType);
signals:
  void changed();

private:
  /**
   * @brief Checks whether the add form is in annual mode.
   * @return True for annual mode.
   */
  bool isAnnualMode() const;
  /**
   * @brief Returns an annual row by id.
   * @param id Annual id.
   * @return Annual row or empty map.
   */
  QVariantMap annualRowById(const QString &id) const;
  /**
   * @brief Returns an analysis row by id.
   * @param id Analysis id.
   * @return Analysis row or empty map.
   */
  QVariantMap analysisRowById(const QString &id) const;
  /**
   * @brief Returns the selected object id for the current add mode.
   * @return Selected annual or analysis id.
   */
  QString selectedAddObjectId() const;
  /**
   * @brief Ensures the add combobox has a valid selection.
   */
  void ensureAddSelection();
  /**
   * @brief Refreshes package entry display data from workspace rows.
   */
  void refreshEntriesFromWorkspace();
  /**
   * @brief Returns the flattened package items consumed by ExportWorkflow.
   * @return Export item rows.
   */
  QVariantList exportItems() const;
  /**
   * @brief Builds an annual package entry.
   * @param id Annual id.
   * @param name Annual display name.
   * @param analyses Nested analysis entries.
   * @return Export entry row.
   */
  QVariantMap createAnnualEntry(const QString &id, const QString &name,
                                const QVariantList &analyses) const;
  /**
   * @brief Builds a standalone analysis package entry.
   * @param id Analysis id.
   * @param name Analysis display name.
   * @param type Analysis type.
   * @param exportType Export type key.
   * @return Export entry row.
   */
  QVariantMap createAnalysisEntry(const QString &id, const QString &name,
                                  const QString &type,
                                  const QString &exportType) const;
  /**
   * @brief Builds nested analysis entries for an annual.
   * @param annualId Annual id.
   * @param currentAnalyses Existing nested entries.
   * @return Nested analysis entries.
   */
  QVariantList analysesForAnnual(const QString &annualId,
                                 const QVariantList &currentAnalyses) const;
  /**
   * @brief Returns export type options for an analysis type.
   * @param type Analysis type.
   * @return Export option rows.
   */
  QVariantList exportOptionsForAnalysisType(const QString &type) const;
  /**
   * @brief Normalizes an export type for an analysis type.
   * @param exportType Raw export type.
   * @param type Analysis type.
   * @return Normalized export type.
   */
  QString normalizedExportType(const QString &exportType,
                               const QString &type) const;
  /**
   * @brief Returns the default export type for an analysis type.
   * @param type Analysis type.
   * @return Export type key.
   */
  QString defaultExportType(const QString &type) const;
  /**
   * @brief Returns analysis type for an analysis id.
   * @param id Analysis id.
   * @return Analysis type key.
   */
  QString analysisTypeById(const QString &id) const;
  /**
   * @brief Returns the default target directory.
   * @return Directory path.
   */
  QString defaultTargetDirectory() const;
  /**
   * @brief Builds the package payload for the workflow.
   * @return Export package payload.
   */
  QVariantMap payload() const;
  /**
   * @brief Returns the default locale for export formatting.
   * @return Locale name.
   */
  QString defaultLocale() const;
  /**
   * @brief Returns the current workflow mode.
   * @return Workflow mode index.
   */
  int workflowMode() const;
  /**
   * @brief Connects workspace signals.
   * @param value Workspace facade or nullptr.
   */
  void bindWorkspace(WorkspaceFacade *value);
  /**
   * @brief Connects shell action signals.
   * @param value Actions object or nullptr.
   */
  void bindActions(Actions *value);
  /**
   * @brief Connects settings signals.
   * @param value Settings object or nullptr.
   */
  void bindSettings(Settings *value);
  /**
   * @brief Connects workflow signals.
   * @param value Export workflow or nullptr.
   */
  void bindWorkflow(ExportWorkflow *value);
  /**
   * @brief Configures the workflow export-log sink.
   */
  void configureExportLogSink();
  /**
   * @brief Emits the shared changed signal.
   */
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
  QString selectedAddAnnualId_;
  QString selectedAddAnalysisId_;
  QVariantList exportEntries_;
};

} // namespace ui
