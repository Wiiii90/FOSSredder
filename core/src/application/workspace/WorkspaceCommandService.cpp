/**
 * @file core/src/application/workspace/WorkspaceCommandService.cpp
 * @brief Implements workspace catalog command orchestration using typed boundary commands.
 */

#include "core/application/workspace/WorkspaceCommandService.h"

#include "core/application/import/transaction/AmountParser.h"
#include "core/application/analysis/AnalysisWorkflowSupport.h"
#include "core/application/workspace/WorkspaceSession.h"

#include "../../utils/StableId.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/policies/AliasPolicy.h"
#include "core/domain/policies/AnalysisPolicy.h"
#include "core/domain/policies/AnnualPolicy.h"
#include "core/domain/policies/TransactionPolicy.h"
#include "core/domain/values/AnalysisType.h"
#include "core/domain/values/BookingDate.h"
#include "core/domain/values/ContractType.h"
#include "core/domain/values/EntityName.h"
#include "core/domain/values/ExportFormat.h"
#include "core/domain/values/FilterSpec.h"
#include "core/domain/values/MoneyAmount.h"
#include "core/domain/values/Year.h"
#include "../../utils/Time.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <optional>
#include <unordered_set>

namespace {

std::vector<core::domain::Alias> toAliases(const std::vector<core::ports::workspace::AliasSnapshot>& aliases) {
    std::vector<core::domain::Alias> out;
    out.reserve(aliases.size());
    for (const auto& alias : aliases) {
        out.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
    }
    return out;
}

template <typename Entity>
concept HasTimestamps = requires(Entity entity) {
    entity.setCreatedAt(std::string{});
    entity.setUpdatedAt(std::string{});
};

template <typename Entity>
void stampCreated(Entity& entity) {
    if constexpr (HasTimestamps<Entity>) {
        const auto now = core::utils::currentTimestampUtc();
        if (entity.createdAt().empty()) {
            entity.setCreatedAt(now);
        }
        if (entity.updatedAt().empty()) {
            entity.setUpdatedAt(entity.createdAt());
        }
    }
}

template <typename Entity>
void stampUpdated(Entity& entity) {
    if constexpr (HasTimestamps<Entity>) {
        entity.setUpdatedAt(core::utils::currentTimestampUtc());
    }
}

template <typename Collection, typename Configure>
std::string appendEntity(Collection& collection, Configure&& configure) {
    using Entity = typename Collection::value_type::element_type;
    auto entity = std::make_shared<Entity>();
    entity->setId(core::utils::makeStableId());
    configure(*entity);
    stampCreated(*entity);
    const std::string id = entity->id();
    collection.push_back(std::move(entity));
    return id;
}

template <typename Collection, typename Update>
bool updateEntity(Collection& collection, const std::string& id, Update&& update) {
    if (id.empty()) {
        return false;
    }
    for (auto& entity : collection) {
        if (!entity || entity->id() != id) {
            continue;
        }
        update(*entity);
        stampUpdated(*entity);
        return true;
    }
    return false;
}

template <typename Collection>
bool eraseEntity(Collection& collection, const std::string& id) {
    if (id.empty()) {
        return false;
    }
    const auto originalSize = collection.size();
    collection.erase(std::remove_if(collection.begin(), collection.end(), [&](const auto& entity) {
        return entity && entity->id() == id;
    }), collection.end());
    return collection.size() != originalSize;
}

void applyActorDraft(core::domain::Actor& actor, const core::application::ActorInput& input) {
    actor.rename(input.name);
    actor.setAliases(input.aliases);
    actor.setContractIds(input.contractIds);
}

void applyPropertyDraft(core::domain::Property& property, const core::application::PropertyInput& input) {
    property.rename(input.name);
    property.setAliases(input.aliases);
    property.setContractIds(input.contractIds);
}

void applyContractDraft(core::domain::Contract& contract, const core::application::ContractInput& input) {
    std::vector<std::string> actorIds;
    actorIds.reserve(1);
    for (const auto& actorId : input.actorIds) {
        const auto normalized = core::domain::policies::alias::trimCopy(actorId);
        if (normalized.empty()) {
            continue;
        }
        actorIds.push_back(normalized);
        break;
    }
    contract.rename(input.name);
    contract.setType(input.type);
    contract.setAllocatableMode(input.allocatableMode);
    contract.setActorIds(actorIds);
    contract.setPropertyIds(input.propertyIds);
    contract.setAliases(input.aliases);
}

void applyStatementName(core::domain::Statement& statement, const std::string& name) {
    statement.rename(name);
}

std::string serializeSnapshotTransactionsForDomain(
    const std::vector<core::ports::workspace::TransactionSnapshot>& source);
core::domain::FilterSpec analysisFilterSpecFromSelection(
    const core::ports::analysis::AnalysisFilterSelection& selection);

void applyAnalysisDraft(core::domain::Analysis& analysis, const core::application::AnalysisInput& input) {
    analysis.rename(input.name);
    analysis.setType(input.type);
    analysis.setConfigJson(core::application::analysis::buildAnalysisConfigJson(input.config));
    analysis.setFilterSpec(analysisFilterSpecFromSelection(input.filter));
    analysis.setExportFormat(input.exportFormat);
    analysis.setIncludeCalculationAdjustments(input.includeCalculationAdjustments);
    analysis.setExportStateJson({});
    analysis.setSnapshotTransactionsJson(
        serializeSnapshotTransactionsForDomain(input.snapshotTransactions));
    analysis.clearAdjustments();
    for (const auto& [key, value] : input.adjustments) {
        analysis.setAdjustment(key, value);
    }
}

void applyAnnualDraft(core::domain::Annual& annual,
                      const std::string& name,
                      int year,
                      const std::vector<std::string>& analysisIds) {
    annual.apply(name, year, analysisIds);
}

void applyTransactionDraft(core::domain::Transaction& tx, const core::application::TransactionInput& input) {
    tx.setName(input.name);
    tx.setBookingDate(input.bookingDate.value());
    tx.setValuta(input.valuta);
    tx.setAmount(input.amount);
    tx.setStatementId(input.statementId);
    tx.setStatus(input.status);
    tx.setActorId(input.actorId);
    tx.setContractId(input.contractId);
    tx.setAllocatable(input.allocatable);
    tx.setPropertyIds(input.propertyIds);
}

std::vector<std::pair<std::string, double>>
analysisAdjustmentsFromCommand(
    const core::ports::workspace::AnalysisCommand& command) {
    return command.adjustments;
}

core::domain::FilterSpec analysisFilterSpecFromSelection(
    const core::ports::analysis::AnalysisFilterSelection& selection) {
    return core::domain::FilterSpec(
        core::ports::analysis::buildAnalysisFilterSpec(selection));
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

bool isBlank(const std::string& value) {
    return core::domain::policies::alias::trimCopy(value).empty();
}

void requireText(core::ports::workspace::ValidationResult& result,
                 const std::string& field,
                 const std::string& value,
                 const std::string& label) {
    if (isBlank(value)) {
        result.addError(field, "required", label + " is required.");
    }
}

void requireEntityName(core::ports::workspace::ValidationResult& result,
                       const std::string& field,
                       const std::string& value,
                       const std::string& label) {
    requireText(result, field, value, label);
    if (!isBlank(value) && !core::domain::EntityName::isValid(value)) {
        result.addError(field, "invalid", label + " is invalid.");
    }
}

void requireId(core::ports::workspace::ValidationResult& result,
               const std::string& field,
               const std::string& value,
               const std::string& label) {
    if (isBlank(value)) {
        result.addError(field, "required", label + " is required.");
    }
}

bool valid(const core::ports::workspace::ValidationResult& result) {
    return result.valid();
}

void appendValidationIssues(core::ports::workspace::ValidationResult& target,
                            const core::ports::workspace::ValidationResult& source) {
    target.issues.insert(target.issues.end(), source.issues.begin(), source.issues.end());
}

std::optional<double> parseTransactionAmount(const core::ports::workspace::TransactionCommand& command) {
    return core::application::importing::transaction::parseAmountString(command.amountText);
}

core::ports::workspace::ValidationResult validateTransactionCommand(
    const core::ports::workspace::TransactionCommand& command,
    bool requireStatementId) {
    core::ports::workspace::ValidationResult result;
    requireText(result, "bookingDate", command.bookingDate, "Booking date");
    if (!isBlank(command.bookingDate) && !core::domain::policies::transaction::hasValidBookingDate(command.bookingDate)) {
        result.addError("bookingDate", "invalid", "Booking date is invalid.");
    }
    const auto amount = parseTransactionAmount(command);
    if (!amount || !core::domain::policies::transaction::hasValidAmount(*amount)) {
        result.addError("amount", "invalid", "Amount is invalid.");
    }
    if (requireStatementId) {
        requireId(result, "statementId", command.statementId, "Statement");
    }
    return result;
}

core::application::TransactionInput transactionInputFromCommand(
    const core::ports::workspace::TransactionCommand& command,
    double amount) {
    core::application::TransactionInput input;
    input.name = command.name;
    input.bookingDate = core::domain::BookingDate(command.bookingDate);
    input.valuta = command.valuta;
    input.amount = amount;
    input.statementId = command.statementId;
    input.insertAfterTransactionId = command.insertAfterTransactionId;
    input.status = static_cast<core::domain::Transaction::Status>(command.status);
    input.actorId = command.actorId;
    input.contractId = command.contractId;
    input.allocatable = command.allocatable;
    input.propertyIds = command.propertyIds;
    return input;
}

bool syncActorRelations(core::domain::catalog::WorkspaceCatalog& state,
                        const std::string& actorId,
                        const std::vector<std::string>& desiredContractIds) {
    if (actorId.empty()) {
        return false;
    }

    std::unordered_set<std::string> desired;
    for (const auto& contractId : desiredContractIds) {
        if (!contractId.empty()) {
            desired.insert(contractId);
        }
    }

    bool changed = false;
    for (auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        const bool shouldLink = desired.find(contract->id()) != desired.end();
        const bool hasLink = contract->containsActorId(actorId);
        if (shouldLink) {
            const auto& actorIds = contract->actorIds();
            const bool alreadySingleOwner = actorIds.size() == 1u && actorIds.front() == actorId;
            if (!alreadySingleOwner) {
                contract->setActorIds({actorId});
                stampUpdated(*contract);
                changed = true;
            }
        } else if (!shouldLink && hasLink) {
            contract->removeActorId(actorId);
            stampUpdated(*contract);
            changed = true;
        }
    }

    return changed;
}

bool syncPropertyRelations(core::domain::catalog::WorkspaceCatalog& state,
                           const std::string& propertyId,
                           const std::vector<std::string>& desiredContractIds) {
    if (propertyId.empty()) {
        return false;
    }

    std::unordered_set<std::string> desired;
    for (const auto& contractId : desiredContractIds) {
        if (!contractId.empty()) {
            desired.insert(contractId);
        }
    }

    bool changed = false;
    for (auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        const bool shouldLink = desired.find(contract->id()) != desired.end();
        const bool hasLink = contract->containsPropertyId(propertyId);
        if (shouldLink && !hasLink) {
            contract->addPropertyId(propertyId);
            stampUpdated(*contract);
            changed = true;
        } else if (!shouldLink && hasLink) {
            contract->removePropertyId(propertyId);
            stampUpdated(*contract);
            changed = true;
        }
    }

    return changed;
}

void projectActorPropertyLinksFromContracts(core::domain::catalog::WorkspaceCatalog& state) {
    const auto& contracts = state.contracts();

    for (auto& actor : state.actors()) {
        if (!actor) continue;
        std::vector<std::string> contractIds;
        for (const auto& contract : contracts) {
            if (contract && contract->containsActorId(actor->id())) {
                contractIds.push_back(contract->id());
            }
        }
        actor->setContractIds(std::move(contractIds));
    }

    for (auto& property : state.properties()) {
        if (!property) continue;
        std::vector<std::string> contractIds;
        for (const auto& contract : contracts) {
            if (contract && contract->containsPropertyId(property->id())) {
                contractIds.push_back(contract->id());
            }
        }
        property->setContractIds(std::move(contractIds));
    }
}

template <typename T>
std::shared_ptr<T> findById(const std::vector<std::shared_ptr<T>>& items, const std::string& id) {
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && item->id() == id;
    });
    return it == items.end() ? nullptr : *it;
}

