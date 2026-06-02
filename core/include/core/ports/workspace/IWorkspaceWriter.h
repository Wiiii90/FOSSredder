/**
 * @file core/include/core/ports/workspace/IWorkspaceWriter.h
 * @brief Write-side workspace boundary that accepts typed command models.
 */

#pragma once

#include <functional>
#include <string>

#include "core/errors/IErrorReporter.h"
#include "core/ports/storage/IStorageManager.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::workspace {

class IWorkspaceWriter {
public:
    using SnapshotChanged = std::function<void(const WorkspaceSnapshot&)>;
    using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;

    virtual ~IWorkspaceWriter() = default;

    /** @brief Registers callback invoked after state changes. */
    virtual void setSnapshotChangedCallback(SnapshotChanged cb) = 0;
    /** @brief Registers application error reporter used by workspace operations. */
    virtual void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter) = 0;
    /** @brief Registers atomic persistence save callback. */
    virtual void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn) = 0;
    /** @brief Registers atomic persistence load callback. */
    virtual void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn) = 0;
    /** @brief Registers deletion impact callback invoked after save operations. */
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;

    /** @brief Opens latest known workspace file. */
    virtual void openLatest() = 0;
    /** @brief Creates a new workspace file at path. */
    virtual void newFile(const std::string& path) = 0;
    /** @brief Opens existing workspace file from path. */
    virtual void openFile(const std::string& path) = 0;
    /** @brief Saves workspace to current path. */
    virtual void saveFile() = 0;
    /** @brief Saves workspace to specified path. */
    virtual void saveFileAs(const std::string& path) = 0;
    /** @brief Commits in-memory state to persistence and publishes snapshot. */
    virtual void commit() = 0;
    /** @brief Publishes the current snapshot without persistence changes. */
    virtual void notifySnapshot() = 0;

    /** @brief Adds actor from command and returns created id. */
    [[nodiscard]] virtual std::string addActor(const ActorCommand& command) = 0;
    /** @brief Updates actor from command. */
    virtual void updateActor(const ActorCommand& command) = 0;
    /** @brief Deletes actor by id. */
    virtual void deleteActor(const std::string& id) = 0;

    /** @brief Adds property from command and returns created id. */
    [[nodiscard]] virtual std::string addProperty(const PropertyCommand& command) = 0;
    /** @brief Updates property from command. */
    virtual void updateProperty(const PropertyCommand& command) = 0;
    /** @brief Deletes property by id. */
    virtual void deleteProperty(const std::string& id) = 0;

    /** @brief Adds contract from command and returns created id. */
    [[nodiscard]] virtual std::string addContract(const ContractCommand& command) = 0;
    /** @brief Updates contract from command. */
    virtual void updateContract(const ContractCommand& command) = 0;
    /** @brief Deletes contract by id. */
    virtual void deleteContract(const std::string& id) = 0;

    /** @brief Adds statement from command and returns created id. */
    [[nodiscard]] virtual std::string addStatement(const StatementCommand& command) = 0;
    /** @brief Updates statement from command. */
    virtual void updateStatement(const StatementCommand& command) = 0;
    /** @brief Deletes statement by id. */
    virtual void deleteStatement(const std::string& id) = 0;

    /** @brief Adds transaction from command and returns created id. */
    [[nodiscard]] virtual std::string addTransaction(const TransactionCommand& command) = 0;
    /** @brief Updates transaction from command. */
    virtual void updateTransaction(const TransactionCommand& command) = 0;
    /** @brief Deletes transaction by id. */
    virtual void deleteTransaction(const std::string& id) = 0;

    /** @brief Adds analysis from command and returns created id. */
    [[nodiscard]] virtual std::string addAnalysis(const AnalysisCommand& command) = 0;
    /** @brief Updates analysis from command. */
    virtual void updateAnalysis(const AnalysisCommand& command) = 0;
    /** @brief Deletes analysis by id. */
    virtual void deleteAnalysis(const std::string& id) = 0;

    /** @brief Adds annual aggregate from command and returns created id. */
    [[nodiscard]] virtual std::string addAnnual(const AnnualCommand& command) = 0;
    /** @brief Updates annual aggregate from command. */
    virtual void updateAnnual(const AnnualCommand& command) = 0;
    /** @brief Deletes annual aggregate by id. */
    virtual void deleteAnnual(const std::string& id) = 0;

    /** @brief Finalizes a statement draft and returns created statement id. */
    [[nodiscard]] virtual std::string finalizeStatementDraft(const FinalizeStatementDraftCommand& command) = 0;
    /** @brief Saves or updates statement draft from command. */
    virtual void saveStatementDraft(const StatementDraftCommand& command) = 0;
    /** @brief Clears one draft by id or all drafts when id is empty. */
    virtual void clearStatementDraft(const std::string& draftId = {}) = 0;

    /** @brief Replaces import logs with command payload. */
    virtual void setImportLogs(const ImportLogsCommand& command) = 0;
    /** @brief Saves or updates one import log. */
    virtual void saveImportLog(const ImportLogCommand& command) = 0;
    /** @brief Deletes one import log by id. */
    virtual void deleteImportLog(const std::string& id) = 0;
    /** @brief Deletes all import logs. */
    virtual void clearImportLogs() = 0;
    /** @brief Replaces export logs with command payload. */
    virtual void setExportLogs(const ExportLogsCommand& command) = 0;
    /** @brief Saves or updates one export log. */
    virtual void saveExportLog(const ExportLogCommand& command) = 0;
    /** @brief Deletes one export log by id. */
    virtual void deleteExportLog(const std::string& id) = 0;
    /** @brief Deletes all export logs. */
    virtual void clearExportLogs() = 0;
};

} // namespace core::ports::workspace
