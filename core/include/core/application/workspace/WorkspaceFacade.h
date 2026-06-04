/**
 * @file core/include/core/application/workspace/WorkspaceFacade.h
 * @brief Declares the workspace facade implementing snapshot reader and command writer ports.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/errors/IErrorReporter.h"
#include "core/ports/infra/storage/IStorageManager.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

namespace core::application {

class WorkspaceSession;
class WorkspaceCommandService;
class WorkspaceWorkflowService;
class WorkspaceQueryService;

/**
 * @brief Exposes state mutation and workspace persistence as a single application facade.
 *
 * This type is consumed by presentation-layer controllers in `ui`, while the actual
 * MVC-style UI controllers remain outside `core`.
 */
class WorkspaceFacade : public core::ports::workspace::IWorkspaceReader,
                        public core::ports::workspace::IWorkspaceWriter {
public:
    using SnapshotChanged = core::ports::workspace::IWorkspaceWriter::SnapshotChanged;
    using StateChanged = std::function<void(const core::application::workspace::WorkspaceSessionState&)>;

    /**
     * @brief Construct with unique ownership of an `IStorageManager`.
     * @param storageManager Unique pointer to the storage manager implementation.
     */
    explicit WorkspaceFacade(std::unique_ptr<core::ports::storage::IStorageManager> storageManager);
    ~WorkspaceFacade();

    WorkspaceFacade(const WorkspaceFacade&) = delete;
    WorkspaceFacade& operator=(const WorkspaceFacade&) = delete;
    WorkspaceFacade(WorkspaceFacade&&) noexcept;
    WorkspaceFacade& operator=(WorkspaceFacade&&) noexcept;

    /**
     * @brief Registers a snapshot callback used by read-side consumers.
     * @param cb Callback invoked after state changes publish a fresh snapshot.
     */
    void setSnapshotChangedCallback(SnapshotChanged cb) override;

    /**
     * @brief Registers a callback that receives the mutable session state view.
     * @param cb Callback invoked after state changes.
     */
    void setStateChangedCallback(StateChanged cb);

    /**
     * @brief Registers the error reporter used by workspace operations.
     * @param reporter Shared error reporter implementation.
     */
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter) override;

    /**
     * @brief Registers the atomic save callback delegated to the storage manager.
     * @param saveFn Save callback implementation.
     */
    void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn) override;

    /**
     * @brief Registers the atomic load callback delegated to the storage manager.
     * @param loadFn Load callback implementation.
     */
    void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn) override;

    /**
     * @brief Registers a callback that receives deletion impact information after saves.
     * @param cb Deletion impact callback implementation.
     */
    void setDeletionImpactCallback(core::ports::workspace::IWorkspaceWriter::DeletionImpactCallback cb) override;

    /** @brief Returns the current immutable workspace snapshot. */
    core::ports::workspace::WorkspaceSnapshot workspaceSnapshot() const override;

    /**
     * @brief Returns one statement draft snapshot.
     * @param draftId Optional statement draft identifier.
     * @return Matching draft snapshot when present.
     */
    std::optional<core::ports::workspace::StatementDraftSnapshot> statementDraftSnapshot(const std::string& draftId = {}) const override;
    core::ports::workspace::WorkspaceIdentitySnapshot actorIdentityByName(const std::string& name) const override;
    core::ports::workspace::WorkspaceIdentitySnapshot propertyIdentityByName(const std::string& name) const override;
    core::ports::workspace::WorkspaceIdentitySnapshot contractIdentityBySignature(
        const std::string& name,
        const std::string& type,
        const std::vector<std::string>& actorIds,
        const std::vector<std::string>& propertyIds) const override;
    std::string nextContractName() const override;

    void openLatest() override;
    void newFile(const std::string& path) override;
    void openFile(const std::string& path) override;
    void saveFile() override;
    void saveFileAs(const std::string& path) override;
    void commit() override;
    void notifySnapshot() override;

    core::ports::workspace::ValidationResult validateActor(const core::ports::workspace::ActorCommand& command) const override;
    core::ports::workspace::ValidationResult validateProperty(const core::ports::workspace::PropertyCommand& command) const override;
    core::ports::workspace::ValidationResult validateContract(const core::ports::workspace::ContractCommand& command) const override;
    core::ports::workspace::ValidationResult validateStatement(const core::ports::workspace::StatementCommand& command) const override;
    core::ports::workspace::ValidationResult validateTransaction(const core::ports::workspace::TransactionCommand& command) const override;
    core::ports::workspace::ValidationResult validateAnalysis(const core::ports::workspace::AnalysisCommand& command) const override;
    core::ports::workspace::ValidationResult validateAnnual(const core::ports::workspace::AnnualCommand& command) const override;

    std::string addActor(const core::ports::workspace::ActorCommand& command) override;
    void updateActor(const core::ports::workspace::ActorCommand& command) override;
    void deleteActor(const std::string& id) override;

    std::string addProperty(const core::ports::workspace::PropertyCommand& command) override;
    void updateProperty(const core::ports::workspace::PropertyCommand& command) override;
    void deleteProperty(const std::string& id) override;

    std::string addContract(const core::ports::workspace::ContractCommand& command) override;
    void updateContract(const core::ports::workspace::ContractCommand& command) override;
    void deleteContract(const std::string& id) override;

    std::string addStatement(const core::ports::workspace::StatementCommand& command) override;
    void updateStatement(const core::ports::workspace::StatementCommand& command) override;
    void deleteStatement(const std::string& id) override;

    std::string addTransaction(const core::ports::workspace::TransactionCommand& command) override;
    void updateTransaction(const core::ports::workspace::TransactionCommand& command) override;
    void deleteTransaction(const std::string& id) override;

    std::string addAnalysis(const core::ports::workspace::AnalysisCommand& command) override;
    void updateAnalysis(const core::ports::workspace::AnalysisCommand& command) override;
    void deleteAnalysis(const std::string& id) override;

    std::string addAnnual(const core::ports::workspace::AnnualCommand& command) override;
    void updateAnnual(const core::ports::workspace::AnnualCommand& command) override;
    void deleteAnnual(const std::string& id) override;

    std::string finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand& command) override;
    void saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command) override;
    void clearStatementDraft(const std::string& draftId = {}) override;
    void setImportLogs(const core::ports::workspace::ImportLogsCommand& command) override;
    void saveImportLog(const core::ports::workspace::ImportLogCommand& command) override;
    void deleteImportLog(const std::string& id) override;
    void clearImportLogs() override;
    void setExportLogs(const core::ports::workspace::ExportLogsCommand& command) override;
    void saveExportLog(const core::ports::workspace::ExportLogCommand& command) override;
    void deleteExportLog(const std::string& id) override;
    void clearExportLogs() override;

    /** @brief Returns the full mutable session state used by the application layer. */
    const core::application::workspace::WorkspaceSessionState& state() const noexcept;
    /** @brief Returns the current workspace catalog aggregate. */
    const core::domain::catalog::WorkspaceCatalog& catalogState() const noexcept;
    /** @brief Returns the current workspace path tracked by the storage layer. */
    std::string currentPath() const override;

private:
    void installStateChangedDispatcher();

    SnapshotChanged onSnapshotChanged_;
    StateChanged onStateChanged_;
    std::unique_ptr<WorkspaceSession> session_;
    std::unique_ptr<WorkspaceCommandService> commands_;
    std::unique_ptr<WorkspaceWorkflowService> workflows_;
    std::unique_ptr<WorkspaceQueryService> queries_;
};

} // namespace core::application