void removeTransaction(core::domain::catalog::WorkspaceCatalog::StatementList& statements,
                       core::domain::catalog::WorkspaceCatalog::TransactionList& transactions,
                       const std::string& id);

void removeStatement(core::domain::catalog::WorkspaceCatalog::StatementList& statements,
                     core::domain::catalog::WorkspaceCatalog::TransactionList& transactions,
                     const std::string& id) {
    const auto statement = findById(statements, id);
    if (!statement) {
        return;
    }

    for (const auto& transactionId : statement->transactionIds()) {
        removeTransaction(statements, transactions, transactionId);
    }

    statements.erase(std::remove_if(statements.begin(), statements.end(), [&](const auto& item) {
        return item && item->id() == id;
    }), statements.end());
}

void removeTransaction(core::domain::catalog::WorkspaceCatalog::StatementList& statements,
                       core::domain::catalog::WorkspaceCatalog::TransactionList& transactions,
                       const std::string& id) {
    for (auto& statement : statements) {
        if (statement) {
            statement->removeTransaction(id);
        }
    }

    transactions.erase(std::remove_if(transactions.begin(), transactions.end(), [&](const auto& item) {
        return item && item->id() == id;
    }), transactions.end());
}

void linkTransactionToStatement(core::domain::catalog::WorkspaceCatalog::StatementList& statements,
                                const std::string& statementId,
                                const std::string& transactionId) {
    const auto statement = findById(statements, statementId);
    if (!statement) {
        return;
    }
    statement->addTransaction(transactionId);
}

