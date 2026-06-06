/**
 * @file core/src/application/workspace/WorkspaceStateManager.cpp
 * @brief Implements load, save, rehydration, and validation for workspace session state.
 */

#include "core/pch.h"

#include "core/application/workspace/WorkspaceStateManager.h"
#include "core/application/workspace/WorkspaceSessionState.h"

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/ContractType.h"
#include "core/domain/values/EntityName.h"

#include "core/ports/repositories/IActorRepository.h"
#include "core/ports/repositories/IAnalysisRepository.h"
#include "core/ports/repositories/IAnnualRepository.h"
#include "core/ports/repositories/IContractRepository.h"
#include "core/ports/repositories/IImportLogRepository.h"
#include "core/ports/repositories/IExportLogRepository.h"
#include "core/ports/repositories/IPropertyRepository.h"
#include "core/ports/repositories/IStatementRepository.h"
#include "core/ports/repositories/IStatementDraftRepository.h"
#include "core/ports/repositories/ITransactionRepository.h"
#include "core/ports/repositories/ITransactionDraftRepository.h"
#include "core/application/storage/RepositoryBundle.h"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace core::application {

namespace {

/** @brief Loads a collection from a repository when the repository is present. */
template <typename RepoPtr, typename LoadFn>
auto loadCollection(const RepoPtr& repo, LoadFn&& load) {
    using Collection = std::decay_t<decltype(load(*repo))>;
    if (!repo) return Collection{};
    return load(*repo);
} // namespace core::application

/** @brief Upserts a collection into a repository while skipping null items. */
template <typename RepoPtr, typename Collection, typename UpsertFn>
void upsertCollection(const RepoPtr& repo, const Collection& items, UpsertFn&& upsert) {
    if (!repo) return;
    for (const auto& item : items) {
        if (!item) continue;
        upsert(*repo, item);
    }
}

/** @brief Replaces all items in a repository with the provided collection. */
template <typename RepoPtr, typename Collection, typename ClearFn, typename UpsertFn>
void replaceCollection(const RepoPtr& repo, const Collection& items, ClearFn&& clear, UpsertFn&& upsert) {
    if (!repo) return;

    clear(*repo);
    for (const auto& item : items) {
        if (!item) continue;
        upsert(*repo, item);
    }
}

void dedupStrings(std::vector<std::string>& values) {
    std::unordered_set<std::string> seen;
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        if (value.empty()) continue;
        if (!seen.insert(value).second) continue;
        out.push_back(value);
    }
    std::sort(out.begin(), out.end());
    values = std::move(out);
}

void dedupAliases(std::vector<core::domain::Alias>& values) {
    std::unordered_set<std::string> seen;
    std::vector<core::domain::Alias> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        if (value.value().empty()) continue;
        if (!seen.insert(value.value()).second) continue;
        out.push_back(value);
    }
    values = std::move(out);
}

void normalizeAliases(std::vector<core::domain::Alias>& aliases) {
    for (auto& alias : aliases) {
        if (alias.value().empty()) alias.setValue(alias.source());
        if (alias.source().empty()) alias.setSource(alias.value());
    }
}

void projectActorPropertyLinksFromContracts(core::application::workspace::WorkspaceSessionState& document) {
    const auto& contracts = document.catalog.contracts();

    for (auto& actor : document.catalog.actors()) {
        if (!actor) continue;
        std::vector<std::string> contractIds;
        for (const auto& contract : contracts) {
            if (contract && contract->containsActorId(actor->id())) {
                contractIds.push_back(contract->id());
            }
        }
        dedupStrings(contractIds);
        actor->setContractIds(std::move(contractIds));
    }

    for (auto& property : document.catalog.properties()) {
        if (!property) continue;
        std::vector<std::string> contractIds;
        for (const auto& contract : contracts) {
            if (contract && contract->containsPropertyId(property->id())) {
                contractIds.push_back(contract->id());
            }
        }
        dedupStrings(contractIds);
        property->setContractIds(std::move(contractIds));
    }
}

template <typename T>
std::shared_ptr<T> cloneEntity(const std::shared_ptr<T>& entity) {
    return entity;
}

template <typename Dst, typename Src>
void cloneCollection(Dst& dst, const Src& src) {
    dst.reserve(src.size());
    for (const auto& item : src) {
        dst.push_back(cloneEntity(item));
    }
}

