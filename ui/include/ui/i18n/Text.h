/**
 * @file ui/include/ui/i18n/Text.h
 * @brief Provides translated C++ UI strings for shell, workflow, adapter and
 * view model code.
 *
 * QML strings are translated in QML. This header covers strings emitted from
 * C++ UI code, while keeping the `QT_TRANSLATE_NOOP` source literals visible to
 * Qt Linguist.
 */

#pragma once

#include <QCoreApplication>
#include <QString>

namespace ui::text {

namespace detail {

/**
 * @brief Translate a source string in the given Qt translation context.
 * @param context Qt Linguist context name.
 * @param source Source string registered through `QT_TRANSLATE_NOOP`.
 * @return Localized string for the active application translator.
 */
inline QString translate(const char* context, const char* source) {
  return QCoreApplication::translate(context, source);
}

} // namespace detail

/**
 * @brief General status text shown by the shell.
 */
namespace status {

inline constexpr auto kContext = "Messages";
inline constexpr auto kReadySource = QT_TRANSLATE_NOOP("Messages", "Ready");

/** @brief Text for the idle application state. */
inline QString ready() {
  return detail::translate(kContext, kReadySource);
}

} // namespace status

/**
 * @brief QAction labels used by the desktop shell.
 */
namespace actions {

inline constexpr auto kContext = "Actions";
inline constexpr auto kNewFileSource = QT_TRANSLATE_NOOP("Actions", "New...");
inline constexpr auto kOpenFileSource = QT_TRANSLATE_NOOP("Actions", "Open...");
inline constexpr auto kSaveFileSource = QT_TRANSLATE_NOOP("Actions", "Save");
inline constexpr auto kSaveFileAsSource =
    QT_TRANSLATE_NOOP("Actions", "Save As...");
inline constexpr auto kQuitSource = QT_TRANSLATE_NOOP("Actions", "Quit");
inline constexpr auto kAboutSource = QT_TRANSLATE_NOOP("Actions", "About");

/** @brief Label for creating a new workspace file. */
inline QString newFile() {
  return detail::translate(kContext, kNewFileSource);
}
/** @brief Label for opening an existing workspace file. */
inline QString openFile() {
  return detail::translate(kContext, kOpenFileSource);
}
/** @brief Label for saving the current workspace file. */
inline QString saveFile() {
  return detail::translate(kContext, kSaveFileSource);
}
/** @brief Label for saving the current workspace under another path. */
inline QString saveFileAs() {
  return detail::translate(kContext, kSaveFileAsSource);
}
/** @brief Label for quitting the application. */
inline QString quit() {
  return detail::translate(kContext, kQuitSource);
}
/** @brief Label for opening the about dialog. */
inline QString about() {
  return detail::translate(kContext, kAboutSource);
}

} // namespace actions

/**
 * @brief File dialog titles and filter labels.
 */
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

/** @brief Title for PDF import file selection. */
inline QString selectPdfTitle() {
  return detail::translate(kContext, kSelectPdfTitleSource);
}
/** @brief Title for selecting an export target file. */
inline QString exportFileTitle() {
  return detail::translate(kContext, kExportFileTitleSource);
}
/** @brief Title for selecting an export target directory. */
inline QString exportDirectoryTitle() {
  return detail::translate(kContext, kExportDirectoryTitleSource);
}
/** @brief Title for creating a workspace database file. */
inline QString newFileTitle() {
  return detail::translate(kContext, kNewFileTitleSource);
}
/** @brief Title for opening a workspace database file. */
inline QString openFileTitle() {
  return detail::translate(kContext, kOpenFileTitleSource);
}
/** @brief Title for saving a workspace database file under another path. */
inline QString saveFileAsTitle() {
  return detail::translate(kContext, kSaveFileAsTitleSource);
}
/** @brief File filter for workspace database files. */
inline QString databaseFilter() {
  return detail::translate(kContext, kDatabaseFilterSource);
}
/** @brief File filter for PDF import sources. */
inline QString importPdfFilter() {
  return detail::translate(kContext, kImportPdfFilterSource);
}
/** @brief File filter for direct export targets. */
inline QString exportFileFilter() {
  return detail::translate(kContext, kExportFileFilterSource);
}

} // namespace dialogs