void unlinkTransactionFromStatement(core::domain::catalog::WorkspaceCatalog::StatementList& statements,
                                    const std::string& statementId,
                                    const std::string& transactionId) {
    const auto statement = findById(statements, statementId);
    if (!statement) {
        return;
    }
    statement->removeTransaction(transactionId);
}

class WorkspaceCatalogMutator {
public:
    std::string addActor(core::domain::catalog::WorkspaceCatalog& state, const core::application::ActorInput& input) const {
        if (!core::domain::EntityName::isValid(input.name)) {
            return {};
        }
        auto actors = state.actors();
        const auto id = appendEntity(actors, [&](core::domain::Actor& actor) { applyActorDraft(actor, input); });
        state.setActors(std::move(actors));
        syncActorRelations(state, id, input.contractIds);
        projectActorPropertyLinksFromContracts(state);
        return id;
    }

    bool updateActor(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const core::application::ActorInput& input) const {
        if (!core::domain::EntityName::isValid(input.name)) {
            return false;
        }
        auto actors = state.actors();
        const bool updated = updateEntity(actors, id, [&](core::domain::Actor& actor) { applyActorDraft(actor, input); });
        state.setActors(std::move(actors));
        if (updated) {
            syncActorRelations(state, id, input.contractIds);
            projectActorPropertyLinksFromContracts(state);
        }
        return updated;
    }

