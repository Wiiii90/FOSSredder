/**
 * @file core/src/application/workspace/WorkspaceFacade.cpp
 * @brief Implements the workspace facade for snapshot reads and command writes.
 */

#include "core/pch.h"
#include "core/application/workspace/WorkspaceFacade.h"

#include "core/application/workspace/WorkspaceCommandService.h"
#include "core/application/workspace/WorkspaceQueryService.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "core/application/workspace/WorkspaceWorkflowService.h"

namespace core::application {

namespace {

core::ports::workspace::DeletionImpact toWorkspaceDeletionImpact(
    const core::domain::DeletionImpact& impact) {
    return {
        impact.deletedActorIds,
        impact.deletedPropertyIds,
        impact.deletedContractIds,
        impact.deletedStatementIds,
        impact.deletedTransactionIds,
        impact.deletedAnalysisIds,
        impact.deletedAnnualIds,
        impact.deletedStatementDraftIds,
        impact.deletedTransactionDraftIds,
        impact.deletedImportLogIds,
        impact.deletedExportLogIds
    };
}

} // namespace

WorkspaceFacade::WorkspaceFacade(
    std::unique_ptr<core::ports::storage::IStorageManager> storageManager)
    : session_(std::make_unique<WorkspaceSession>(std::move(storageManager))),
      commands_(std::make_unique<WorkspaceCommandService>(*session_)),
      workflows_(std::make_unique<WorkspaceWorkflowService>(*session_)),
      queries_(std::make_unique<WorkspaceQueryService>(*session_))
{
}

WorkspaceFacade::~WorkspaceFacade() = default;

WorkspaceFacade::WorkspaceFacade(WorkspaceFacade&&) noexcept = default;

WorkspaceFacade& WorkspaceFacade::operator=(WorkspaceFacade&&) noexcept = default;

void WorkspaceFacade::setSnapshotChangedCallback(SnapshotChanged cb) {
    onSnapshotChanged_ = std::move(cb);
    installStateChangedDispatcher();
}

void WorkspaceFacade::setStateChangedCallback(StateChanged cb) {
    onStateChanged_ = std::move(cb);
    installStateChangedDispatcher();
}

void WorkspaceFacade::installStateChangedDispatcher() {
    session_->setStateChangedCallback(
        [this](const core::application::workspace::WorkspaceSessionState& state) {
            if (onSnapshotChanged_) {
                onSnapshotChanged_(workspaceSnapshot());
            }
            if (onStateChanged_) {
                onStateChanged_(state);
            }
        });
}

void WorkspaceFacade::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter) {
    session_->setErrorReporter(std::move(reporter));
}

void WorkspaceFacade::setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn) {
    session_->setAtomicStoreSave(std::move(saveFn));
}

void WorkspaceFacade::setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn) {
    session_->setAtomicStoreLoad(std::move(loadFn));
}

void WorkspaceFacade::setDeletionImpactCallback(
    core::ports::workspace::IWorkspaceWriter::DeletionImpactCallback cb) {
    session_->setDeletionImpactCallback(
        [callback = std::move(cb)](const core::domain::DeletionImpact& impact) {
            if (callback) {
                callback(toWorkspaceDeletionImpact(impact));
            }
        });
}

core::ports::workspace::WorkspaceSnapshot WorkspaceFacade::workspaceSnapshot() const {
    return queries_->workspaceSnapshot();
}

std::optional<core::ports::workspace::StatementDraftSnapshot>
WorkspaceFacade::statementDraftSnapshot(const std::string& draftId) const {
    return queries_->statementDraftSnapshot(draftId);
}

void WorkspaceFacade::openLatest() {
    session_->openLatest();
}

void WorkspaceFacade::newFile(const std::string& path) {
    session_->newFile(path);
}

void WorkspaceFacade::openFile(const std::string& path) {
    session_->openFile(path);
}

void WorkspaceFacade::saveFile() {
    session_->saveFile();
}

void WorkspaceFacade::saveFileAs(const std::string& path) {
    session_->saveFileAs(path);
}

void WorkspaceFacade::commit() {
    session_->commit();
}

void WorkspaceFacade::notifySnapshot() {
    session_->notifyState();
}

std::string WorkspaceFacade::addActor(const core::ports::workspace::ActorCommand& command) {
    return commands_->addActor(command);
}

