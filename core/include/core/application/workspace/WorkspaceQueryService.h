/**
 * @file core/include/core/application/workspace/WorkspaceQueryService.h
 * @brief Reads workspace snapshots and draft projections from the application layer.
 */

#pragma once

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/application/workspace/WorkspaceSnapshotProjector.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <optional>
#include <string>
#include <vector>

namespace core::application {

class WorkspaceSession;

class WorkspaceQueryService {
public:
    /** @brief Creates the query service for one workspace session. */
    explicit WorkspaceQueryService(WorkspaceSession& session);

    WorkspaceQueryService(const WorkspaceQueryService&) = delete;
    WorkspaceQueryService& operator=(const WorkspaceQueryService&) = delete;
    WorkspaceQueryService(WorkspaceQueryService&&) noexcept;
    WorkspaceQueryService& operator=(WorkspaceQueryService&&) noexcept;

    /** @brief Projects full workspace snapshot for port consumers. */
    [[nodiscard]] core::ports::workspace::WorkspaceSnapshot workspaceSnapshot() const;

    /**
     * @brief Projects one statement draft snapshot.
     * @param draftId Optional draft identifier.
     * @return Matching draft snapshot when found.
     */
    [[nodiscard]] std::optional<core::ports::workspace::StatementDraftSnapshot> statementDraftSnapshot(const std::string& draftId = {}) const;
    /** @brief Finds an actor identity by normalized name. */
    [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot actorIdentityByName(const std::string& name) const;
    /** @brief Finds a property identity by normalized name. */
    [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot propertyIdentityByName(const std::string& name) const;
    /** @brief Finds a contract identity by normalized signature. */
    [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot contractIdentityBySignature(
        const std::string& name,
        const std::string& type,
        const std::vector<std::string>& actorIds,
        const std::vector<std::string>& propertyIds) const;
    /** @brief Returns the next generated contract name. */
    [[nodiscard]] std::string nextContractName() const;
    /** @brief Returns current workspace file path. */
    const std::string& currentPath() const noexcept;

private:
    const core::application::workspace::WorkspaceSessionState& stateRef() const noexcept;

    WorkspaceSession* session_ = nullptr;
    WorkspaceSnapshotProjector projector_;
};

} // namespace core::application
