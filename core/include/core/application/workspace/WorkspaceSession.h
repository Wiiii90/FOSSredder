/**
 * @file core/include/core/application/workspace/WorkspaceSession.h
 * @brief Declares the mutable workspace session that ties in-memory state to storage.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/infra/storage/IStorageManager.h"

#include <functional>
#include <memory>
#include <string>

namespace core::application {

/**
 * @brief Maintains the editable app state for one workspace and persists it through `IStorageManager`.
 */
class WorkspaceSession {
public:
    using StateChanged = std::function<void(const core::application::workspace::WorkspaceSessionState&)>;

    /**
     * @brief Creates a session over an existing storage manager.
     * @throws std::invalid_argument when `storageManager` is null.
     */
    explicit WorkspaceSession(std::unique_ptr<core::ports::storage::IStorageManager> storageManager);

    const core::application::workspace::WorkspaceSessionState& state() const noexcept { return document_; }
    core::application::workspace::WorkspaceSessionState& mutableState() noexcept { return document_; }
    const core::domain::catalog::WorkspaceCatalog& catalogState() const noexcept { return document_.catalog; }
    core::domain::catalog::WorkspaceCatalog& mutableCatalogState() noexcept { return document_.catalog; }
    const std::string& currentPath() const noexcept;

    void setStateChangedCallback(StateChanged cb);
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);
    void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn);
    void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn);
    void setDeletionImpactCallback(core::ports::storage::IStorageManager::DeletionImpactCallback cb);

    void openLatest();
    void newFile(const std::string& path);
    void openFile(const std::string& path);
    void saveFile();
    void saveFileAs(const std::string& path);
    /** @brief Persists the session when a file is open and notifies listeners. */
    void commit();
    void notifyState();

private:
    void reportException(core::errors::ErrorSeverity severity,
                         const char* origin,
                         std::exception_ptr exception) const;

    std::unique_ptr<core::ports::storage::IStorageManager> storageManager_;
    core::application::workspace::WorkspaceSessionState document_;
    StateChanged onStateChanged_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
    core::ports::storage::IStorageManager::DeletionImpactCallback onDeletionImpact_;
};

} // namespace core::application