/**
 * @brief Text used by the QWidget host window.
 */
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

/** @brief Status pattern for a selected file path. */
inline QString selectedStatusPattern() {
  return detail::translate(kContext, kSelectedStatusPatternSource);
}
/** @brief Status pattern for an export path selected by the user. */
inline QString exportPathStatusPattern() {
  return detail::translate(kContext, kExportPathStatusPatternSource);
}
/** @brief About dialog window title. */
inline QString aboutTitle() {
  return detail::translate(kContext, kAboutTitleSource);
}
/** @brief About dialog headline. */
inline QString aboutHeadline() {
  return detail::translate(kContext, kAboutHeadlineSource);
}
/** @brief About dialog body copy. */
inline QString aboutBody() {
  return detail::translate(kContext, kAboutBodySource);
}

} // namespace mainWindow

/**
 * @brief User-facing errors emitted by UI workflows.
 */
namespace workflowErrors {

inline constexpr auto kContext = "WorkflowErrors";
inline constexpr auto kStorageSaveFailedSource =
    QT_TRANSLATE_NOOP("WorkflowErrors", "Failed to save file");
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

/** @brief Error text for failed storage save operations. */
inline QString storageSaveFailed() {
  return detail::translate(kContext, kStorageSaveFailedSource);
}
/** @brief Generic export failure text. */
inline QString exportFailed() {
  return detail::translate(kContext, kExportFailedSource);
}
/** @brief Error text when export state cannot be read. */
inline QString exportStateUnavailable() {
  return detail::translate(kContext, kExportStateUnavailableSource);
}
/** @brief Error text when import workflow wiring is unavailable. */
inline QString importWorkflowUnavailable() {
  return detail::translate(kContext, kImportWorkflowUnavailableSource);
}
/** @brief Error text when an operation requires a selected file. */
inline QString noFileSelected() {
  return detail::translate(kContext, kNoFileSelectedSource);
}
/** @brief Generic import failure text. */
inline QString importFailed() {
  return detail::translate(kContext, kImportFailedSource);
}
/** @brief Error text when the analysis runner is unavailable. */
inline QString analysisEngineUnavailable() {
  return detail::translate(kContext, kAnalysisEngineUnavailableSource);
}
/** @brief Error text when the analysis view model state is unavailable. */
inline QString analysisViewModelUnavailable() {
  return detail::translate(kContext, kAnalysisStateUnavailableSource);
}

} // namespace workflowErrors

/**
 * @brief Import workflow labels, phases and stored run log details.
 */
