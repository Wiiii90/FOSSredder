/**
 * @file core/include/core/application/workspace/WorkspaceFacade.h
 * @brief Declares the workspace facade implementing snapshot reader and command writer ports.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

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
    /**
     * @brief Looks up an actor identity by name.
     * @param name Actor name.
     * @return Matching identity snapshot or an empty identity.
     */
    core::ports::workspace::WorkspaceIdentitySnapshot actorIdentityByName(const std::string& name) const override;
    /**
     * @brief Looks up a property identity by name.
     * @param name Property name.
     * @return Matching identity snapshot or an empty identity.
     */
    core::ports::workspace::WorkspaceIdentitySnapshot propertyIdentityByName(const std::string& name) const override;
    /**
     * @brief Looks up a contract identity by signature.
     * @param name Contract name.
     * @param type Contract type.
     * @param actorIds Linked actor ids.
     * @param propertyIds Linked property ids.
     * @return Matching identity snapshot or an empty identity.
     */
    core::ports::workspace::WorkspaceIdentitySnapshot contractIdentityBySignature(
        const std::string& name,
        const std::string& type,
        const std::vector<std::string>& actorIds,
        const std::vector<std::string>& propertyIds) const override;
    /**
     * @brief Applies catalog-dependent transaction selection rules.
     * @param change Transaction catalog selection change.
     * @return Resulting catalog selection.
     */
    core::ports::workspace::TransactionCatalogSelection transactionCatalogSelection(
        const core::ports::workspace::TransactionCatalogSelectionChange& change) const override;
    /**
     * @brief Returns the next generated contract name.
     * @return Generated contract name.
     */
    std::string nextContractName() const override;

    /**
     * @brief Opens the latest workspace file.
     */
    void openLatest() override;
    /**
     * @brief Creates a new workspace file.
     * @param path Target file path.
     */
    void newFile(const std::string& path) override;
    /**
     * @brief Opens a workspace file.
     * @param path Source file path.
     */
    void openFile(const std::string& path) override;
    /**
     * @brief Saves the current workspace file.
     */
    void saveFile() override;
    /**
     * @brief Saves the current workspace file to another path.
     * @param path Target file path.
     */
    void saveFileAs(const std::string& path) override;
    /**
     * @brief Commits pending workspace storage changes.
     */
    void commit() override;
    /**
     * @brief Publishes a fresh workspace snapshot.
     */
    void notifySnapshot() override;

    /**
     * @brief Validates an actor command.
     * @param command Actor command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateActor(const core::ports::workspace::ActorCommand& command) const override;
    /**
     * @brief Validates a property command.
     * @param command Property command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateProperty(const core::ports::workspace::PropertyCommand& command) const override;
    /**
     * @brief Validates a contract command.
     * @param command Contract command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateContract(const core::ports::workspace::ContractCommand& command) const override;
    /**
     * @brief Validates a statement command.
     * @param command Statement command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateStatement(const core::ports::workspace::StatementCommand& command) const override;
    /**
     * @brief Validates a transaction command.
     * @param command Transaction command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateTransaction(const core::ports::workspace::TransactionCommand& command) const override;
    /**
     * @brief Validates a statement batch command.
     * @param command Statement and transaction command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateStatementWithTransactions(const core::ports::workspace::StatementWithTransactionsCommand& command) const override;
    /**
     * @brief Validates an analysis command.
     * @param command Analysis command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateAnalysis(const core::ports::workspace::AnalysisCommand& command) const override;
    /**
     * @brief Validates an annual command.
     * @param command Annual command.
     * @return Validation result.
     */
    core::ports::workspace::ValidationResult validateAnnual(const core::ports::workspace::AnnualCommand& command) const override;

    /**
     * @brief Adds an actor.
     * @param command Actor command.
     * @return Created actor id.
     */
    std::string addActor(const core::ports::workspace::ActorCommand& command) override;
    /**
     * @brief Updates an actor.
     * @param command Actor command.
     */
    void updateActor(const core::ports::workspace::ActorCommand& command) override;
    /**
     * @brief Deletes an actor.
     * @param id Actor id.
     */
    void deleteActor(const std::string& id) override;

    /**
     * @brief Adds a property.
     * @param command Property command.
     * @return Created property id.
     */
    std::string addProperty(const core::ports::workspace::PropertyCommand& command) override;
    /**
     * @brief Updates a property.
     * @param command Property command.
     */
    void updateProperty(const core::ports::workspace::PropertyCommand& command) override;
    /**
     * @brief Deletes a property.
     * @param id Property id.
     */
    void deleteProperty(const std::string& id) override;

    /**
     * @brief Adds a contract.
     * @param command Contract command.
     * @return Created contract id.
     */
    std::string addContract(const core::ports::workspace::ContractCommand& command) override;
    /**
     * @brief Updates a contract.
     * @param command Contract command.
     */
    void updateContract(const core::ports::workspace::ContractCommand& command) override;
    /**
     * @brief Deletes a contract.
     * @param id Contract id.
     */
    void deleteContract(const std::string& id) override;

    /**
     * @brief Adds a statement.
     * @param command Statement command.
     * @return Created statement id.
     */
    std::string addStatement(const core::ports::workspace::StatementCommand& command) override;
    /**
     * @brief Adds a statement and its transactions.
     * @param command Statement creation command with transaction commands.
     * @return Created statement id.
     */
    std::string addStatementWithTransactions(const core::ports::workspace::StatementWithTransactionsCommand& command) override;
    /**
     * @brief Updates a statement.
     * @param command Statement command.
     */
    void updateStatement(const core::ports::workspace::StatementCommand& command) override;
    /**
     * @brief Deletes a statement.
     * @param id Statement id.
     */
    void deleteStatement(const std::string& id) override;

    /**
     * @brief Adds a transaction.
     * @param command Transaction command.
     * @return Created transaction id.
     */
    std::string addTransaction(const core::ports::workspace::TransactionCommand& command) override;
    /**
     * @brief Updates a transaction.
     * @param command Transaction command.
     */
    void updateTransaction(const core::ports::workspace::TransactionCommand& command) override;
    /**
     * @brief Deletes a transaction.
     * @param id Transaction id.
     */
    void deleteTransaction(const std::string& id) override;

    /**
     * @brief Adds an analysis.
     * @param command Analysis command.
     * @return Created analysis id.
     */
    std::string addAnalysis(const core::ports::workspace::AnalysisCommand& command) override;
    /**
     * @brief Updates an analysis.
     * @param command Analysis command.
     */
    void updateAnalysis(const core::ports::workspace::AnalysisCommand& command) override;
    /**
     * @brief Deletes an analysis.
     * @param id Analysis id.
     */
    void deleteAnalysis(const std::string& id) override;

    /**
     * @brief Adds an annual.
     * @param command Annual command.
     * @return Created annual id.
     */
    std::string addAnnual(const core::ports::workspace::AnnualCommand& command) override;
    /**
     * @brief Updates an annual.
     * @param command Annual command.
     */
    void updateAnnual(const core::ports::workspace::AnnualCommand& command) override;
    /**
     * @brief Deletes an annual.
     * @param id Annual id.
     */
    void deleteAnnual(const std::string& id) override;

    /**
     * @brief Finalizes a statement draft.
     * @param command Finalization command.
     * @return Created statement id.
     */
    std::string finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand& command) override;
    /**
     * @brief Saves a statement draft.
     * @param command Statement draft command.
     */
    void saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command) override;
    /**
     * @brief Clears one statement draft or all drafts when id is empty.
     * @param draftId Statement draft id.
     */
    void clearStatementDraft(const std::string& draftId = {}) override;
    /**
     * @brief Saves an import log.
     * @param command Import log command.
     */
    void saveImportLog(const core::ports::workspace::ImportLogCommand& command) override;
    /**
     * @brief Deletes an import log.
     * @param id Import log id.
     */
    void deleteImportLog(const std::string& id) override;
    /**
     * @brief Saves an export log.
     * @param command Export log command.
     */
    void saveExportLog(const core::ports::workspace::ExportLogCommand& command) override;
    /**
     * @brief Deletes an export log.
     * @param id Export log id.
     */
    void deleteExportLog(const std::string& id) override;

    /** @brief Returns the current workspace path tracked by the storage layer. */
    std::string currentPath() const override;

private:
    void installStateChangedDispatcher();

    SnapshotChanged onSnapshotChanged_;
    std::unique_ptr<WorkspaceSession> session_;
    std::unique_ptr<WorkspaceCommandService> commands_;
    std::unique_ptr<WorkspaceWorkflowService> workflows_;
    std::unique_ptr<WorkspaceQueryService> queries_;
};

} // namespace core::application
