/**
 * @file core/include/core/ports/infra/storage/IStorageManager.h
 * @brief Storage manager port used by application services.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/application/storage/DeletionImpact.h"

namespace core::ports::storage {

class IStorageManager {
public:
    /**
     * @brief Destroy the storage manager port.
     */
    using AtomicStoreSave = std::function<core::domain::DeletionImpact(const std::string& dbPath, const core::application::workspace::WorkspaceSessionState& document)>;
    using AtomicStoreLoad = std::function<core::application::workspace::WorkspaceSessionState(const std::string& dbPath)>;
    using DeletionImpactCallback = std::function<void(const core::domain::DeletionImpact&)>;

    virtual ~IStorageManager() = default;

    /**
     * @brief Configure the atomic save callback.
     * @param saveFn Save callback.
     */
    virtual void setAtomicStoreSave(AtomicStoreSave saveFn) = 0;

    /**
     * @brief Configure the atomic load callback.
     * @param loadFn Load callback.
     */
    virtual void setAtomicStoreLoad(AtomicStoreLoad loadFn) = 0;

    /**
     * @brief Configure the deletion impact callback.
     * @param cb Callback invoked after save operations.
     */
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;

    /**
     * @brief Load the latest known path.
     * @return Latest path, if one is stored.
     */
    virtual std::optional<std::string> loadLatestPath() const = 0;

    /**
     * @brief Load an application state from a file path.
     * @param filePath File path to load.
     * @return Loaded application state.
     */
    virtual core::application::workspace::WorkspaceSessionState loadFrom(const std::string& filePath) = 0;

    /**
     * @brief Save the current application state.
     * @param state State to save.
     */
    virtual void save(const core::application::workspace::WorkspaceSessionState& document) = 0;

    /**
     * @brief Save the current application state to a specific file path.
     * @param filePath Target file path.
     * @param state State to save.
     */
    virtual void saveAs(const std::string& filePath, const core::application::workspace::WorkspaceSessionState& document) = 0;

    /**
     * @brief Create a new storage target.
     * @param filePath File path to create.
     */
    virtual void createNew(const std::string& filePath) = 0;

    /**
     * @brief Get the current storage path.
     * @return Current storage path.
     */
    virtual const std::string& currentPath() const noexcept = 0;
};

} // namespace core::ports::storage