namespace importing {

inline constexpr auto kContext = "Import";
inline constexpr auto kTypeStatementSource =
    QT_TRANSLATE_NOOP("Import", "Statement");
inline constexpr auto kStatusCanceledSource =
    QT_TRANSLATE_NOOP("Import", "Canceled");
inline constexpr auto kStatusFailedSource =
    QT_TRANSLATE_NOOP("Import", "Failed");
inline constexpr auto kStatusSuccessSource =
    QT_TRANSLATE_NOOP("Import", "Success");
inline constexpr auto kStatusRunningSource =
    QT_TRANSLATE_NOOP("Import", "Running");
inline constexpr auto kStatusPausedSource =
    QT_TRANSLATE_NOOP("Import", "Paused");
inline constexpr auto kStatusDraftSource = QT_TRANSLATE_NOOP("Import", "Draft");
inline constexpr auto kStatusFinalizedSource =
    QT_TRANSLATE_NOOP("Import", "Finalized");
inline constexpr auto kStatusDeletedSource =
    QT_TRANSLATE_NOOP("Import", "Deleted");
inline constexpr auto kStatusDraftDiscardedSource =
    QT_TRANSLATE_NOOP("Import", "Draft discarded");
inline constexpr auto kStatusFinalizeFailedSource =
    QT_TRANSLATE_NOOP("Import", "Finalize failed");
inline constexpr auto kMessageDraftPausedSource =
    QT_TRANSLATE_NOOP("Import", "Draft paused. Click log entry to continue.");
inline constexpr auto kMessageDraftDiscardedSource =
    QT_TRANSLATE_NOOP("Import", "Statement draft was discarded.");
inline constexpr auto kMessageCanceledBeforeStartSource =
    QT_TRANSLATE_NOOP("Import", "Import canceled before start.");
inline constexpr auto kMessageFinalizeFailedSource =
    QT_TRANSLATE_NOOP("Import", "Draft could not be finalized.");
inline constexpr auto kMessageFinalizedSource =
    QT_TRANSLATE_NOOP("Import", "Draft was finalized into a statement.");
inline constexpr auto kMessageImportPausedSource =
    QT_TRANSLATE_NOOP("Import", "Import paused.");
inline constexpr auto kMessageImportResumedSource =
    QT_TRANSLATE_NOOP("Import", "Import resumed.");
inline constexpr auto kMessageDraftReadySource =
    QT_TRANSLATE_NOOP("Import", "Draft ready for manual review.");
inline constexpr auto kPhaseStoppingSource =
    QT_TRANSLATE_NOOP("Import", "Stopping...");
inline constexpr auto kPhaseStartingSource =
    QT_TRANSLATE_NOOP("Import", "Starting import...");
inline constexpr auto kPhaseRunningSource =
    QT_TRANSLATE_NOOP("Import", "Running import...");
inline constexpr auto kPhasePausedSource =
    QT_TRANSLATE_NOOP("Import", "Paused");
inline constexpr auto kPhaseCanceledSource =
    QT_TRANSLATE_NOOP("Import", "Import canceled");
inline constexpr auto kPhaseFailedSource =
    QT_TRANSLATE_NOOP("Import", "Import failed");
inline constexpr auto kPhaseFinishedSource =
    QT_TRANSLATE_NOOP("Import", "Import finished");

/** @brief Import log object type for statement imports. */
inline QString typeStatement() {
  return detail::translate(kContext, kTypeStatementSource);
}
/** @brief Import log status for canceled runs. */
inline QString statusCanceled() {
  return detail::translate(kContext, kStatusCanceledSource);
}
/** @brief Import log status for failed runs. */
inline QString statusFailed() {
  return detail::translate(kContext, kStatusFailedSource);
}
/** @brief Import log status for successful runs. */
inline QString statusSuccess() {
  return detail::translate(kContext, kStatusSuccessSource);
}
/** @brief Import log status for active runs. */
inline QString statusRunning() {
  return detail::translate(kContext, kStatusRunningSource);
}
/** @brief Import log status for paused draft review. */
inline QString statusPaused() {
  return detail::translate(kContext, kStatusPausedSource);
}
/** @brief Import log status for draft rows. */
inline QString statusDraft() {
  return detail::translate(kContext, kStatusDraftSource);
}
/** @brief Import log status for finalized drafts. */
inline QString statusFinalized() {
  return detail::translate(kContext, kStatusFinalizedSource);
}
/** @brief Import log status for deleted draft rows. */
inline QString statusDeleted() {
  return detail::translate(kContext, kStatusDeletedSource);
}
/** @brief Import log status for discarded drafts. */
inline QString statusDraftDiscarded() {
  return detail::translate(kContext, kStatusDraftDiscardedSource);
}
/** @brief Import log status for failed draft finalization. */
inline QString statusFinalizeFailed() {
  return detail::translate(kContext, kStatusFinalizeFailedSource);
}
/** @brief Import log detail shown while a draft waits for review. */
inline QString messageDraftPaused() {
  return detail::translate(kContext, kMessageDraftPausedSource);
}
/** @brief Import log detail shown after a draft is discarded. */
inline QString messageDraftDiscarded() {
  return detail::translate(kContext, kMessageDraftDiscardedSource);
}
/** @brief Import log detail for queued files canceled before execution. */
inline QString messageCanceledBeforeStart() {
  return detail::translate(kContext, kMessageCanceledBeforeStartSource);
}
/** @brief Import log detail shown when finalizing a draft fails. */
inline QString messageFinalizeFailed() {
  return detail::translate(kContext, kMessageFinalizeFailedSource);
}
/** @brief Import log detail shown after a draft becomes a statement. */
inline QString messageFinalized() {
  return detail::translate(kContext, kMessageFinalizedSource);
}
/** @brief Import log detail shown when a running import is paused. */
inline QString messageImportPaused() {
  return detail::translate(kContext, kMessageImportPausedSource);
}
/** @brief Import log detail shown when a paused import resumes. */
inline QString messageImportResumed() {
  return detail::translate(kContext, kMessageImportResumedSource);
}
/** @brief Import log detail shown when a draft is ready for review. */
inline QString messageDraftReady() {
  return detail::translate(kContext, kMessageDraftReadySource);
}
/** @brief Phase text while a cancellation request is being processed. */
inline QString phaseStopping() {
  return detail::translate(kContext, kPhaseStoppingSource);
}
/** @brief Phase text when an import has just been submitted. */
inline QString phaseStarting() {
  return detail::translate(kContext, kPhaseStartingSource);
}
/** @brief Phase text after a paused import resumes. */
inline QString phaseRunning() {
  return detail::translate(kContext, kPhaseRunningSource);
}
/** @brief Phase text while an import is paused for draft review. */
inline QString phasePaused() {
  return detail::translate(kContext, kPhasePausedSource);
}
/** @brief Phase text after an import was canceled. */
inline QString phaseCanceled() {
  return detail::translate(kContext, kPhaseCanceledSource);
}
/** @brief Phase text after an import failed. */
inline QString phaseFailed() {
  return detail::translate(kContext, kPhaseFailedSource);
}
/** @brief Phase text after an import finished. */
inline QString phaseFinished() {
  return detail::translate(kContext, kPhaseFinishedSource);
}

} // namespace importing

