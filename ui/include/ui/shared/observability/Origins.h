/**
 * @file ui/include/ui/shared/observability/Origins.h
 * @brief Declarations for the UI Origins component.
 */

#pragma once

namespace ui::observability::origins {

namespace app {
inline constexpr auto kQmlWarnings = "app::qml::warnings";
inline constexpr auto kToCoreExportFormat = "app::toCoreExportFormat";
} // namespace app

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
} // namespace analysis
namespace exportFlow {
inline constexpr auto kStart = "ui::ExportWorkflow::exportData";
inline constexpr auto kFinish = "ui::ExportWorkflow::onExportFinished";
} // namespace exportFlow
} // namespace workflow

namespace workspace {
namespace actor {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addActor";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateActor";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteActor";
} // namespace actor
namespace contract {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addContract";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateContract";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteContract";
} // namespace contract
namespace property {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addProperty";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateProperty";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteProperty";
} // namespace property
namespace storage {
inline constexpr auto kNewFile = "ui::WorkspaceFacade::newFile";
inline constexpr auto kOpenFile = "ui::WorkspaceFacade::openFile";
inline constexpr auto kSaveFile = "ui::WorkspaceFacade::saveFile";
inline constexpr auto kSaveFileAs = "ui::WorkspaceFacade::saveFileAs";
} // namespace storage
namespace statement {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addStatement";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateStatement";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteStatement";
} // namespace statement
namespace transaction {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addTransaction";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateTransaction";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteTransaction";
} // namespace transaction
} // namespace workspace

namespace workflow {
namespace import {
inline constexpr auto kFinalize = "ui::ImportWorkflow::finalizeStatementDraft";
inline constexpr auto kStart = "ui::ImportWorkflow::startStatementImport";
inline constexpr auto kCancel = "ui::ImportWorkflow::cancelImport";
inline constexpr auto kCancelAll = "ui::ImportWorkflow::cancelAllImports";
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