void rehydrate(core::application::workspace::WorkspaceSessionState& document) {
    auto& state = document.catalog;
    for (auto& actor : state.actors()) {
        if (!actor) continue;
        auto aliases = actor->aliases();
        dedupAliases(aliases);
        actor->setAliases(std::move(aliases));
    }
    for (auto& property : state.properties()) {
        if (!property) continue;
        auto aliases = property->aliases();
        dedupAliases(aliases);
        property->setAliases(std::move(aliases));
    }
    for (auto& contract : state.contracts()) {
        if (!contract || contract->id().empty()) continue;
        auto aliases = contract->aliases();
        dedupAliases(aliases);
        contract->setAliases(std::move(aliases));
        auto actorIds = contract->actorIds();
        dedupStrings(actorIds);
        contract->setActorIds(std::move(actorIds));
        auto propertyIds = contract->propertyIds();
        dedupStrings(propertyIds);
        contract->setPropertyIds(std::move(propertyIds));
    }
    for (auto& statement : state.statements()) {
        if (!statement) continue;
        auto transactionIds = statement->transactionIds();
        dedupStrings(transactionIds);
        statement->setTransactionIds(std::move(transactionIds));
    }
    for (auto& annual : state.annuals()) {
        if (!annual) continue;
        auto analysisIds = annual->analysisIds();
        dedupStrings(analysisIds);
        annual->setAnalysisIds(std::move(analysisIds));
    }
    projectActorPropertyLinksFromContracts(document);
}

void validate(const core::application::workspace::WorkspaceSessionState& document, bool strict) {
    const auto& state = document.catalog;
    const auto fail = [strict](const std::string& msg) {
        if (strict) throw std::runtime_error(msg);
    };

    for (const auto& contract : state.contracts()) {
        if (!contract) continue;
        if (!core::domain::EntityName::isValid(contract->name())) fail("Contract.name is invalid");
        if (!core::domain::ContractType::isValid(contract->type()) && !contract->type().empty()) fail("Contract.type is invalid");
        if (!contract->hasRelations()) fail("Contract has no relations");
    }

    std::unordered_map<std::string, const core::domain::Statement*> statementById;
    for (const auto& statement : state.statements()) {
        if (statement && !statement->id().empty()) {
            statementById.emplace(statement->id(), statement.get());
        }
    }

    std::unordered_set<std::string> transactionIds;
    for (const auto& transaction : state.transactions()) {
        if (transaction && !core::domain::EntityName::isValid(transaction->name())) fail("Transaction.name is invalid");
        if (transaction && !transaction->id().empty()) transactionIds.insert(transaction->id());
    }

    std::unordered_set<std::string> actorIds;
    std::unordered_set<std::string> contractIds;
    std::unordered_set<std::string> analysisIds;
    for (const auto& actor : state.actors()) {
        if (actor && !actor->id().empty()) actorIds.insert(actor->id());
    }
    for (const auto& contract : state.contracts()) {
        if (contract && !contract->id().empty()) contractIds.insert(contract->id());
    }
    for (const auto& analysis : state.analyses()) {
        if (analysis && !analysis->id().empty()) {
            analysisIds.insert(analysis->id());
            if (!core::domain::EntityName::isValid(analysis->name())) fail("Analysis.name is invalid");
            if (!analysis->hasType() && analysis->hasExportFormat()) fail("Analysis.exportFormat without type");
            if (analysis->hasType() && !analysis->hasExportFormat()) fail("Analysis.type without exportFormat");
            if (analysis->isConfigured() && !analysis->isResultReady()) fail("Analysis configuration is incomplete");
        }
    }

    for (const auto& transaction : state.transactions()) {
        if (!transaction) continue;
        if (transaction->hasActor() && !actorIds.count(transaction->actorId())) fail("Transaction.actorId unresolved");
        if (transaction->hasContract() && !contractIds.count(transaction->contractId())) fail("Transaction.contractId unresolved");
        if (transaction->hasStatement()) {
            const auto it = statementById.find(transaction->statementId());
            if (it == statementById.end()) {
                fail("Transaction.statementId unresolved");
            } else if (!it->second->containsTransaction(transaction->id())) {
                fail("Transaction missing from owning statement");
            }
        }
    }

    for (const auto& annual : state.annuals()) {
        if (!annual) continue;
        if (!core::domain::EntityName::isValid(annual->name())) {
            fail("Annual.name is invalid");
        }
        if (!annual->hasYear()) {
            fail("Annual.year is invalid");
        }
        for (const auto& analysisId : annual->analysisIds()) {
            if (!analysisIds.count(analysisId)) {
                fail("Annual references missing analysis");
            }
        }
    }

    for (const auto& actor : state.actors()) {
        if (actor && !core::domain::EntityName::isValid(actor->name())) {
            fail("Actor.name is invalid");
        }
        if (actor) {
            for (const auto& contractId : actor->contractIds()) {
                if (!contractIds.count(contractId)) {
                    fail("Actor references missing contract");
                }
            }
        }
    }
    for (const auto& property : state.properties()) {
        if (property && !core::domain::EntityName::isValid(property->name())) {
            fail("Property.name is invalid");
        }
        if (property) {
            for (const auto& contractId : property->contractIds()) {
                if (!contractIds.count(contractId)) {
                    fail("Property references missing contract");
                }
            }
        }
    }
    for (const auto& statement : state.statements()) {
        if (statement && !core::domain::EntityName::isValid(statement->name())) {
            fail("Statement.name is invalid");
        }
    }
    for (const auto& statement : state.statements()) {
        if (!statement) continue;
        for (const auto& transactionId : statement->transactionIds()) {
            if (!transactionIds.count(transactionId)) {
                fail("Statement references missing transaction");
            }
        }
    }
}

