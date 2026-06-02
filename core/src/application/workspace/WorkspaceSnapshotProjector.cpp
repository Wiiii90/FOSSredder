/**
 * @file core/src/application/workspace/WorkspaceSnapshotProjector.cpp
 * @brief Implements projection from workspace session state to port snapshots.
 */

#include "core/application/workspace/WorkspaceSnapshotProjector.h"

#include <algorithm>

namespace {

core::ports::workspace::AliasSnapshot projectAlias(const core::domain::Alias& alias) {
    return {alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()};
}

core::ports::workspace::TransactionDraftSnapshot projectTransactionDraft(const core::application::importing::draft::TransactionDraft& draft) {
    core::ports::workspace::TransactionDraftSnapshot snapshot;
    snapshot.id = draft.id;
    snapshot.statementDraftId = draft.statementDraftId;
    snapshot.name = draft.name;
    snapshot.bookingDate = draft.bookingDate;
    snapshot.valuta = draft.valuta;
    snapshot.amount = draft.amount;
    snapshot.actorId = draft.actorId;
    snapshot.contractId = draft.contractId;
    snapshot.propertyIds = draft.propertyIds;
    snapshot.status = static_cast<int>(draft.status);
    snapshot.allocatable = draft.allocatable;
    snapshot.position = draft.position;
    snapshot.metadata = draft.metadata;
    snapshot.proofImageData = draft.proofImageData;
    return snapshot;
}

} // namespace

