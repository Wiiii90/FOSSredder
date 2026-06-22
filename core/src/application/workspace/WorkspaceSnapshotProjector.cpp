/**
 * @file core/src/application/workspace/WorkspaceSnapshotProjector.cpp
 * @brief Implements projection from workspace session state to port snapshots.
 */

#include "core/application/workspace/WorkspaceSnapshotProjector.h"

#include "core/application/analysis/AnalysisKeys.h"
#include "core/ports/usecases/analysis/AnalysisRequest.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string_view>

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

std::string jsonString(const nlohmann::json& object, std::string_view key) {
    const auto it = object.find(std::string(key));
    return it != object.end() && it->is_string() ? it->get<std::string>() : std::string{};
}

double jsonDouble(const nlohmann::json& object, std::string_view key) {
    const auto it = object.find(std::string(key));
    return it != object.end() && it->is_number() ? it->get<double>() : 0.0;
}

bool jsonBool(const nlohmann::json& object, std::string_view key) {
    const auto it = object.find(std::string(key));
    return it != object.end() && it->is_boolean() ? it->get<bool>() : false;
}

std::vector<std::string> jsonStringList(const nlohmann::json& object,
                                        std::string_view key) {
    std::vector<std::string> out;
    const auto it = object.find(std::string(key));
    if (it == object.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (item.is_string()) {
            out.push_back(item.get<std::string>());
        }
    }
    return out;
}

core::ports::analysis::AnalysisConfigInput
analysisConfigFromJson(const std::string& raw, const std::string& type) {
    core::ports::analysis::AnalysisConfigInput out;
    out.type = type;
    if (raw.empty()) {
        return out;
    }
    nlohmann::json config;
    try {
        config = nlohmann::json::parse(raw);
    } catch (...) {
        return out;
    }
    if (!config.is_object()) {
        return out;
    }
    out.plotType = jsonString(config, core::application::analysis::keys::kPlotTypeKey);
    out.plotMeasure = jsonString(config, core::application::analysis::keys::kPlotMeasureKey);
    out.propertyIds = jsonStringList(config, core::application::analysis::keys::kPropertiesKey);
    out.contractTypes = jsonStringList(config, core::application::analysis::keys::kContractTypesKey);
    out.taxPercent = jsonDouble(config, core::application::analysis::keys::calculation::kPercentKey);
    return out;
}

std::vector<core::ports::workspace::TransactionSnapshot>
snapshotTransactionsFromJson(const std::string& raw) {
    std::vector<core::ports::workspace::TransactionSnapshot> out;
    if (raw.empty()) {
        return out;
    }
    nlohmann::json rows;
    try {
        rows = nlohmann::json::parse(raw);
    } catch (...) {
        return out;
    }
    if (rows.is_object()) {
        const auto transactions = rows.find("transactions");
        rows = transactions != rows.end() ? *transactions : nlohmann::json{};
    }
    if (!rows.is_array()) {
        return out;
    }
    out.reserve(rows.size());
    for (const auto& row : rows) {
        if (!row.is_object()) {
            continue;
        }
        core::ports::workspace::TransactionSnapshot tx;
        tx.id = jsonString(row, "id");
        if (tx.id.empty()) {
            tx.id = jsonString(row, "transactionId");
        }
        tx.name = jsonString(row, "name");
        if (tx.name.empty()) {
            tx.name = jsonString(row, "transactionName");
        }
        tx.bookingDate = jsonString(row, "bookingDate");
        if (tx.bookingDate.empty()) {
            tx.bookingDate = jsonString(row, "date");
        }
        tx.valuta = jsonString(row, "valuta");
        tx.amount = jsonDouble(row, "amount");
        tx.status = static_cast<int>(jsonDouble(row, "status"));
        tx.contractId = jsonString(row, "contractId");
        tx.contractType = jsonString(row, "contractType");
        tx.actorId = jsonString(row, "actorId");
        tx.statementId = jsonString(row, "statementId");
        tx.allocatable = jsonBool(row, "allocatable");
        tx.propertyIds = jsonStringList(row, "propertyIds");
        tx.propertyNames = jsonStringList(row, "propertyNames");
        out.push_back(std::move(tx));
    }
    return out;
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
        row.config = analysisConfigFromJson(item->configJson(), item->type());
        row.filter = core::ports::analysis::parseAnalysisFilterSelection(item->filterSpec());
        row.exportFormat = item->exportFormat();
        row.includeCalculationAdjustments = item->includeCalculationAdjustments();
        row.snapshotTransactions = snapshotTransactionsFromJson(item->snapshotTransactionsJson());
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