core::application::workspace::WorkspaceSessionState prepareForSave(const core::application::workspace::WorkspaceSessionState& document) {
    core::application::workspace::WorkspaceSessionState out;
    core::domain::catalog::WorkspaceCatalog::ActorList actors;
    core::domain::catalog::WorkspaceCatalog::PropertyList properties;
    core::domain::catalog::WorkspaceCatalog::ContractList contracts;
    core::domain::catalog::WorkspaceCatalog::StatementList statements;
    core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
    core::domain::catalog::WorkspaceCatalog::AnalysisList analyses;
    core::domain::catalog::WorkspaceCatalog::AnnualList annuals;
    cloneCollection(actors, document.catalog.actors());
    cloneCollection(properties, document.catalog.properties());
    cloneCollection(contracts, document.catalog.contracts());
    cloneCollection(statements, document.catalog.statements());
    cloneCollection(transactions, document.catalog.transactions());
    cloneCollection(analyses, document.catalog.analyses());
    cloneCollection(annuals, document.catalog.annuals());
    out.catalog.setActors(std::move(actors));
    out.catalog.setProperties(std::move(properties));
    out.catalog.setContracts(std::move(contracts));
    out.catalog.setStatements(std::move(statements));
    out.catalog.setTransactions(std::move(transactions));
    out.catalog.setAnalyses(std::move(analyses));
    out.catalog.setAnnuals(std::move(annuals));
    out.workflow = document.workflow;

    for (const auto& actor : out.catalog.actors()) {
        if (actor) {
            auto aliases = actor->aliases();
            normalizeAliases(aliases);
            dedupAliases(aliases);
            actor->setAliases(std::move(aliases));
        }
    }
    for (const auto& property : out.catalog.properties()) {
        if (property) {
            auto aliases = property->aliases();
            normalizeAliases(aliases);
            dedupAliases(aliases);
            property->setAliases(std::move(aliases));
        }
    }
    for (const auto& contract : out.catalog.contracts()) {
        if (!contract) continue;
        auto aliases = contract->aliases();
        normalizeAliases(aliases);
        dedupAliases(aliases);
        contract->setAliases(std::move(aliases));
        auto actorIds = contract->actorIds();
        dedupStrings(actorIds);
        contract->setActorIds(std::move(actorIds));
        auto propertyIds = contract->propertyIds();
        dedupStrings(propertyIds);
        contract->setPropertyIds(std::move(propertyIds));
    }
    for (const auto& statement : out.catalog.statements()) {
        if (!statement) continue;
        auto transactionIds = statement->transactionIds();
        dedupStrings(transactionIds);
        statement->setTransactionIds(std::move(transactionIds));
    }
    for (const auto& annual : out.catalog.annuals()) {
        if (!annual) continue;
        auto analysisIds = annual->analysisIds();
        dedupStrings(analysisIds);
        annual->setAnalysisIds(std::move(analysisIds));
    }

    return out;
}

}

class WorkspaceStateManager::Impl {
public:
    explicit Impl(Repositories repositories)
        : repos(std::move(repositories)) {
    }

    Repositories repos;
    bool strictValidation = false;
};

WorkspaceStateManager::WorkspaceStateManager(Repositories repos)
    : impl_(std::make_unique<Impl>(std::move(repos))) {
}