namespace core::application {

core::ports::workspace::WorkspaceSnapshot WorkspaceSnapshotProjector::project(
    const core::application::workspace::WorkspaceSessionState& state,
    const std::string& currentPath) const {
    core::ports::workspace::WorkspaceSnapshot snapshot;
    snapshot.currentPath = currentPath;
    snapshot.hasCurrentPath = !currentPath.empty();

    snapshot.actors.reserve(state.catalog.actors().size());
    for (const auto& item : state.catalog.actors()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::ActorSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        row.contractIds = item->contractIds();
        row.aliases.reserve(item->aliases().size());
        for (const auto& alias : item->aliases()) {
            row.aliases.push_back(projectAlias(alias));
        }
        snapshot.actors.push_back(std::move(row));
    }

    snapshot.properties.reserve(state.catalog.properties().size());
    for (const auto& item : state.catalog.properties()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::PropertySnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        row.contractIds = item->contractIds();
        row.aliases.reserve(item->aliases().size());
        for (const auto& alias : item->aliases()) {
            row.aliases.push_back(projectAlias(alias));
        }
        snapshot.properties.push_back(std::move(row));
    }

    snapshot.contracts.reserve(state.catalog.contracts().size());
    for (const auto& item : state.catalog.contracts()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::ContractSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.type = item->type();
        row.allocatableMode = item->allocatableMode();
        row.actorIds = item->actorIds();
        row.propertyIds = item->propertyIds();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        row.aliases.reserve(item->aliases().size());
        for (const auto& alias : item->aliases()) {
            row.aliases.push_back(projectAlias(alias));
        }
        snapshot.contracts.push_back(std::move(row));
    }

    snapshot.statements.reserve(state.catalog.statements().size());
    for (const auto& item : state.catalog.statements()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::StatementSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.transactionIds = item->transactionIds();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        snapshot.statements.push_back(std::move(row));
    }

    snapshot.transactions.reserve(state.catalog.transactions().size());
    for (const auto& item : state.catalog.transactions()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::TransactionSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.bookingDate = item->bookingDate();
        row.valuta = item->valuta();
        row.amount = item->amount();
        row.status = static_cast<int>(item->status());
        row.contractId = item->contractId();
        row.actorId = item->actorId();
        row.statementId = item->statementId();
        row.allocatable = item->isAllocatable();
        row.propertyIds = item->propertyIds();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        snapshot.transactions.push_back(std::move(row));
    }

    snapshot.analyses.reserve(state.catalog.analyses().size());
    for (const auto& item : state.catalog.analyses()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::AnalysisSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.type = item->type();
        row.configJson = item->configJson();
        row.filterSpec = item->filterSpec();
        row.exportFormat = item->exportFormat();
        row.includeCalculationAdjustments = item->includeCalculationAdjustments();
        row.exportStateJson = item->exportStateJson();
        row.snapshotTransactionsJson = item->snapshotTransactionsJson();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        row.adjustments.reserve(item->adjustments().size());
        for (const auto& [key, value] : item->adjustments()) {
            row.adjustments.emplace_back(key, value);
        }
        snapshot.analyses.push_back(std::move(row));
    }

    snapshot.annuals.reserve(state.catalog.annuals().size());
    for (const auto& item : state.catalog.annuals()) {
        if (!item) {
            continue;
        }

        core::ports::workspace::AnnualSnapshot row;
        row.id = item->id();
        row.name = item->name();
        row.year = item->year();
        row.analysisIds = item->analysisIds();
        row.createdAt = item->createdAt();
        row.updatedAt = item->updatedAt();
        snapshot.annuals.push_back(std::move(row));
    }

    snapshot.statementDrafts.reserve(state.workflow.statementDrafts.size());
    for (const auto& item : state.workflow.statementDrafts) {
        if (!item) {
            continue;
        }

        core::ports::workspace::StatementDraftSnapshot row;
        row.id = item->id;
        row.name = item->name;
        row.transactionIds = item->transactionIds;
        row.createdAt = item->createdAt;
        row.updatedAt = item->updatedAt;

        std::vector<const core::application::importing::draft::TransactionDraft*> txDrafts;
        txDrafts.reserve(state.workflow.transactionDrafts.size());
        for (const auto& tx : state.workflow.transactionDrafts) {
            if (!tx || tx->statementDraftId != item->id) {
                continue;
            }
            txDrafts.push_back(tx.get());
        }
        std::sort(txDrafts.begin(), txDrafts.end(), [](const auto* lhs, const auto* rhs) {
            return lhs->position < rhs->position;
        });

        row.transactions.reserve(txDrafts.size());
        for (const auto* tx : txDrafts) {
            row.transactions.push_back(projectTransactionDraft(*tx));
        }

        snapshot.statementDrafts.push_back(std::move(row));
    }

    snapshot.importLogs.reserve(state.workflow.importLogs.size());
    for (const auto& item : state.workflow.importLogs) {
        if (!item) {
            continue;
        }

        snapshot.importLogs.push_back({
            item->id,
            item->time,
            item->type,
            item->file,
            item->status,
            item->message,
            item->draftAttached,
            item->draftId,
            item->statementDraftIds,
            item->statementId
        });
    }

    snapshot.exportLogs.reserve(state.workflow.exportLogs.size());
    for (const auto& item : state.workflow.exportLogs) {
        if (!item) {
            continue;
        }

        snapshot.exportLogs.push_back({
            item->id,
            item->time,
            item->targetPath,
            item->status,
            item->message,
            item->payload,
            item->annualIds,
            item->analysisIds
        });
    }

    return snapshot;
}

std::optional<core::ports::workspace::StatementDraftSnapshot> WorkspaceSnapshotProjector::projectStatementDraft(
    const core::application::workspace::WorkspaceSessionState& state,
    const std::string& draftId) const {
    const auto snapshot = project(state, {});
    if (snapshot.statementDrafts.empty()) {
        return std::nullopt;
    }

    if (draftId.empty()) {
        return snapshot.statementDrafts.front();
    }

    const auto it = std::find_if(snapshot.statementDrafts.begin(), snapshot.statementDrafts.end(),
                                 [&draftId](const auto& item) {
                                     return item.id == draftId;
                                 });
    if (it == snapshot.statementDrafts.end()) {
        return std::nullopt;
    }

    return *it;
}

} // namespace core::application
