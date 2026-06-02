/**
 * @file core/include/core/ports/workspace/IWorkspaceReader.h
 * @brief Read-side workspace boundary that exposes immutable snapshots.
 */

#pragma once

#include <optional>
#include <string>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::workspace {

class IWorkspaceReader {
public:
    virtual ~IWorkspaceReader() = default;

    /** @brief Returns the full immutable workspace snapshot. */
    [[nodiscard]] virtual WorkspaceSnapshot workspaceSnapshot() const = 0;

    /** @brief Returns the current workspace path, or an empty string when none is open. */
    [[nodiscard]] virtual std::string currentPath() const = 0;

    /**
     * @brief Returns one statement draft snapshot.
     * @param draftId Optional draft identifier; when empty, the first draft is used.
     * @return Matching statement draft snapshot if available.
     */
    [[nodiscard]] virtual std::optional<StatementDraftSnapshot> statementDraftSnapshot(const std::string& draftId = {}) const = 0;
};

} // namespace core::ports::workspace
