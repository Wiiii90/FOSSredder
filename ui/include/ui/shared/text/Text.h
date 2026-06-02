/**
 * @file ui/include/ui/shared/text/Text.h
 * @brief Centralizes translatable UI text helpers used by widgets, workflows
 * and QML bridges.
 */

#pragma once

#include <QCoreApplication>
#include <QString>

namespace ui::text {

namespace detail {

/** @brief Translates a source string in the given Qt translation context. */
inline QString translate(const char *context, const char *source) {
  return QCoreApplication::translate(context, source);
}

} // namespace detail

namespace status {

inline constexpr auto kContext = "Messages";
inline constexpr auto kReadySource = QT_TRANSLATE_NOOP("Messages", "Ready");
inline constexpr auto kImportCanceledSource =
    QT_TRANSLATE_NOOP("Messages", "Import canceled");
inline constexpr auto kImportFinishedSource =
    QT_TRANSLATE_NOOP("Messages", "Import finished");
inline constexpr auto kImportFailedSource =
    QT_TRANSLATE_NOOP("Messages", "Import failed");

inline QString ready() { return detail::translate(kContext, kReadySource); }
inline QString importCanceled() {
  return detail::translate(kContext, kImportCanceledSource);
}
inline QString importFinished() {
  return detail::translate(kContext, kImportFinishedSource);
}
inline QString importFailed() {
  return detail::translate(kContext, kImportFailedSource);
}

} // namespace status

namespace actions {

inline constexpr auto kContext = "Actions";
inline constexpr auto kNewFileSource = QT_TRANSLATE_NOOP("Actions", "New...");
inline constexpr auto kOpenFileSource = QT_TRANSLATE_NOOP("Actions", "Open...");
inline constexpr auto kSaveFileSource = QT_TRANSLATE_NOOP("Actions", "Save");
inline constexpr auto kSaveFileAsSource =
    QT_TRANSLATE_NOOP("Actions", "Save As...");
inline constexpr auto kQuitSource = QT_TRANSLATE_NOOP("Actions", "Quit");
inline constexpr auto kAboutSource = QT_TRANSLATE_NOOP("Actions", "About");

inline QString newFile() { return detail::translate(kContext, kNewFileSource); }
inline QString openFile() {
  return detail::translate(kContext, kOpenFileSource);
}
inline QString saveFile() {
  return detail::translate(kContext, kSaveFileSource);
}
inline QString saveFileAs() {
  return detail::translate(kContext, kSaveFileAsSource);
}
inline QString quit() { return detail::translate(kContext, kQuitSource); }
inline QString about() { return detail::translate(kContext, kAboutSource); }

} // namespace actions

namespace dialogs {

inline constexpr auto kContext = "FileDialogs";
inline constexpr auto kSelectPdfTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Select PDF");
inline constexpr auto kExportFileTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Export File");
inline constexpr auto kExportDirectoryTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Select Export Directory");
inline constexpr auto kNewFileTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "New File");
inline constexpr auto kOpenFileTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Open File");
inline constexpr auto kSaveFileAsTitleSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Save File As");
inline constexpr auto kDatabaseFilterSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Database (*.db)");
inline constexpr auto kImportPdfFilterSource =
    QT_TRANSLATE_NOOP("FileDialogs", "PDF Files (*.pdf)");
inline constexpr auto kExportFileFilterSource =
    QT_TRANSLATE_NOOP("FileDialogs", "Excel Files (*.xlsx);;CSV Files (*.csv)");

inline QString selectPdfTitle() {
  return detail::translate(kContext, kSelectPdfTitleSource);
}
inline QString exportFileTitle() {
  return detail::translate(kContext, kExportFileTitleSource);
}
inline QString exportDirectoryTitle() {
  return detail::translate(kContext, kExportDirectoryTitleSource);
}
inline QString newFileTitle() {
  return detail::translate(kContext, kNewFileTitleSource);
}
inline QString openFileTitle() {
  return detail::translate(kContext, kOpenFileTitleSource);
}
inline QString saveFileAsTitle() {
  return detail::translate(kContext, kSaveFileAsTitleSource);
}
inline QString databaseFilter() {
  return detail::translate(kContext, kDatabaseFilterSource);
}
inline QString importPdfFilter() {
  return detail::translate(kContext, kImportPdfFilterSource);
}
inline QString exportFileFilter() {
  return detail::translate(kContext, kExportFileFilterSource);
}

} // namespace dialogs

