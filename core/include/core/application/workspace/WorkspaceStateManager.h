/**
 * @file core/include/core/application/workspace/WorkspaceStateManager.h
 * @brief Loader/saver for internal workspace session state using repository abstractions.
 */

#pragma once

#include <memory>

namespace core::application::workspace {
struct WorkspaceSessionState;
}

namespace core::storage {
struct RepositoryBundle;
}

namespace core::application {

/**
 * @brief Loads and saves aggregate workspace state via repository abstractions.
 */
class WorkspaceStateManager {
public:
    using Repositories = core::storage::RepositoryBundle;

    /** @brief Creates a manager over the supplied repository bundle. */
    explicit WorkspaceStateManager(Repositories repos);
    ~WorkspaceStateManager();

    WorkspaceStateManager(const WorkspaceStateManager&) = delete;
    WorkspaceStateManager& operator=(const WorkspaceStateManager&) = delete;
    WorkspaceStateManager(WorkspaceStateManager&&) noexcept;
    WorkspaceStateManager& operator=(WorkspaceStateManager&&) noexcept;

    /** @brief Loads, rehydrates and validates workspace session state. */
    core::application::workspace::WorkspaceSessionState load();
    /** @brief Projects, validates and persists workspace session state. */
    void save(const core::application::workspace::WorkspaceSessionState& document);

    /** @brief Enables or disables strict validation during load and save. */
    void setStrictValidation(bool enabled) noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
