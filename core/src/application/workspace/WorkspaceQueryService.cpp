/**
 * @file core/src/application/workspace/WorkspaceQueryService.cpp
 * @brief Implements snapshot-based workspace read-side query projections.
 */

#include "core/pch.h"

#include "core/application/workspace/WorkspaceQueryService.h"

#include "core/application/workspace/WorkspaceSession.h"
#include "core/domain/entities/Contract.h"
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

std::vector<std::string> normalizedValues(const std::vector<std::string>& values) {
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        const auto normalized = core::domain::policies::alias::trimCopy(value);
        if (!normalized.empty()) {
            out.push_back(normalized);
        }
    }
    return out;
}

bool containsValue(const std::vector<std::string>& values,
                   const std::string& value) {
    const auto target = core::domain::policies::alias::trimCopy(value);
    if (target.empty()) {
        return false;
    }
    const auto normalized = normalizedValues(values);
    return std::find(normalized.begin(), normalized.end(), target) != normalized.end();
}

bool containsAllValues(const std::vector<std::string>& values,
                       const std::vector<std::string>& required) {
    const auto normalizedRequired = normalizedValues(required);
    return std::all_of(normalizedRequired.begin(), normalizedRequired.end(),
                       [&](const std::string& value) {
                           return containsValue(values, value);
                       });
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

const core::domain::Contract* findContractById(
    const core::application::workspace::WorkspaceSessionState& state,
    const std::string& id) {
    const auto target = core::domain::policies::alias::trimCopy(id);
    if (target.empty()) {
        return nullptr;
    }
    for (const auto& contract : state.catalog.contracts()) {
        if (contract && contract->id() == target) {
            return contract.get();
        }
    }
    return nullptr;
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

core::ports::workspace::TransactionCatalogSelection
WorkspaceQueryService::transactionCatalogSelection(
    const core::ports::workspace::TransactionCatalogSelectionChange& change) const {
    core::ports::workspace::TransactionCatalogSelection out = change.current;

    if (change.contractChanged) {
        out.contractId = core::domain::policies::alias::trimCopy(change.contractId);
        if (const auto* contract = findContractById(stateRef(), out.contractId)) {
            const auto actorIds = normalizedValues(contract->actorIds());
            out.actorId = actorIds.empty() ? std::string{} : actorIds.front();
            out.propertyIds = normalizedValues(contract->propertyIds());
        } else {
            out.actorId.clear();
            out.propertyIds.clear();
        }
    }

    if (change.actorChanged) {
        out.actorId = core::domain::policies::alias::trimCopy(change.actorId);
        const auto* contract = findContractById(stateRef(), out.contractId);
        if (contract && !out.actorId.empty() &&
            !containsValue(contract->actorIds(), out.actorId)) {
            out.contractId.clear();
        }
    }

    if (change.propertiesChanged) {
        out.propertyIds = normalizedValues(change.propertyIds);
        const auto* contract = findContractById(stateRef(), out.contractId);
        if (contract && !containsAllValues(contract->propertyIds(), out.propertyIds)) {
            out.contractId.clear();
        }
    }

    return out;
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