    bool deleteActor(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto actors = state.actors();
        const bool removed = eraseEntity(actors, id);
        state.setActors(std::move(actors));
        if (removed) {
            syncActorRelations(state, id, std::vector<std::string>{});
            projectActorPropertyLinksFromContracts(state);
        }
        return removed;
    }

    std::string addProperty(core::domain::catalog::WorkspaceCatalog& state, const core::application::PropertyInput& input) const {
        if (!core::domain::EntityName::isValid(input.name)) {
            return {};
        }
        auto properties = state.properties();
        const auto id = appendEntity(properties, [&](core::domain::Property& property) { applyPropertyDraft(property, input); });
        state.setProperties(std::move(properties));
        syncPropertyRelations(state, id, input.contractIds);
        projectActorPropertyLinksFromContracts(state);
        return id;
    }

    bool updateProperty(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const core::application::PropertyInput& input) const {
        if (!core::domain::EntityName::isValid(input.name)) {
            return false;
        }
        auto properties = state.properties();
        const bool updated = updateEntity(properties, id, [&](core::domain::Property& property) { applyPropertyDraft(property, input); });
        state.setProperties(std::move(properties));
        if (updated) {
            syncPropertyRelations(state, id, input.contractIds);
            projectActorPropertyLinksFromContracts(state);
        }
        return updated;
    }

    bool deleteProperty(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto properties = state.properties();
        const bool removed = eraseEntity(properties, id);
        state.setProperties(std::move(properties));
        if (removed) {
            syncPropertyRelations(state, id, std::vector<std::string>{});
            projectActorPropertyLinksFromContracts(state);
        }
        return removed;
    }

    std::string addContract(core::domain::catalog::WorkspaceCatalog& state, const core::application::ContractInput& input) const {
        if (!core::domain::EntityName::isValid(input.name) || !core::domain::ContractType::isValid(input.type)) {
            return {};
        }
        auto contracts = state.contracts();
        const auto id = appendEntity(contracts, [&](core::domain::Contract& contract) { applyContractDraft(contract, input); });
        state.setContracts(std::move(contracts));
        projectActorPropertyLinksFromContracts(state);
        return id;
    }