namespace mainWindow {

inline constexpr auto kContext = "MainWindow";
inline constexpr auto kSelectedStatusPatternSource =
    QT_TRANSLATE_NOOP("MainWindow", "Selected: %1");
inline constexpr auto kExportPathStatusPatternSource =
    QT_TRANSLATE_NOOP("MainWindow", "Export path: %1");
inline constexpr auto kAboutTitleSource =
    QT_TRANSLATE_NOOP("MainWindow", "About FOSSredder");
inline constexpr auto kAboutHeadlineSource =
    QT_TRANSLATE_NOOP("MainWindow", "FOSSredder");
inline constexpr auto kAboutBodySource = QT_TRANSLATE_NOOP(
    "MainWindow",
    "Local-first document cleanup for bank statements, annual cost allocation, "
    "and the kind of Excel archaeology that quietly ruins "
    "weekends.\n\nFOSSredder extracts transactions from PDF statements, "
    "matches them to actors, properties, and contracts, and turns the result "
    "into analyses, annual reports, and exports.\n\nNo cloud. No external "
    "APIs. Deliberately overengineered for a very real workflow.");

inline QString selectedStatusPattern() {
  return detail::translate(kContext, kSelectedStatusPatternSource);
}
inline QString exportPathStatusPattern() {
  return detail::translate(kContext, kExportPathStatusPatternSource);
}
inline QString aboutTitle() {
  return detail::translate(kContext, kAboutTitleSource);
}
inline QString aboutHeadline() {
  return detail::translate(kContext, kAboutHeadlineSource);
}
inline QString aboutBody() {
  return detail::translate(kContext, kAboutBodySource);
}

} // namespace mainWindow

namespace workflowErrors {

inline constexpr auto kContext = "WorkflowErrors";
inline constexpr auto kStorageCreateFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Failed to create file");
inline constexpr auto kStorageOpenFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Failed to open file");
inline constexpr auto kStorageSaveFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Failed to save file");
inline constexpr auto kStorageSaveAsFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Failed to save file as");
inline constexpr auto kExportFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Export failed");
inline constexpr auto kExportStateUnavailableSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Export state not available");
inline constexpr auto kImportWorkflowUnavailableSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Import workflow not available");
inline constexpr auto kNoFileSelectedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "No file selected");
inline constexpr auto kImportFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Import failed");
inline constexpr auto kAnalysisEngineUnavailableSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Analysis engine not available");
inline constexpr auto kAnalysisStateUnavailableSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Analysis state not available");

inline QString storageCreateFailed() {
  return detail::translate(kContext, kStorageCreateFailedSource);
}
inline QString storageOpenFailed() {
  return detail::translate(kContext, kStorageOpenFailedSource);
}
inline QString storageSaveFailed() {
  return detail::translate(kContext, kStorageSaveFailedSource);
}
inline QString storageSaveAsFailed() {
  return detail::translate(kContext, kStorageSaveAsFailedSource);
}
inline QString exportFailed() {
  return detail::translate(kContext, kExportFailedSource);
}
inline QString exportStateUnavailable() {
  return detail::translate(kContext, kExportStateUnavailableSource);
}
inline QString importWorkflowUnavailable() {
  return detail::translate(kContext, kImportWorkflowUnavailableSource);
}
inline QString noFileSelected() {
  return detail::translate(kContext, kNoFileSelectedSource);
}
inline QString importFailed() {
  return detail::translate(kContext, kImportFailedSource);
}
inline QString analysisEngineUnavailable() {
  return detail::translate(kContext, kAnalysisEngineUnavailableSource);
}
inline QString analysisViewModelUnavailable() {
  return detail::translate(kContext, kAnalysisStateUnavailableSource);
}

} // namespace workflowErrors