/**
 * @brief Analysis presentation labels.
 */
namespace analysis {

inline constexpr auto kContext = "Analysis";
inline constexpr auto kUnassignedContractTypeSource =
    QT_TRANSLATE_NOOP("Analysis", "unassigned");

/** @brief Label for transactions without an assigned contract type. */
inline QString unassignedContractType() {
  return detail::translate(kContext, kUnassignedContractTypeSource);
}

} // namespace analysis

/**
 * @brief Transaction status labels shared by C++ presentation mappers.
 */
namespace transactionStatus {

inline constexpr auto kContext = "TransactionStatus";
inline constexpr auto kNeutralSource =
    QT_TRANSLATE_NOOP("TransactionStatus", "Neutral");
inline constexpr auto kUnverifiedSource =
    QT_TRANSLATE_NOOP("TransactionStatus", "Unverified");
inline constexpr auto kVerifiedSource =
    QT_TRANSLATE_NOOP("TransactionStatus", "Verified");
inline constexpr auto kCompletedSource =
    QT_TRANSLATE_NOOP("TransactionStatus", "Completed");

/** @brief Label for neutral transaction status. */
inline QString neutral() {
  return detail::translate(kContext, kNeutralSource);
}
/** @brief Label for unverified transaction status. */
inline QString unverified() {
  return detail::translate(kContext, kUnverifiedSource);
}
/** @brief Label for verified transaction status. */
inline QString verified() {
  return detail::translate(kContext, kVerifiedSource);
}
/** @brief Label for completed transaction status. */
inline QString completed() {
  return detail::translate(kContext, kCompletedSource);
}

} // namespace transactionStatus

/**
 * @brief Export workflow diagnostics and stored run log details.
 */