    bool updateContract(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const core::application::ContractInput& input) const {
        if (!core::domain::EntityName::isValid(input.name) || !core::domain::ContractType::isValid(input.type)) {
            return false;
        }
        auto contracts = state.contracts();
        const bool updated = updateEntity(contracts, id, [&](core::domain::Contract& contract) { applyContractDraft(contract, input); });
        state.setContracts(std::move(contracts));
        if (updated) {
            projectActorPropertyLinksFromContracts(state);
        }
        return updated;
    }

    bool deleteContract(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto contracts = state.contracts();
        const bool removed = eraseEntity(contracts, id);
        state.setContracts(std::move(contracts));
        if (removed) {
            projectActorPropertyLinksFromContracts(state);
        }
        return removed;
    }

    std::string addStatement(core::domain::catalog::WorkspaceCatalog& state, const std::string& name) const {
        if (!core::domain::EntityName::isValid(name)) {
            return {};
        }
        auto statements = state.statements();
        const auto id = appendEntity(statements, [&](core::domain::Statement& statement) { applyStatementName(statement, name); });
        state.setStatements(std::move(statements));
        return id;
    }

    bool updateStatement(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const std::string& name) const {
        if (!core::domain::EntityName::isValid(name)) {
            return false;
        }
        auto statements = state.statements();
        const bool updated = updateEntity(statements, id, [&](core::domain::Statement& statement) { applyStatementName(statement, name); });
        state.setStatements(std::move(statements));
        return updated;
    }

    bool deleteStatement(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        if (id.empty()) {
            return false;
        }

        auto statements = state.statements();
        auto transactions = state.transactions();
        const bool hadStatement = static_cast<bool>(findById(statements, id));
        const auto originalTransactionCount = transactions.size();
        removeStatement(statements, transactions, id);
        state.setStatements(std::move(statements));
        state.setTransactions(std::move(transactions));
        return hadStatement || state.transactions().size() != originalTransactionCount;
    }

    std::string addTransaction(core::domain::catalog::WorkspaceCatalog& state, const core::application::TransactionInput& input) const {
        if (isBlank(input.statementId) || input.bookingDate.empty() || !core::domain::MoneyAmount::isValid(input.amount)) {
            return {};
        }

        auto statements = state.statements();
        auto transactions = state.transactions();
        if (!findById(statements, input.statementId)) {
            return {};
        }

        const auto id = appendEntity(transactions, [&](core::domain::Transaction& tx) {
            applyTransactionDraft(tx, input);
        });

        const auto statement = findById(statements, input.statementId);
        if (statement && !input.insertAfterTransactionId.empty()
                && statement->containsTransaction(input.insertAfterTransactionId)) {
            statement->insertTransaction(id, statement->indexOfTransaction(input.insertAfterTransactionId) + 1);
        } else {
            linkTransactionToStatement(statements, input.statementId, id);
        }
        state.setStatements(std::move(statements));
        state.setTransactions(std::move(transactions));

        return id;
    }

    bool updateTransaction(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const core::application::TransactionInput& input) const {
        if (input.bookingDate.empty() || !core::domain::MoneyAmount::isValid(input.amount)) {
            return false;
        }

        if (!input.statementId.empty()) {
            if (!findById(state.statements(), input.statementId)) {
                return false;
            }
        }

        std::string previousStatementId;
        std::string previousContractId;
        bool previousAllocatable = false;
        auto statements = state.statements();
        auto transactions = state.transactions();
        if (const auto transaction = findById(transactions, id)) {
            previousStatementId = transaction->statementId();
            previousContractId = transaction->contractId();
            previousAllocatable = transaction->isAllocatable();
        }

        const auto updated = updateEntity(transactions, id, [&](core::domain::Transaction& tx) {
            applyTransactionDraft(tx, input);
        });

        if (!updated) {
            return false;
        }

        if (const auto updatedTx = findById(transactions, id)) {
            const bool allocatableChanged = updatedTx->isAllocatable() != previousAllocatable;
            const std::string effectiveContractId = !updatedTx->contractId().empty() ? updatedTx->contractId() : previousContractId;
            if (allocatableChanged && !effectiveContractId.empty()) {
                auto contracts = state.contracts();
                const bool contractModeUpdated = updateEntity(contracts, effectiveContractId, [](core::domain::Contract& contract) {
                    if (contract.allocatableMode() != "mixed") {
                        contract.setAllocatableMode("mixed");
                        stampUpdated(contract);
                    }
                });
                if (contractModeUpdated) {
                    state.setContracts(std::move(contracts));
                }
            }
        }

        if (previousStatementId != input.statementId) {
            if (!previousStatementId.empty()) {
                unlinkTransactionFromStatement(statements, previousStatementId, id);
            }
            if (!input.statementId.empty()) {
                linkTransactionToStatement(statements, input.statementId, id);
            }
        } else if (!input.statementId.empty()) {
            linkTransactionToStatement(statements, input.statementId, id);
        }

        state.setStatements(std::move(statements));
        state.setTransactions(std::move(transactions));
        return true;
    }