void WorkspaceFacade::updateActor(const core::ports::workspace::ActorCommand& command) {
    commands_->updateActor(command);
}

void WorkspaceFacade::deleteActor(const std::string& id) {
    commands_->deleteActor(id);
}

std::string WorkspaceFacade::addProperty(const core::ports::workspace::PropertyCommand& command) {
    return commands_->addProperty(command);
}

void WorkspaceFacade::updateProperty(const core::ports::workspace::PropertyCommand& command) {
    commands_->updateProperty(command);
}

void WorkspaceFacade::deleteProperty(const std::string& id) {
    commands_->deleteProperty(id);
}

void WorkspaceFacade::deleteContract(const std::string& id) {
    commands_->deleteContract(id);
}

std::string WorkspaceFacade::addContract(const core::ports::workspace::ContractCommand& command) {
    return commands_->addContract(command);
}

void WorkspaceFacade::updateContract(const core::ports::workspace::ContractCommand& command) {
    commands_->updateContract(command);
}

std::string WorkspaceFacade::addStatement(const core::ports::workspace::StatementCommand& command) {
    return commands_->addStatement(command);
}

void WorkspaceFacade::updateStatement(const core::ports::workspace::StatementCommand& command) {
    commands_->updateStatement(command);
}

void WorkspaceFacade::deleteStatement(const std::string& id) {
    commands_->deleteStatement(id);
}

std::string WorkspaceFacade::addTransaction(const core::ports::workspace::TransactionCommand& command) {
    return commands_->addTransaction(command);
}

void WorkspaceFacade::updateTransaction(const core::ports::workspace::TransactionCommand& command) {
    commands_->updateTransaction(command);
}

void WorkspaceFacade::deleteTransaction(const std::string& id) {
    commands_->deleteTransaction(id);
}

std::string WorkspaceFacade::addAnalysis(const core::ports::workspace::AnalysisCommand& command) {
    return commands_->addAnalysis(command);
}

void WorkspaceFacade::updateAnalysis(const core::ports::workspace::AnalysisCommand& command) {
    commands_->updateAnalysis(command);
}

void WorkspaceFacade::deleteAnalysis(const std::string& id) {
    commands_->deleteAnalysis(id);
}

std::string WorkspaceFacade::addAnnual(const core::ports::workspace::AnnualCommand& command) {
    return commands_->addAnnual(command);
}

void WorkspaceFacade::updateAnnual(const core::ports::workspace::AnnualCommand& command) {
    commands_->updateAnnual(command);
}

void WorkspaceFacade::deleteAnnual(const std::string& id) {
    commands_->deleteAnnual(id);
}

std::string WorkspaceFacade::finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand& command) {
    return workflows_->finalizeStatementDraft(command);
}

void WorkspaceFacade::saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command) {
    workflows_->saveStatementDraft(command);
}

void WorkspaceFacade::clearStatementDraft(const std::string& draftId) {
    workflows_->clearStatementDraft(draftId);
}

void WorkspaceFacade::setImportLogs(const core::ports::workspace::ImportLogsCommand& command) {
    workflows_->setImportLogs(command);
}

void WorkspaceFacade::saveImportLog(const core::ports::workspace::ImportLogCommand& command) {
    workflows_->saveImportLog(command);
}

void WorkspaceFacade::deleteImportLog(const std::string& id) {
    workflows_->deleteImportLog(id);
}

void WorkspaceFacade::clearImportLogs() {
    workflows_->clearImportLogs();
}

void WorkspaceFacade::setExportLogs(const core::ports::workspace::ExportLogsCommand& command) {
    workflows_->setExportLogs(command);
}

void WorkspaceFacade::saveExportLog(const core::ports::workspace::ExportLogCommand& command) {
    workflows_->saveExportLog(command);
}

void WorkspaceFacade::deleteExportLog(const std::string& id) {
    workflows_->deleteExportLog(id);
}

void WorkspaceFacade::clearExportLogs() {
    workflows_->clearExportLogs();
}

const core::application::workspace::WorkspaceSessionState& WorkspaceFacade::state() const noexcept {
    return session_->state();
}

const core::domain::catalog::WorkspaceCatalog& WorkspaceFacade::catalogState() const noexcept {
    return session_->catalogState();
}

std::string WorkspaceFacade::currentPath() const {
    return queries_->currentPath();
}

} // namespace core::application