namespace importLogs {

inline constexpr auto kContext = "ImportWorkflowState";
inline constexpr auto kTypeStatementSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Statement");
inline constexpr auto kStatusCanceledSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Canceled");
inline constexpr auto kStatusFailedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Failed");
inline constexpr auto kStatusSuccessSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Success");
inline constexpr auto kStatusRunningSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Running");
inline constexpr auto kStatusPausedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Paused");
inline constexpr auto kStatusDraftSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Draft");
inline constexpr auto kStatusFinalizedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Finalized");
inline constexpr auto kStatusDeletedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Deleted");

inline QString typeStatement() {
  return detail::translate(kContext, kTypeStatementSource);
}
inline QString statusCanceled() {
  return detail::translate(kContext, kStatusCanceledSource);
}
inline QString statusFailed() {
  return detail::translate(kContext, kStatusFailedSource);
}
inline QString statusSuccess() {
  return detail::translate(kContext, kStatusSuccessSource);
}
inline QString statusRunning() {
  return detail::translate(kContext, kStatusRunningSource);
}
inline QString statusPaused() {
  return detail::translate(kContext, kStatusPausedSource);
}
inline QString statusDraft() {
  return detail::translate(kContext, kStatusDraftSource);
}
inline QString statusFinalized() {
  return detail::translate(kContext, kStatusFinalizedSource);
}
inline QString statusDeleted() {
  return detail::translate(kContext, kStatusDeletedSource);
}

} // namespace importLogs

namespace importPhases {

inline constexpr auto kContext = "ImportWorkflowState";
inline constexpr auto kStoppingSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Stopping...");
inline constexpr auto kStartingSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Starting import...");
inline constexpr auto kCanceledSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Import canceled");
inline constexpr auto kFailedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Import failed");
inline constexpr auto kFinishedSource =
    QT_TRANSLATE_NOOP("ImportWorkflowState", "Import finished");

inline QString stopping() {
  return detail::translate(kContext, kStoppingSource);
}
inline QString starting() {
  return detail::translate(kContext, kStartingSource);
}
inline QString canceled() {
  return detail::translate(kContext, kCanceledSource);
}
inline QString failed() { return detail::translate(kContext, kFailedSource); }
inline QString finished() {
  return detail::translate(kContext, kFinishedSource);
}

} // namespace importPhases

namespace analysis {

inline constexpr auto kContext = "AnalysisPayloadMapper";
inline constexpr auto kUnassignedContractTypeSource =
    QT_TRANSLATE_NOOP("AnalysisPayloadMapper", "unassigned");

inline QString unassignedContractType() {
  return detail::translate(kContext, kUnassignedContractTypeSource);
}

} // namespace analysis

namespace exportRunner {

inline constexpr auto kContext = "ExportRunner";
inline constexpr auto kRunnerUnavailableSource =
    QT_TRANSLATE_NOOP("ExportRunner", "Export runner is not configured");
inline constexpr auto kStateSnapshotUnavailableSource =
    QT_TRANSLATE_NOOP("ExportRunner", "Export state snapshot is null");

inline QString runnerUnavailable() {
  return detail::translate(kContext, kRunnerUnavailableSource);
}
inline QString stateSnapshotUnavailable() {
  return detail::translate(kContext, kStateSnapshotUnavailableSource);
}

} // namespace exportRunner

namespace exportRuns {

inline constexpr auto kContext = "ExportWorkflowState";
inline constexpr auto kSuccessDetailSource =
    QT_TRANSLATE_NOOP("ExportWorkflowState", "Export completed successfully.");
inline constexpr auto kStartingDetailSource =
    QT_TRANSLATE_NOOP("ExportWorkflowState", "Starting export...");

inline QString successDetail() {
  return detail::translate(kContext, kSuccessDetailSource);
}
inline QString startingDetail() {
  return detail::translate(kContext, kStartingDetailSource);
}

} // namespace exportRuns

namespace language {

inline constexpr auto kContext = "LanguageService";
inline constexpr auto kEnglishLabelSource =
    QT_TRANSLATE_NOOP("LanguageService", "English");
inline constexpr auto kGermanLabelSource =
    QT_TRANSLATE_NOOP("LanguageService", "Deutsch");

inline QString englishLabel() {
  return detail::translate(kContext, kEnglishLabelSource);
}
inline QString germanLabel() {
  return detail::translate(kContext, kGermanLabelSource);
}

} // namespace language

} // namespace ui::text