WorkspaceStateManager::~WorkspaceStateManager() = default;

WorkspaceStateManager::WorkspaceStateManager(WorkspaceStateManager&&) noexcept = default;

WorkspaceStateManager& WorkspaceStateManager::operator=(WorkspaceStateManager&&) noexcept = default;

core::application::workspace::WorkspaceSessionState WorkspaceStateManager::load() {
    core::application::workspace::WorkspaceSessionState document;

    document.catalog.setActors(loadCollection(impl_->repos.actors, [](const auto& repo) { return repo.getActors(); }));
    document.catalog.setProperties(loadCollection(impl_->repos.properties, [](const auto& repo) { return repo.getProperties(); }));
    document.catalog.setContracts(loadCollection(impl_->repos.contracts, [](const auto& repo) { return repo.getContracts(); }));
    document.catalog.setStatements(loadCollection(impl_->repos.statements, [](const auto& repo) { return repo.getStatements(); }));
    document.catalog.setTransactions(loadCollection(impl_->repos.transactions, [](const auto& repo) { return repo.getTransactions(); }));
    document.catalog.setAnalyses(loadCollection(impl_->repos.analyses, [](const auto& repo) { return repo.getAnalyses(); }));
    document.catalog.setAnnuals(loadCollection(impl_->repos.annuals, [](const auto& repo) { return repo.getAnnuals(); }));
    document.workflow.importLogs = loadCollection(impl_->repos.importLogs, [](const auto& repo) { return repo.getImportLogs(); });
    document.workflow.exportLogs = loadCollection(impl_->repos.exportLogs, [](const auto& repo) { return repo.getExportLogs(); });

    document.workflow.statementDrafts = loadCollection(impl_->repos.statementDrafts, [](const auto& repo) { return repo.getStatementDrafts(); });

    if (impl_->repos.transactionDrafts) {
        document.workflow.transactionDrafts = impl_->repos.transactionDrafts->getTransactionDrafts();
    }

    rehydrate(document);
    validate(document, impl_->strictValidation);

    return document;
}

void WorkspaceStateManager::save(const core::application::workspace::WorkspaceSessionState& document) {
    const core::application::workspace::WorkspaceSessionState projected = prepareForSave(document);
    validate(projected, impl_->strictValidation);

    upsertCollection(impl_->repos.actors, projected.catalog.actors(), [](auto& repo, const auto& actor) { repo.upsertActor(actor); });
    upsertCollection(impl_->repos.properties, projected.catalog.properties(), [](auto& repo, const auto& property) { repo.upsertProperty(property); });
    upsertCollection(impl_->repos.contracts, projected.catalog.contracts(), [](auto& repo, const auto& contract) { repo.upsertContract(contract); });
    upsertCollection(impl_->repos.statements, projected.catalog.statements(), [](auto& repo, const auto& statement) { repo.upsertStatement(statement); });
    upsertCollection(impl_->repos.transactions, projected.catalog.transactions(), [](auto& repo, const auto& transaction) { repo.upsertTransaction(transaction); });
    upsertCollection(impl_->repos.analyses, projected.catalog.analyses(), [](auto& repo, const auto& analysis) { repo.upsertAnalysis(analysis); });
    upsertCollection(impl_->repos.annuals, projected.catalog.annuals(), [](auto& repo, const auto& annual) { repo.upsertAnnual(annual); });
    replaceCollection(impl_->repos.importLogs,
                      projected.workflow.importLogs,
                      [](auto& repo) { repo.clearImportLogs(); },
                      [](auto& repo, const auto& item) { repo.upsertImportLog(item); });
    replaceCollection(impl_->repos.exportLogs,
                      projected.workflow.exportLogs,
                      [](auto& repo) { repo.clearExportLogs(); },
                      [](auto& repo, const auto& item) { repo.upsertExportLog(item); });
    replaceCollection(impl_->repos.statementDrafts,
                      projected.workflow.statementDrafts,
                      [](auto& repo) { repo.clearStatementDrafts(); },
                      [](auto& repo, const auto& item) { repo.upsertStatementDraft(item); });
    replaceCollection(impl_->repos.transactionDrafts,
                      projected.workflow.transactionDrafts,
                      [](auto& repo) { repo.clearTransactionDrafts(); },
                      [](auto& repo, const auto& item) { repo.upsertTransactionDraft(item); });
}

void WorkspaceStateManager::setStrictValidation(bool enabled) noexcept {
    impl_->strictValidation = enabled;
}

}