    bool deleteTransaction(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto statements = state.statements();
        auto transactions = state.transactions();
        const auto originalSize = transactions.size();
        removeTransaction(statements, transactions, id);
        state.setStatements(std::move(statements));
        state.setTransactions(std::move(transactions));
        return state.transactions().size() != originalSize;
    }

    std::string addAnalysis(core::domain::catalog::WorkspaceCatalog& state, const core::application::AnalysisInput& input) const {
        const auto filterSpec = analysisFilterSpecFromSelection(input.filter);
        if (!core::domain::EntityName::isValid(input.name) || !core::domain::AnalysisType::isValid(input.type.value()) ||
            !core::domain::FilterSpec::isValid(filterSpec.value()) || !core::domain::ExportFormat::isValid(input.exportFormat.value())) {
            return {};
        }
        auto analyses = state.analyses();
        const auto id = appendEntity(analyses, [&](core::domain::Analysis& analysis) { applyAnalysisDraft(analysis, input); });
        state.setAnalyses(std::move(analyses));
        return id;
    }

    bool updateAnalysis(core::domain::catalog::WorkspaceCatalog& state, const std::string& id, const core::application::AnalysisInput& input) const {
        const auto filterSpec = analysisFilterSpecFromSelection(input.filter);
        if (!core::domain::EntityName::isValid(input.name) || !core::domain::AnalysisType::isValid(input.type.value()) ||
            !core::domain::FilterSpec::isValid(filterSpec.value()) || !core::domain::ExportFormat::isValid(input.exportFormat.value())) {
            return false;
        }
        auto analyses = state.analyses();
        const bool updated = updateEntity(analyses, id, [&](core::domain::Analysis& analysis) { applyAnalysisDraft(analysis, input); });
        state.setAnalyses(std::move(analyses));
        return updated;
    }

    bool deleteAnalysis(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto analyses = state.analyses();
        const bool removed = eraseEntity(analyses, id);
        state.setAnalyses(std::move(analyses));
        return removed;
    }

    std::string addAnnual(core::domain::catalog::WorkspaceCatalog& state,
                          const std::string& name,
                          int year,
                          const std::vector<std::string>& analysisIds) const {
        if (!core::domain::EntityName::isValid(name) || !core::domain::Year::isValid(year) ||
            !core::domain::policies::annual::hasUniqueAnalysisIds(analysisIds)) {
            return {};
        }
        auto annuals = state.annuals();
        const auto id = appendEntity(annuals, [&](core::domain::Annual& annual) { applyAnnualDraft(annual, name, year, analysisIds); });
        state.setAnnuals(std::move(annuals));
        return id;
    }

    bool updateAnnual(core::domain::catalog::WorkspaceCatalog& state,
                      const std::string& id,
                      const std::string& name,
                      int year,
                      const std::vector<std::string>& analysisIds) const {
        if (!core::domain::EntityName::isValid(name) || !core::domain::Year::isValid(year) ||
            !core::domain::policies::annual::hasUniqueAnalysisIds(analysisIds)) {
            return false;
        }
        auto annuals = state.annuals();
        const bool updated = updateEntity(annuals, id, [&](core::domain::Annual& annual) { applyAnnualDraft(annual, name, year, analysisIds); });
        state.setAnnuals(std::move(annuals));
        return updated;
    }

    bool deleteAnnual(core::domain::catalog::WorkspaceCatalog& state, const std::string& id) const {
        auto annuals = state.annuals();
        const bool removed = eraseEntity(annuals, id);
        state.setAnnuals(std::move(annuals));
        return removed;
    }
};

