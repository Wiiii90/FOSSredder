/**
 * @file core/src/application/workspace/WorkspaceSnapshotCatalogMapper.cpp
 * @brief Implements mapping from workspace port snapshots to catalog state.
 */

#include "core/application/workspace/WorkspaceSnapshotCatalogMapper.h"

#include "core/application/analysis/AnalysisWorkflowSupport.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Alias.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace {

std::vector<core::domain::Alias> toAliases(
    const std::vector<core::ports::workspace::AliasSnapshot>& source) {
    std::vector<core::domain::Alias> aliases;
    aliases.reserve(source.size());
    for (const auto& alias : source) {
        aliases.emplace_back(alias.value,
                             alias.kind,
                             alias.source,
                             alias.createdAt,
                             alias.updatedAt,
                             alias.hitCount,
                             alias.lastUsedAt);
    }
    return aliases;
}

std::string serializeSnapshotTransactionsForDomain(
    const std::vector<core::ports::workspace::TransactionSnapshot>& source) {
    if (source.empty()) {
        return {};
    }
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& tx : source) {
        rows.push_back(nlohmann::json{
            {"id", tx.id},
            {"transactionId", tx.id},
            {"name", tx.name},
            {"transactionName", tx.name},
            {"bookingDate", tx.bookingDate},
            {"date", tx.bookingDate},
            {"valuta", tx.valuta},
            {"amount", tx.amount},
            {"status", tx.status},
            {"contractId", tx.contractId},
            {"contractType", tx.contractType},
            {"actorId", tx.actorId},
            {"statementId", tx.statementId},
            {"allocatable", tx.allocatable},
            {"propertyIds", tx.propertyIds},
            {"propertyNames", tx.propertyNames},
        });
    }
    return rows.dump();
}

} // namespace

namespace core::application::workspace {

core::domain::catalog::WorkspaceCatalog
toWorkspaceCatalog(const core::ports::workspace::WorkspaceSnapshot& snapshot) {
    core::domain::catalog::WorkspaceCatalog state;

    core::domain::catalog::WorkspaceCatalog::ActorList actors;
    actors.reserve(snapshot.actors.size());
    for (const auto& src : snapshot.actors) {
        auto entity = std::make_shared<core::domain::Actor>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setAliases(toAliases(src.aliases));
        entity->setContractIds(src.contractIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        actors.push_back(std::move(entity));
    }
    state.setActors(std::move(actors));

    core::domain::catalog::WorkspaceCatalog::PropertyList properties;
    properties.reserve(snapshot.properties.size());
    for (const auto& src : snapshot.properties) {
        auto entity = std::make_shared<core::domain::Property>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setAliases(toAliases(src.aliases));
        entity->setContractIds(src.contractIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        properties.push_back(std::move(entity));
    }
    state.setProperties(std::move(properties));

    core::domain::catalog::WorkspaceCatalog::ContractList contracts;
    contracts.reserve(snapshot.contracts.size());
    for (const auto& src : snapshot.contracts) {
        auto entity = std::make_shared<core::domain::Contract>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setType(src.type);
        entity->setAllocatableMode(src.allocatableMode);
        entity->setActorIds(src.actorIds);
        entity->setPropertyIds(src.propertyIds);
        entity->setAliases(toAliases(src.aliases));
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        contracts.push_back(std::move(entity));
    }
    state.setContracts(std::move(contracts));

    core::domain::catalog::WorkspaceCatalog::StatementList statements;
    statements.reserve(snapshot.statements.size());
    for (const auto& src : snapshot.statements) {
        auto entity = std::make_shared<core::domain::Statement>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setTransactionIds(src.transactionIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        statements.push_back(std::move(entity));
    }
    state.setStatements(std::move(statements));

    core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
    transactions.reserve(snapshot.transactions.size());
    for (const auto& src : snapshot.transactions) {
        auto entity = std::make_shared<core::domain::Transaction>();
        entity->setId(src.id);
        entity->setName(src.name);
        entity->setBookingDate(src.bookingDate);
        entity->setValuta(src.valuta);
        entity->setAmount(src.amount);
        entity->setStatus(static_cast<core::domain::Transaction::Status>(src.status));
        entity->setContractId(src.contractId);
        entity->setActorId(src.actorId);
        entity->setStatementId(src.statementId);
        entity->setAllocatable(src.allocatable);
        entity->setPropertyIds(src.propertyIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        transactions.push_back(std::move(entity));
    }
    state.setTransactions(std::move(transactions));

    core::domain::catalog::WorkspaceCatalog::AnalysisList analyses;
    analyses.reserve(snapshot.analyses.size());
    for (const auto& src : snapshot.analyses) {
        auto entity = std::make_shared<core::domain::Analysis>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setType(src.type);
        entity->setConfigJson(core::application::analysis::buildAnalysisConfigJson(src.config));
        entity->setFilterSpec(core::domain::FilterSpec(
            core::ports::analysis::buildAnalysisFilterSpec(src.filter)));
        entity->setExportFormat(src.exportFormat);
        entity->setIncludeCalculationAdjustments(src.includeCalculationAdjustments);
        entity->setExportStateJson({});
        entity->setSnapshotTransactionsJson(
            serializeSnapshotTransactionsForDomain(src.snapshotTransactions));
        for (const auto& [key, value] : src.adjustments) {
            entity->setAdjustment(key, value);
        }
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        analyses.push_back(std::move(entity));
    }
    state.setAnalyses(std::move(analyses));

    core::domain::catalog::WorkspaceCatalog::AnnualList annuals;
    annuals.reserve(snapshot.annuals.size());
    for (const auto& src : snapshot.annuals) {
        auto entity = std::make_shared<core::domain::Annual>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setYear(src.year);
        entity->setAnalysisIds(src.analysisIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        annuals.push_back(std::move(entity));
    }
    state.setAnnuals(std::move(annuals));

    return state;
}

} // namespace core::application::workspace
