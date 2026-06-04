/**
 * @file core/include/core/ports/workspace/IWorkspaceWriter.h
 * @brief Write-side workspace boundary that accepts typed command models.
 */

#pragma once

#include <functional>
#include <string>

#include "core/errors/IErrorReporter.h"
#include "core/ports/infra/storage/IStorageManager.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::workspace {

/**
 * @brief Write-side workspace boundary for persistence, CRUD, drafts, and logs.
 */
class IWorkspaceWriter {
public:
    using SnapshotChanged = std::function<void(const WorkspaceSnapshot&)>;
    using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;

    /**
     * @brief Destroys the workspace writer port.
     */
    virtual ~IWorkspaceWriter() = default;

    /**
     * @brief Registers callback invoked after state changes.
     * @param cb Callback receiving the current workspace snapshot.
     */
    virtual void setSnapshotChangedCallback(SnapshotChanged cb) = 0;
    /**
     * @brief Registers application error reporter used by workspace operations.
     * @param reporter Error reporter implementation.
     */
    virtual void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter) = 0;
    /**
     * @brief Registers atomic persistence save callback.
     * @param saveFn Callback used by storage to save the current document.
     */
    virtual void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn) = 0;
    /**
     * @brief Registers atomic persistence load callback.
     * @param loadFn Callback used by storage to load a document.
     */
    virtual void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn) = 0;
    /**
     * @brief Registers deletion impact callback invoked after save operations.
     * @param cb Callback receiving deletion impact information.
     */
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;

    /** @brief Opens latest known workspace file. */
    virtual void openLatest() = 0;
    /**
     * @brief Creates a new workspace file at path.
     * @param path Target workspace file path.
     */
    virtual void newFile(const std::string& path) = 0;
    /**
     * @brief Opens existing workspace file from path.
     * @param path Workspace file path to open.
     */
    virtual void openFile(const std::string& path) = 0;
    /** @brief Saves workspace to current path. */
    virtual void saveFile() = 0;
    /**
     * @brief Saves workspace to specified path.
     * @param path Target workspace file path.
     */
    virtual void saveFileAs(const std::string& path) = 0;
    /** @brief Commits in-memory state to persistence and publishes snapshot. */
    virtual void commit() = 0;
    /** @brief Publishes the current snapshot without persistence changes. */
    virtual void notifySnapshot() = 0;

    /**
     * @brief Validates an actor command without mutating workspace state.
     * @param command Actor command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateActor(const ActorCommand& command) const = 0;
    /**
     * @brief Validates a property command without mutating workspace state.
     * @param command Property command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateProperty(const PropertyCommand& command) const = 0;
    /**
     * @brief Validates a contract command without mutating workspace state.
     * @param command Contract command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateContract(const ContractCommand& command) const = 0;
    /**
     * @brief Validates a statement command without mutating workspace state.
     * @param command Statement command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateStatement(const StatementCommand& command) const = 0;
    /**
     * @brief Validates a transaction command without mutating workspace state.
     * @param command Transaction command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateTransaction(const TransactionCommand& command) const = 0;
    /**
     * @brief Validates an analysis command without mutating workspace state.
     * @param command Analysis command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateAnalysis(const AnalysisCommand& command) const = 0;
    /**
     * @brief Validates an annual command without mutating workspace state.
     * @param command Annual command to validate.
     * @return Validation result with field-level issues.
     */
    [[nodiscard]] virtual ValidationResult validateAnnual(const AnnualCommand& command) const = 0;

    /**
     * @brief Adds an actor from a command.
     * @param command Actor creation command.
     * @return Created actor identifier.
     */
    [[nodiscard]] virtual std::string addActor(const ActorCommand& command) = 0;
    /**
     * @brief Updates an actor from a command.
     * @param command Actor update command.
     */
    virtual void updateActor(const ActorCommand& command) = 0;
    /**
     * @brief Deletes an actor by identifier.
     * @param id Actor identifier.
     */
    virtual void deleteActor(const std::string& id) = 0;

    /**
     * @brief Adds a property from a command.
     * @param command Property creation command.
     * @return Created property identifier.
     */
    [[nodiscard]] virtual std::string addProperty(const PropertyCommand& command) = 0;
    /**
     * @brief Updates a property from a command.
     * @param command Property update command.
     */
    virtual void updateProperty(const PropertyCommand& command) = 0;
    /**
     * @brief Deletes a property by identifier.
     * @param id Property identifier.
     */
    virtual void deleteProperty(const std::string& id) = 0;

    /**
     * @brief Adds a contract from a command.
     * @param command Contract creation command.
     * @return Created contract identifier.
     */
    [[nodiscard]] virtual std::string addContract(const ContractCommand& command) = 0;
    /**
     * @brief Updates a contract from a command.
     * @param command Contract update command.
     */
    virtual void updateContract(const ContractCommand& command) = 0;
    /**
     * @brief Deletes a contract by identifier.
     * @param id Contract identifier.
     */
    virtual void deleteContract(const std::string& id) = 0;

    /**
     * @brief Adds a statement from a command.
     * @param command Statement creation command.
     * @return Created statement identifier.
     */
    [[nodiscard]] virtual std::string addStatement(const StatementCommand& command) = 0;
    /**
     * @brief Updates a statement from a command.
     * @param command Statement update command.
     */
    virtual void updateStatement(const StatementCommand& command) = 0;
    /**
     * @brief Deletes a statement by identifier.
     * @param id Statement identifier.
     */
    virtual void deleteStatement(const std::string& id) = 0;

    /**
     * @brief Adds a transaction from a command.
     * @param command Transaction creation command.
     * @return Created transaction identifier.
     */
    [[nodiscard]] virtual std::string addTransaction(const TransactionCommand& command) = 0;
    /**
     * @brief Updates a transaction from a command.
     * @param command Transaction update command.
     */
    virtual void updateTransaction(const TransactionCommand& command) = 0;
    /**
     * @brief Deletes a transaction by identifier.
     * @param id Transaction identifier.
     */
    virtual void deleteTransaction(const std::string& id) = 0;

    /**
     * @brief Adds an analysis from a command.
     * @param command Analysis creation command.
     * @return Created analysis identifier.
     */
    [[nodiscard]] virtual std::string addAnalysis(const AnalysisCommand& command) = 0;
    /**
     * @brief Updates an analysis from a command.
     * @param command Analysis update command.
     */
    virtual void updateAnalysis(const AnalysisCommand& command) = 0;
    /**
     * @brief Deletes an analysis by identifier.
     * @param id Analysis identifier.
     */
    virtual void deleteAnalysis(const std::string& id) = 0;

    /**
     * @brief Adds an annual aggregate from a command.
     * @param command Annual creation command.
     * @return Created annual identifier.
     */
    [[nodiscard]] virtual std::string addAnnual(const AnnualCommand& command) = 0;
    /**
     * @brief Updates an annual aggregate from a command.
     * @param command Annual update command.
     */
    virtual void updateAnnual(const AnnualCommand& command) = 0;
    /**
     * @brief Deletes an annual aggregate by identifier.
     * @param id Annual identifier.
     */
    virtual void deleteAnnual(const std::string& id) = 0;

    /**
     * @brief Finalizes a statement draft.
     * @param command Finalization command containing the draft and target data.
     * @return Created statement identifier.
     */
    [[nodiscard]] virtual std::string finalizeStatementDraft(const FinalizeStatementDraftCommand& command) = 0;
    /**
     * @brief Saves or updates a statement draft from a command.
     * @param command Statement draft command.
     */
    virtual void saveStatementDraft(const StatementDraftCommand& command) = 0;
    /**
     * @brief Clears one draft by id or all drafts when id is empty.
     * @param draftId Optional statement draft identifier.
     */
    virtual void clearStatementDraft(const std::string& draftId = {}) = 0;

    /**
     * @brief Replaces import logs with command payload.
     * @param command Import logs replacement command.
     */
    virtual void setImportLogs(const ImportLogsCommand& command) = 0;
    /**
     * @brief Saves or updates one import log.
     * @param command Import log command.
     */
    virtual void saveImportLog(const ImportLogCommand& command) = 0;
    /**
     * @brief Deletes one import log by identifier.
     * @param id Import log identifier.
     */
    virtual void deleteImportLog(const std::string& id) = 0;
    /** @brief Deletes all import logs. */
    virtual void clearImportLogs() = 0;
    /**
     * @brief Replaces export logs with command payload.
     * @param command Export logs replacement command.
     */
    virtual void setExportLogs(const ExportLogsCommand& command) = 0;
    /**
     * @brief Saves or updates one export log.
     * @param command Export log command.
     */
    virtual void saveExportLog(const ExportLogCommand& command) = 0;
    /**
     * @brief Deletes one export log by identifier.
     * @param id Export log identifier.
     */
    virtual void deleteExportLog(const std::string& id) = 0;
    /** @brief Deletes all export logs. */
    virtual void clearExportLogs() = 0;
};

} // namespace core::ports::workspace