const WorkspaceCatalogMutator& catalogMutator() {
    static const WorkspaceCatalogMutator instance;
    return instance;
}

} // namespace

namespace core::application {

WorkspaceCommandService::WorkspaceCommandService(WorkspaceSession& session)
    : session_(&session) {
}

WorkspaceCommandService::~WorkspaceCommandService() = default;

WorkspaceCommandService::WorkspaceCommandService(WorkspaceCommandService&&) noexcept = default;

WorkspaceCommandService& WorkspaceCommandService::operator=(WorkspaceCommandService&&) noexcept = default;

core::domain::catalog::WorkspaceCatalog& WorkspaceCommandService::mutableCatalogState() noexcept {
    return session_->mutableCatalogState();
}

const core::domain::catalog::WorkspaceCatalog& WorkspaceCommandService::catalogState() const noexcept {
    return session_->catalogState();
}

void WorkspaceCommandService::commit() {
    session_->commit();
}

void WorkspaceCommandService::notifyCatalogChanged() {
    session_->notifyState();
}

std::string WorkspaceCommandService::commitCreated(WorkspaceCommandService& service, std::string id) {
    if (id.empty()) {
        return {};
    }

    service.notifyCatalogChanged();
    return id;
}

void WorkspaceCommandService::commitIfChanged(WorkspaceCommandService& service, bool changed) {
    if (changed) {
        service.notifyCatalogChanged();
    }
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::ActorCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Actor name");
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::PropertyCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Property name");
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::ContractCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Contract name");
    requireText(result, "type", command.type, "Contract type");
    if (!isBlank(command.type) && !core::domain::ContractType::isValid(command.type)) {
        result.addError("type", "invalid", "Contract type is invalid.");
    }
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::StatementCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Statement name");
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::TransactionCommand& command) const {
    return validateTransactionCommand(command, true);
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(
    const core::ports::workspace::StatementWithTransactionsCommand& command) const {
    auto result = validate(command.statement);
    for (const auto& transaction : command.transactions) {
        appendValidationIssues(result, validateTransactionCommand(transaction, false));
    }
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::AnalysisCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Analysis name");
    requireText(result, "type", command.type, "Analysis type");
    if (!isBlank(command.type) && !core::domain::AnalysisType::isValid(command.type)) {
        result.addError("type", "invalid", "Analysis type is invalid.");
    }
    if (!isBlank(command.type) && !core::domain::policies::analysis::supportsResultType(command.type)) {
        result.addError("type", "unsupported", "Analysis type is not supported.");
    }
    const auto filterSpec = analysisFilterSpecFromSelection(command.filter);
    if (!filterSpec.value().empty() && !core::domain::FilterSpec::isValid(filterSpec.value())) {
        result.addError("filterSpec", "invalid", "Filter specification is invalid.");
    }
    if (!isBlank(command.exportFormat) && !core::domain::ExportFormat::isValid(command.exportFormat)) {
        result.addError("exportFormat", "invalid", "Export format is invalid.");
    }
    return result;
}

core::ports::workspace::ValidationResult WorkspaceCommandService::validate(const core::ports::workspace::AnnualCommand& command) const {
    core::ports::workspace::ValidationResult result;
    requireEntityName(result, "name", command.name, "Annual name");
    if (!core::domain::policies::annual::isValidYear(command.year)) {
        result.addError("year", "invalid", "Year is invalid.");
    }
    return result;
}

std::string WorkspaceCommandService::addActor(const core::ports::workspace::ActorCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addActor(mutableCatalogState(), {command.name, toAliases(command.aliases), command.contractIds}));
}

void WorkspaceCommandService::updateActor(const core::ports::workspace::ActorCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Actor");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateActor(mutableCatalogState(), command.id, {command.name, toAliases(command.aliases), command.contractIds}));
}

void WorkspaceCommandService::deleteActor(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteActor(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addProperty(const core::ports::workspace::PropertyCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addProperty(mutableCatalogState(), {command.name, toAliases(command.aliases), command.contractIds}));
}

void WorkspaceCommandService::updateProperty(const core::ports::workspace::PropertyCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Property");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateProperty(mutableCatalogState(), command.id, {command.name, toAliases(command.aliases), command.contractIds}));
}

void WorkspaceCommandService::deleteProperty(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteProperty(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addContract(const core::ports::workspace::ContractCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addContract(mutableCatalogState(), {
        command.name,
        command.type,
        command.allocatableMode,
        command.actorIds,
        command.propertyIds,
        toAliases(command.aliases)
    }));
}

void WorkspaceCommandService::updateContract(const core::ports::workspace::ContractCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Contract");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateContract(mutableCatalogState(),
                                                                                     command.id,
                                                                                     {command.name,
                                                                                      command.type,
                                                                                      command.allocatableMode,
                                                                                      command.actorIds,
                                                                                      command.propertyIds,
                                                                                      toAliases(command.aliases)}));
}

void WorkspaceCommandService::deleteContract(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteContract(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addStatement(const core::ports::workspace::StatementCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addStatement(mutableCatalogState(), command.name));
}

std::string WorkspaceCommandService::addStatementWithTransactions(
    const core::ports::workspace::StatementWithTransactionsCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }

    std::vector<TransactionInput> inputs;
    inputs.reserve(command.transactions.size());
    for (const auto& transaction : command.transactions) {
        if (!valid(validateTransactionCommand(transaction, false))) {
            return {};
        }
        const auto amount = parseTransactionAmount(transaction);
        if (!amount) {
            return {};
        }
        inputs.push_back(transactionInputFromCommand(transaction, *amount));
    }

    auto& state = mutableCatalogState();
    const std::string statementId = catalogMutator().addStatement(state, command.statement.name);
    if (statementId.empty()) {
        return {};
    }
    for (auto& input : inputs) {
        input.statementId = statementId;
        static_cast<void>(catalogMutator().addTransaction(state, input));
    }
    notifyCatalogChanged();
    return statementId;
}

void WorkspaceCommandService::updateStatement(const core::ports::workspace::StatementCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Statement");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateStatement(mutableCatalogState(), command.id, command.name));
}

void WorkspaceCommandService::deleteStatement(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteStatement(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addTransaction(const core::ports::workspace::TransactionCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    const auto amount = parseTransactionAmount(command);
    if (!amount) {
        return {};
    }
    TransactionInput input = transactionInputFromCommand(command, *amount);
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addTransaction(mutableCatalogState(), input));
}

void WorkspaceCommandService::updateTransaction(const core::ports::workspace::TransactionCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Transaction");
    if (!valid(validation)) {
        return;
    }
    const auto amount = parseTransactionAmount(command);
    if (!amount) {
        return;
    }
    TransactionInput input = transactionInputFromCommand(command, *amount);
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateTransaction(mutableCatalogState(), command.id, input));
}

void WorkspaceCommandService::deleteTransaction(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteTransaction(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addAnalysis(const core::ports::workspace::AnalysisCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addAnalysis(mutableCatalogState(), {
        command.name,
        core::domain::AnalysisType(command.type),
        command.config,
        command.filter,
        core::domain::ExportFormat(command.exportFormat),
        command.includeCalculationAdjustments,
        command.snapshotTransactions,
        analysisAdjustmentsFromCommand(command)
    }));
}

void WorkspaceCommandService::updateAnalysis(const core::ports::workspace::AnalysisCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Analysis");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateAnalysis(mutableCatalogState(), command.id, {
        command.name,
        core::domain::AnalysisType(command.type),
        command.config,
        command.filter,
        core::domain::ExportFormat(command.exportFormat),
        command.includeCalculationAdjustments,
        command.snapshotTransactions,
        analysisAdjustmentsFromCommand(command)
    }));
}

void WorkspaceCommandService::deleteAnalysis(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteAnalysis(mutableCatalogState(), id));
}

std::string WorkspaceCommandService::addAnnual(const core::ports::workspace::AnnualCommand& command) {
    if (!valid(validate(command))) {
        return {};
    }
    return WorkspaceCommandService::commitCreated(*this, catalogMutator().addAnnual(mutableCatalogState(), command.name, command.year, command.assignedAnalysisIds));
}

void WorkspaceCommandService::updateAnnual(const core::ports::workspace::AnnualCommand& command) {
    auto validation = validate(command);
    requireId(validation, "id", command.id, "Annual");
    if (!valid(validation)) {
        return;
    }
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().updateAnnual(mutableCatalogState(), command.id, command.name, command.year, command.assignedAnalysisIds));
}

void WorkspaceCommandService::deleteAnnual(const std::string& id) {
    WorkspaceCommandService::commitIfChanged(*this, catalogMutator().deleteAnnual(mutableCatalogState(), id));
}

} // namespace core::application
