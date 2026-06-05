/**
 * @file core/include/core/ports/workspace/IWorkspaceReader.h
 * @brief Read-side workspace boundary that exposes immutable snapshots.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

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

    /** @brief Finds an actor identity by display name. */
    [[nodiscard]] virtual WorkspaceIdentitySnapshot actorIdentityByName(const std::string& name) const = 0;

    /** @brief Finds a property identity by display name. */
    [[nodiscard]] virtual WorkspaceIdentitySnapshot propertyIdentityByName(const std::string& name) const = 0;

    /** @brief Finds a contract identity by name, type, actor ids, and property ids. */
    [[nodiscard]] virtual WorkspaceIdentitySnapshot contractIdentityBySignature(
        const std::string& name,
        const std::string& type,
        const std::vector<std::string>& actorIds,
        const std::vector<std::string>& propertyIds) const = 0;

    /**
     * @brief Applies catalog consistency rules to a transaction form selection.
     * @param change Current selection plus the user-selected field change.
     * @return Updated selection with invalid dependent choices cleared or filled.
     */
    [[nodiscard]] virtual TransactionCatalogSelection transactionCatalogSelection(
        const TransactionCatalogSelectionChange& change) const = 0;

    /** @brief Returns the next generated contract name. */
    [[nodiscard]] virtual std::string nextContractName() const = 0;
};

} // namespace core::ports::workspace
