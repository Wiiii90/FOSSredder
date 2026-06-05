/**
 * @file ui/include/ui/observability/Origins.h
 * @brief Declarations for the UI Origins component.
 */

#pragma once

namespace ui::observability::origins {

namespace app {
inline constexpr auto kToCoreExportFormat = "app::toCoreExportFormat";
} // namespace app

namespace qml {
inline constexpr auto kLoad = "ui::QmlDiagnostics::load";
inline constexpr auto kWarnings = "ui::QmlDiagnostics::warnings";
} // namespace qml

namespace mainWindow {
inline constexpr auto kActionRouting = "ui::MainWindow::setupActionRouting";
inline constexpr auto kLoadQml = "ui::MainWindow::loadQml";
inline constexpr auto kDragDrop = "ui::MainWindow::eventFilter";
inline constexpr auto kClose = "ui::MainWindow::closeEvent";
inline constexpr auto kCloseSucceeded =
    "ui::MainWindow::handleStorageOperationSucceeded";
inline constexpr auto kCloseFailed =
    "ui::MainWindow::handleStorageOperationFailed";
} // namespace mainWindow

namespace workflow {
namespace analysis {
inline constexpr auto kCompute = "ui::AnalysisWorkflow::computeAnalysis";
inline constexpr auto kPreview = "ui::AnalysisWorkflow::previewTransactions";
} // namespace analysis
namespace annual {
inline constexpr auto kCompute = "ui::AnnualWorkflow::computeAnnual";
inline constexpr auto kPreview = "ui::AnnualWorkflow::computeAnnualPreview";
} // namespace annual
namespace exportFlow {
inline constexpr auto kStart = "ui::ExportWorkflow::exportData";
inline constexpr auto kFinish = "ui::ExportWorkflow::onExportFinished";
} // namespace exportFlow
} // namespace workflow

namespace workspace {
namespace actor {
inline constexpr auto kAdd = "ui::WorkspaceCommands::addActor";
inline constexpr auto kUpdate = "ui::WorkspaceCommands::updateActor";
inline constexpr auto kDelete = "ui::WorkspaceCommands::deleteActor";
} // namespace actor
namespace contract {
inline constexpr auto kAdd = "ui::WorkspaceCommands::addContract";
inline constexpr auto kUpdate = "ui::WorkspaceCommands::updateContract";
inline constexpr auto kDelete = "ui::WorkspaceCommands::deleteContract";
} // namespace contract
namespace property {
inline constexpr auto kAdd = "ui::WorkspaceCommands::addProperty";
inline constexpr auto kUpdate = "ui::WorkspaceCommands::updateProperty";
inline constexpr auto kDelete = "ui::WorkspaceCommands::deleteProperty";
} // namespace property
namespace storage {
inline constexpr auto kNewFile = "ui::WorkspaceCommands::newFile";
inline constexpr auto kOpenFile = "ui::WorkspaceCommands::openFile";
inline constexpr auto kSaveFile = "ui::WorkspaceCommands::saveFile";
inline constexpr auto kSaveFileAs = "ui::WorkspaceCommands::saveFileAs";
} // namespace storage
namespace statement {
inline constexpr auto kAdd = "ui::WorkspaceCommands::addStatement";
inline constexpr auto kUpdate = "ui::WorkspaceCommands::updateStatement";
inline constexpr auto kDelete = "ui::WorkspaceCommands::deleteStatement";
} // namespace statement
namespace transaction {
inline constexpr auto kAdd = "ui::WorkspaceCommands::addTransaction";
inline constexpr auto kUpdate = "ui::WorkspaceCommands::updateTransaction";
inline constexpr auto kDelete = "ui::WorkspaceCommands::deleteTransaction";
} // namespace transaction
} // namespace workspace

namespace workflow {
namespace import {
inline constexpr auto kFinalize = "ui::ImportWorkflow::finalizeStatementDraft";
inline constexpr auto kStart = "ui::ImportWorkflow::startStatementImport";
inline constexpr auto kCancel = "ui::ImportWorkflow::cancelImport";
inline constexpr auto kCancelQueued = "ui::ImportWorkflow::cancelQueuedImports";
inline constexpr auto kTerminal = "ui::ImportWorkflow::onJobTerminal";
} // namespace import
} // namespace workflow

namespace service {
namespace exportRunner {
inline constexpr auto kRun = "ui::adapters::ExportAdapter::runExport";
}
namespace importJobBridge {
inline constexpr auto kStartImport =
    "core::ports::importing::IImportRunner::startStatementImport";
inline constexpr auto kClearSubscription =
    "core::ports::importing::IImportRunner::unsubscribe";
} // namespace importJobBridge
} // namespace service

} // namespace ui::observability::origins
