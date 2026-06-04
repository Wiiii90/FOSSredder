/**
 * @file core/src/application/workspace/WorkspaceQueryService.cpp
 * @brief Implements snapshot-based workspace read-side query projections.
 */

#include "core/pch.h"

#include "core/application/workspace/WorkspaceQueryService.h"

#include "core/application/workspace/WorkspaceSession.h"
#include "core/domain/policies/AliasPolicy.h"

#include <algorithm>

namespace {

constexpr auto kContractBaseName = "Contract";

std::string normalizedText(const std::string& value) {
    return core::domain::policies::alias::canonicalAliasValue(value);
}

std::vector<std::string> normalizedSorted(std::vector<std::string> values) {
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        const auto normalized = core::domain::policies::alias::trimCopy(value);
        if (!normalized.empty()) {
            out.push_back(normalized);
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

bool sameStringSet(const std::vector<std::string>& lhs,
                   const std::vector<std::string>& rhs) {
    return normalizedSorted(lhs) == normalizedSorted(rhs);
}

int trailingContractIndex(const std::string& value) {
    const auto trimmed = core::domain::policies::alias::trimCopy(value);
    const std::string prefix = std::string(kContractBaseName) + " ";
    if (trimmed.size() <= prefix.size() ||
        normalizedText(trimmed.substr(0, prefix.size())) != normalizedText(prefix)) {
        return -1;
    }
    try {
        std::size_t consumed = 0;
        const int index = std::stoi(trimmed.substr(prefix.size()), &consumed);
        return consumed == trimmed.size() - prefix.size() ? index : -1;
    } catch (...) {
        return -1;
    }
}

core::ports::workspace::WorkspaceIdentitySnapshot identity(
    const std::string& id,
    const std::string& name) {
    core::ports::workspace::WorkspaceIdentitySnapshot out;
    out.id = id;
    out.name = name;
    return out;
}

} // namespace

namespace core::application {

WorkspaceQueryService::WorkspaceQueryService(WorkspaceSession& session)
    : session_(&session) {
}

WorkspaceQueryService::WorkspaceQueryService(WorkspaceQueryService&&) noexcept = default;

WorkspaceQueryService& WorkspaceQueryService::operator=(WorkspaceQueryService&&) noexcept = default;

const core::application::workspace::WorkspaceSessionState& WorkspaceQueryService::stateRef() const noexcept {
    return session_->state();
}

core::ports::workspace::WorkspaceSnapshot WorkspaceQueryService::workspaceSnapshot() const {
    return projector_.project(stateRef(), currentPath());
}

const std::string& WorkspaceQueryService::currentPath() const noexcept {
    return session_->currentPath();
}

std::optional<core::ports::workspace::StatementDraftSnapshot> WorkspaceQueryService::statementDraftSnapshot(const std::string& draftId) const {
    return projector_.projectStatementDraft(stateRef(), draftId);
}

core::ports::workspace::WorkspaceIdentitySnapshot WorkspaceQueryService::actorIdentityByName(const std::string& name) const {
    const auto target = normalizedText(name);
    if (target.empty()) {
        return {};
    }
    for (const auto& actor : stateRef().catalog.actors()) {
        if (actor && normalizedText(actor->name()) == target) {
            return identity(actor->id(), actor->name());
        }
    }
    return {};
}

core::ports::workspace::WorkspaceIdentitySnapshot WorkspaceQueryService::propertyIdentityByName(const std::string& name) const {
    const auto target = normalizedText(name);
    if (target.empty()) {
        return {};
    }
    for (const auto& property : stateRef().catalog.properties()) {
        if (property && normalizedText(property->name()) == target) {
            return identity(property->id(), property->name());
        }
    }
    return {};
}

core::ports::workspace::WorkspaceIdentitySnapshot WorkspaceQueryService::contractIdentityBySignature(
    const std::string& name,
    const std::string& type,
    const std::vector<std::string>& actorIds,
    const std::vector<std::string>& propertyIds) const {
    const auto normalizedName = normalizedText(name);
    const auto normalizedType = normalizedText(type);
    if (normalizedName.empty() || normalizedType.empty()) {
        return {};
    }
    for (const auto& contract : stateRef().catalog.contracts()) {
        if (!contract ||
            normalizedText(contract->name()) != normalizedName ||
            normalizedText(contract->type()) != normalizedType ||
            !sameStringSet(contract->actorIds(), actorIds) ||
            !sameStringSet(contract->propertyIds(), propertyIds)) {
            continue;
        }

        auto out = identity(contract->id(), contract->name());
        out.type = contract->type();
        out.allocatableMode = contract->allocatableMode();
        out.actorIds = normalizedSorted(contract->actorIds());
        out.propertyIds = normalizedSorted(contract->propertyIds());
        return out;
    }
    return {};
}

std::string WorkspaceQueryService::nextContractName() const {
    int maxIndex = 0;
    for (const auto& contract : stateRef().catalog.contracts()) {
        if (!contract) {
            continue;
        }
        maxIndex = std::max(maxIndex, trailingContractIndex(contract->name()));
    }
    return std::string(kContractBaseName) + " " + std::to_string(maxIndex + 1);
}

} // namespace core::application
