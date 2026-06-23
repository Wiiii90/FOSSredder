/**
 * @file core/include/core/application/workspace/WorkspaceSnapshotProjector.h
 * @brief Projects internal workspace session state into immutable port snapshots.
 */

#pragma once

#include <optional>
#include <string>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::application {

class WorkspaceSnapshotProjector {
public:
    /**
     * @brief Builds an immutable workspace snapshot from session state.
     * @param state Internal session state.
     * @param currentPath Current workspace file path.
     * @return Full workspace snapshot for port consumers.
     */
    [[nodiscard]] core::ports::workspace::WorkspaceSnapshot project(const core::application::workspace::WorkspaceSessionState& state,
                                                                    const std::string& currentPath) const;

    /**
     * @brief Resolves a single draft snapshot from session state.
     * @param state Internal session state.
     * @param draftId Optional draft identifier.
     * @return Matching statement draft snapshot if present.
     */
    [[nodiscard]] std::optional<core::ports::workspace::StatementDraftSnapshot> projectStatementDraft(
        const core::application::workspace::WorkspaceSessionState& state,
        const std::string& draftId = {}) const;
};

} // namespace core::application
