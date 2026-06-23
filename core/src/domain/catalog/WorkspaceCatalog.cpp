/**
 * @file core/src/domain/catalog/WorkspaceCatalog.cpp
 * @brief Implements the domain workspace catalog aggregate.
 */

#include "core/domain/catalog/WorkspaceCatalog.h"

#include <utility>

namespace core::domain::catalog {

WorkspaceCatalog::WorkspaceCatalog() = default;

const WorkspaceCatalog::PropertyList& WorkspaceCatalog::properties() const noexcept { return properties_; }
void WorkspaceCatalog::setProperties(PropertyList value) { properties_ = std::move(value); }

const WorkspaceCatalog::ActorList& WorkspaceCatalog::actors() const noexcept { return actors_; }
void WorkspaceCatalog::setActors(ActorList value) { actors_ = std::move(value); }

const WorkspaceCatalog::ContractList& WorkspaceCatalog::contracts() const noexcept { return contracts_; }
void WorkspaceCatalog::setContracts(ContractList value) { contracts_ = std::move(value); }

const WorkspaceCatalog::StatementList& WorkspaceCatalog::statements() const noexcept { return statements_; }
void WorkspaceCatalog::setStatements(StatementList value) { statements_ = std::move(value); }

const WorkspaceCatalog::TransactionList& WorkspaceCatalog::transactions() const noexcept { return transactions_; }
void WorkspaceCatalog::setTransactions(TransactionList value) { transactions_ = std::move(value); }

const WorkspaceCatalog::AnalysisList& WorkspaceCatalog::analyses() const noexcept { return analyses_; }
void WorkspaceCatalog::setAnalyses(AnalysisList value) { analyses_ = std::move(value); }

const WorkspaceCatalog::AnnualList& WorkspaceCatalog::annuals() const noexcept { return annuals_; }
void WorkspaceCatalog::setAnnuals(AnnualList value) { annuals_ = std::move(value); }

bool WorkspaceCatalog::empty() const noexcept {
    return properties_.empty() && actors_.empty() && contracts_.empty() &&
           statements_.empty() && transactions_.empty() && analyses_.empty() &&
           annuals_.empty();
}

} // namespace core::domain::catalog
