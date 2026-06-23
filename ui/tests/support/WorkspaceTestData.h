/**
 * @file ui/tests/support/WorkspaceTestData.h
 * @brief Shared port-only workspace test data builders for UI tests.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::tests::support {

inline core::ports::workspace::AliasSnapshot
makeAlias(const std::string& value, const std::string& kind = {},
          const std::string& source = {}) {
  return {value, kind, source.empty() ? value : source, 0, {}, {}, {}};
}

inline core::ports::workspace::ActorSnapshot makeActor() {
  core::ports::workspace::ActorSnapshot actor;
  actor.id = "actor-1";
  actor.name = "Main Actor";
  actor.aliases = {makeAlias("Primary Actor"), makeAlias("Main Actor")};
  actor.contractIds = {"contract-1"};
  actor.createdAt = "2026-01-01T08:00:00Z";
  actor.updatedAt = "2026-01-02T08:00:00Z";
  return actor;
}

inline core::ports::workspace::PropertySnapshot makeProperty() {
  core::ports::workspace::PropertySnapshot property;
  property.id = "property-1";
  property.name = "Primary Property";
  property.aliases = {makeAlias("Property Alias")};
  property.contractIds = {"contract-1"};
  property.createdAt = "2026-01-01T08:00:00Z";
  property.updatedAt = "2026-01-02T08:00:00Z";
  return property;
}

inline core::ports::workspace::ContractSnapshot makeContract() {
  core::ports::workspace::ContractSnapshot contract;
  contract.id = "contract-1";
  contract.name = "Lease Contract";
  contract.type = "lease";
  contract.allocatableMode = "mixed";
  contract.actorIds = {"actor-1"};
  contract.propertyIds = {"property-1"};
  contract.aliases = {makeAlias("Lease")};
  contract.createdAt = "2026-01-01T08:00:00Z";
  contract.updatedAt = "2026-01-02T08:00:00Z";
  return contract;
}

inline core::ports::workspace::StatementSnapshot makeStatement() {
  core::ports::workspace::StatementSnapshot statement;
  statement.id = "statement-1";
  statement.name = "January Statement";
  statement.transactionIds = {"tx-1", "tx-2"};
  statement.createdAt = "2026-01-01T08:00:00Z";
  statement.updatedAt = "2026-01-02T08:00:00Z";
  return statement;
}

inline core::ports::workspace::TransactionSnapshot
makeTransaction(const std::string& id, const std::string& name,
                const std::string& bookingDate, double amount,
                const std::string& statementId, bool allocatable,
                const std::string& contractId = "contract-1",
                const std::string& actorId = "actor-1",
                std::vector<std::string> propertyIds = {"property-1"},
                int status = 2) {
  core::ports::workspace::TransactionSnapshot transaction;
  transaction.id = id;
  transaction.name = name;
  transaction.bookingDate = bookingDate;
  transaction.valuta = "EUR";
  transaction.amount = amount;
  transaction.status = status;
  transaction.contractId = contractId;
  transaction.actorId = actorId;
  transaction.statementId = statementId;
  transaction.allocatable = allocatable;
  transaction.propertyIds = std::move(propertyIds);
  transaction.createdAt = "2026-01-01T08:00:00Z";
  transaction.updatedAt = "2026-01-02T08:00:00Z";
  return transaction;
}

inline core::ports::workspace::AnalysisSnapshot makeAnalysis() {
  core::ports::workspace::AnalysisSnapshot analysis;
  analysis.id = "analysis-1";
  analysis.name = "Monthly Analysis";
  analysis.type = "tabular";
  analysis.config.type = "tabular";
  analysis.filter.dateMode = "year";
  analysis.filter.year = "2026";
  analysis.exportFormat = "csv";
  analysis.includeCalculationAdjustments = true;
  analysis.snapshotTransactions = {
      makeTransaction("tx-1", "Income", "2026-01-15", 1250.0, "statement-1",
                      true, "contract-1", "actor-1", {"property-1"})};
  analysis.adjustments.emplace_back("tx-1", 1500.0);
  analysis.createdAt = "2026-01-01T08:00:00Z";
  analysis.updatedAt = "2026-01-02T08:00:00Z";
  return analysis;
}

inline core::ports::workspace::AnnualSnapshot makeAnnual() {
  core::ports::workspace::AnnualSnapshot annual;
  annual.id = "annual-1";
  annual.name = "2026";
  annual.year = 2026;
  annual.analysisIds = {"analysis-1"};
  annual.createdAt = "2026-01-01T08:00:00Z";
  annual.updatedAt = "2026-01-02T08:00:00Z";
  return annual;
}

inline core::ports::workspace::WorkspaceSnapshot makeWorkspaceSnapshot() {
  core::ports::workspace::WorkspaceSnapshot snapshot;
  snapshot.actors = {makeActor()};
  snapshot.properties = {makeProperty()};
  snapshot.contracts = {makeContract()};
  snapshot.statements = {makeStatement()};
  snapshot.transactions = {makeTransaction("tx-1", "Rent", "2026-01-05", 1250.0,
                                           "statement-1", true),
                           makeTransaction("tx-2", "Fees", "2026-01-06", -35.5,
                                           "statement-1", false, {}, {},
                                           {"property-1"}, 1)};
  snapshot.analyses = {makeAnalysis()};
  snapshot.annuals = {makeAnnual()};
  return snapshot;
}

} // namespace ui::tests::support
