/**
 * @file core/src/application/storage/StorageManager.cpp
 * @brief Implements atomic workspace persistence and latest-path registry handling.
 */

#include "core/pch.h"
#include "core/application/storage/StorageManager.h"

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace core::storage {

StorageManager::StorageManager(
    std::shared_ptr<core::ports::storage::IRegistry> registry)
    : registry_(std::move(registry)) {
}

void StorageManager::setAtomicStoreSave(AtomicStoreSave saveFn) {
    atomicSave_ = std::move(saveFn);
}

void StorageManager::setAtomicStoreLoad(AtomicStoreLoad loadFn) {
    atomicLoad_ = std::move(loadFn);
}

void StorageManager::setDeletionImpactCallback(DeletionImpactCallback cb) {
    onDeletionImpact_ = std::move(cb);
}

std::optional<std::string> StorageManager::loadLatestPath() const {
    if (registry_) {
        return registry_->getLatest();
    }

    return std::nullopt;
}

void StorageManager::rememberLatestPath(const std::string& filePath) {
    if (registry_) {
        registry_->setLatest(filePath);
    }
}

core::application::workspace::WorkspaceSessionState StorageManager::loadFrom(const std::string& filePath) {
    const std::string previousPath = currentPath_;
    currentPath_ = filePath;

    try {
        if (!atomicLoad_) {
            throw std::runtime_error("Atomic store load callback not configured");
        }

        const core::application::workspace::WorkspaceSessionState document = atomicLoad_(currentPath_);
        rememberLatestPath(filePath);
        return document;
    } catch (...) {
        currentPath_ = previousPath;
        throw;
    }
}

void StorageManager::save(const core::application::workspace::WorkspaceSessionState& document) {
    if (currentPath_.empty()) {
        throw std::runtime_error("No file opened");
    }

    saveAs(currentPath_, document);
}

void StorageManager::saveAs(const std::string& filePath, const core::application::workspace::WorkspaceSessionState& document) {
    const std::string previousPath = currentPath_;
    currentPath_ = filePath;

    try {
        if (!atomicSave_) {
            throw std::runtime_error("Atomic store save callback not configured");
        }

        DeletionImpact impact = atomicSave_(currentPath_, document);
        rememberLatestPath(filePath);
        if (onDeletionImpact_ && !impact.empty()) {
            onDeletionImpact_(impact);
        }
    } catch (...) {
        currentPath_ = previousPath;
        throw;
    }
} // namespace core::storage

void StorageManager::createNew(const std::string& filePath) {
    std::filesystem::path p(filePath);
    auto dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    currentPath_ = filePath;

    if (atomicLoad_) {
        (void)atomicLoad_(currentPath_);
    }

    rememberLatestPath(filePath);
}

}
