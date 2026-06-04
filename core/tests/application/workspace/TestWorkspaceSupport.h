/**
 * @file core/tests/application/workspace/TestWorkspaceSupport.h
 * @brief Shared test helpers for workspace application tests.
 */

#pragma once

#include <optional>
#include <string>
#include <utility>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/ports/infra/storage/IStorageManager.h"

namespace core::tests::application::workspace {

class FakeStorageManager final : public core::ports::storage::IStorageManager {
public:
    void setAtomicStoreSave(AtomicStoreSave saveFn) override {
        saveFn_ = std::move(saveFn);
    }

    void setAtomicStoreLoad(AtomicStoreLoad loadFn) override {
        loadFn_ = std::move(loadFn);
    }

    void setDeletionImpactCallback(DeletionImpactCallback cb) override {
        deletionImpactCallback_ = std::move(cb);
    }

    std::optional<std::string> loadLatestPath() const override {
        return latestPath_;
    }

    core::application::workspace::WorkspaceSessionState loadFrom(const std::string& filePath) override {
        if (loadFn_) {
            return loadFn_(filePath);
        }
        return loadedState_;
    }

    void save(const core::application::workspace::WorkspaceSessionState& document) override {
        savedState_ = document;
        if (latestPath_) {
            currentPath_ = *latestPath_;
        } else {
            currentPath_.clear();
        }
        if (saveFn_) {
            lastDeletionImpact_ = saveFn_(currentPath_, document);
            if (deletionImpactCallback_) {
                deletionImpactCallback_(lastDeletionImpact_);
            }
        }
    }

    void saveAs(const std::string& filePath, const core::application::workspace::WorkspaceSessionState& document) override {
        currentPath_ = filePath;
        latestPath_ = filePath;
        savedState_ = document;
        if (saveFn_) {
            lastDeletionImpact_ = saveFn_(filePath, document);
            if (deletionImpactCallback_) {
                deletionImpactCallback_(lastDeletionImpact_);
            }
        }
    }

    void createNew(const std::string& filePath) override {
        currentPath_ = filePath;
        latestPath_ = filePath;
        savedState_ = {};
    }

    const std::string& currentPath() const noexcept override {
        return currentPath_;
    }

    core::application::workspace::WorkspaceSessionState loadedState_{};
    core::application::workspace::WorkspaceSessionState savedState_{};
    core::domain::DeletionImpact lastDeletionImpact_{};

private:
    AtomicStoreSave saveFn_{};
    AtomicStoreLoad loadFn_{};
    DeletionImpactCallback deletionImpactCallback_{};
    std::string currentPath_;
    std::optional<std::string> latestPath_;
};

} // namespace core::tests::application::workspace