namespace exporting {

inline constexpr auto kContext = "Export";
inline constexpr auto kRunnerUnavailableSource =
    QT_TRANSLATE_NOOP("Export", "Export runner is not configured");
inline constexpr auto kStateSnapshotUnavailableSource =
    QT_TRANSLATE_NOOP("Export", "Export state snapshot is null");
inline constexpr auto kSuccessDetailSource =
    QT_TRANSLATE_NOOP("Export", "Export completed successfully.");
inline constexpr auto kStartingDetailSource =
    QT_TRANSLATE_NOOP("Export", "Starting export...");
inline constexpr auto kStatusSuccessSource =
    QT_TRANSLATE_NOOP("Export", "Success");
inline constexpr auto kStatusFailedSource =
    QT_TRANSLATE_NOOP("Export", "Failed");
inline constexpr auto kStatusCanceledSource =
    QT_TRANSLATE_NOOP("Export", "Canceled");
inline constexpr auto kStatusRunningSource =
    QT_TRANSLATE_NOOP("Export", "Running");
inline constexpr auto kPhaseStartingSource =
    QT_TRANSLATE_NOOP("Export", "Starting export...");
inline constexpr auto kPhaseRunningSource =
    QT_TRANSLATE_NOOP("Export", "Running export...");
inline constexpr auto kPhaseCancelRequestedSource =
    QT_TRANSLATE_NOOP("Export", "Cancel requested...");
inline constexpr auto kPhasePausedSource =
    QT_TRANSLATE_NOOP("Export", "Paused");
inline constexpr auto kPhaseFinishedSource =
    QT_TRANSLATE_NOOP("Export", "Finished");
inline constexpr auto kPhaseFailedSource =
    QT_TRANSLATE_NOOP("Export", "Failed");
inline constexpr auto kPhaseCanceledSource =
    QT_TRANSLATE_NOOP("Export", "Canceled");

/** @brief Error text when no export runner has been wired. */
inline QString runnerUnavailable() {
  return detail::translate(kContext, kRunnerUnavailableSource);
}
/** @brief Error text when export state cannot be snapshotted. */
inline QString stateSnapshotUnavailable() {
  return detail::translate(kContext, kStateSnapshotUnavailableSource);
}
/** @brief Export log detail for successful runs. */
inline QString successDetail() {
  return detail::translate(kContext, kSuccessDetailSource);
}
/** @brief Export log detail for newly started runs. */
inline QString startingDetail() {
  return detail::translate(kContext, kStartingDetailSource);
}
/** @brief Export log status for successful runs. */
inline QString statusSuccess() {
  return detail::translate(kContext, kStatusSuccessSource);
}
/** @brief Export log status for failed runs. */
inline QString statusFailed() {
  return detail::translate(kContext, kStatusFailedSource);
}
/** @brief Export log status for canceled runs. */
inline QString statusCanceled() {
  return detail::translate(kContext, kStatusCanceledSource);
}
/** @brief Export log status for active runs. */
inline QString statusRunning() {
  return detail::translate(kContext, kStatusRunningSource);
}
/** @brief Phase text when an export has just been submitted. */
inline QString phaseStarting() {
  return detail::translate(kContext, kPhaseStartingSource);
}
/** @brief Phase text while an export is running. */
inline QString phaseRunning() {
  return detail::translate(kContext, kPhaseRunningSource);
}
/** @brief Phase text after the user requests export cancellation. */
inline QString phaseCancelRequested() {
  return detail::translate(kContext, kPhaseCancelRequestedSource);
}
/** @brief Phase text while an export is paused. */
inline QString phasePaused() {
  return detail::translate(kContext, kPhasePausedSource);
}
/** @brief Phase text after an export completed successfully. */
inline QString phaseFinished() {
  return detail::translate(kContext, kPhaseFinishedSource);
}
/** @brief Phase text after an export failed. */
inline QString phaseFailed() {
  return detail::translate(kContext, kPhaseFailedSource);
}
/** @brief Phase text after an export was canceled. */
inline QString phaseCanceled() {
  return detail::translate(kContext, kPhaseCanceledSource);
}

} // namespace exporting

/**
 * @brief Language names shown by the language selector.
 */
namespace language {

inline constexpr auto kContext = "Language";
inline constexpr auto kEnglishLabelSource =
    QT_TRANSLATE_NOOP("Language", "English");
inline constexpr auto kGermanLabelSource =
    QT_TRANSLATE_NOOP("Language", "Deutsch");
inline constexpr auto kFrenchLabelSource =
    QT_TRANSLATE_NOOP("Language", "Français");

/** @brief Display label for English. */
inline QString englishLabel() {
  return detail::translate(kContext, kEnglishLabelSource);
}
/** @brief Display label for German. */
inline QString germanLabel() {
  return detail::translate(kContext, kGermanLabelSource);
}
/** @brief Display label for French. */
inline QString frenchLabel() {
  return detail::translate(kContext, kFrenchLabelSource);
}

} // namespace language

} // namespace ui::text
