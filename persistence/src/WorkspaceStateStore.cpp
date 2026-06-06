/**
 * @file persistence/src/WorkspaceStateStore.cpp
 * @brief Implements atomic SQLite-backed persistence for the aggregate
 * application state.
 */

#include "persistence/WorkspaceStateStore.h"

#include "core/application/export/ExportLog.h"
#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/storage/RepositoryBundle.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/application/workspace/WorkspaceStateManager.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/SqliteTransaction.h"
#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/repositories/SqliteAnnualRepository.h"
#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/repositories/SqliteExportLogRepository.h"
#include "persistence/repositories/SqliteImportLogRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/repositories/SqliteStatementDraftRepository.h"
#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/repositories/SqliteTransactionDraftRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using IdSet = std::unordered_set<std::string>;

struct SnapshotIds {
  std::vector<std::string> actors;
  std::vector<std::string> properties;
  std::vector<std::string> contracts;
  std::vector<std::string> statements;
  std::vector<std::string> transactions;
  std::vector<std::string> analyses;
  std::vector<std::string> annuals;
  std::vector<std::string> statementDrafts;
  std::vector<std::string> transactionDrafts;
  std::vector<std::string> importLogs;
  std::vector<std::string> exportLogs;
  std::unordered_map<std::string, std::string> transactionStatementById;
};

core::storage::RepositoryBundle
createRepositoryBundle(const std::shared_ptr<SqliteDb> &db) {
  core::storage::RepositoryBundle bundle;
  bundle.actors = std::make_shared<SqliteActorRepository>(db);
  bundle.properties = std::make_shared<SqlitePropertyRepository>(db);
  bundle.contracts = std::make_shared<SqliteContractRepository>(db);
  bundle.statements = std::make_shared<SqliteStatementRepository>(db);
  bundle.transactions = std::make_shared<SqliteTransactionRepository>(db);
  bundle.statementDrafts = std::make_shared<SqliteStatementDraftRepository>(db);
  bundle.transactionDrafts =
      std::make_shared<SqliteTransactionDraftRepository>(db);
  bundle.analyses = std::make_shared<SqliteAnalysisRepository>(db);
  bundle.annuals = std::make_shared<SqliteAnnualRepository>(db);
  bundle.importLogs = std::make_shared<SqliteImportLogRepository>(db);
  bundle.exportLogs = std::make_shared<SqliteExportLogRepository>(db);
  return bundle;
}

template <typename Collection>
std::vector<std::string> collectSortedIds(const Collection &items) {
  std::vector<std::string> ids;
  ids.reserve(items.size());
  for (const auto &item : items) {
    if (!item) {
      continue;
    }

    if constexpr (requires { item->id(); }) {
      if (!item->id().empty()) {
        ids.push_back(item->id());
      }
    } else if constexpr (requires { item->id; }) {
      if (!item->id.empty()) {
        ids.push_back(item->id);
      }
    }
  }

  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  return ids;
}

template <typename RepoPtr, typename LoadFn>
std::vector<std::string> collectRepositoryIds(const RepoPtr &repo,
                                              LoadFn &&load) {
  if (!repo) {
    return {};
  }

  return collectSortedIds(load(*repo));
}

IdSet toIdSet(const std::vector<std::string> &ids) {
  return IdSet(ids.begin(), ids.end());
}

SnapshotIds loadExistingIds(const core::storage::RepositoryBundle &repos) {
  SnapshotIds snapshot;
  snapshot.actors = collectRepositoryIds(
      repos.actors, [](const auto &repo) { return repo.getActors(); });
  snapshot.properties = collectRepositoryIds(
      repos.properties, [](const auto &repo) { return repo.getProperties(); });
  snapshot.contracts = collectRepositoryIds(
      repos.contracts, [](const auto &repo) { return repo.getContracts(); });
  snapshot.statements = collectRepositoryIds(
      repos.statements, [](const auto &repo) { return repo.getStatements(); });
  snapshot.transactions =
      collectRepositoryIds(repos.transactions, [](const auto &repo) {
        return repo.getTransactions();
      });
  snapshot.analyses = collectRepositoryIds(
      repos.analyses, [](const auto &repo) { return repo.getAnalyses(); });
  snapshot.annuals = collectRepositoryIds(
      repos.annuals, [](const auto &repo) { return repo.getAnnuals(); });
  snapshot.statementDrafts =
      collectRepositoryIds(repos.statementDrafts, [](const auto &repo) {
        return repo.getStatementDrafts();
      });
  snapshot.transactionDrafts =
      collectRepositoryIds(repos.transactionDrafts, [](const auto &repo) {
        return repo.getTransactionDrafts();
      });
  snapshot.importLogs = collectRepositoryIds(
      repos.importLogs, [](const auto &repo) { return repo.getImportLogs(); });
  snapshot.exportLogs = collectRepositoryIds(
      repos.exportLogs, [](const auto &repo) { return repo.getExportLogs(); });

  if (repos.transactions) {
    for (const auto &transaction : repos.transactions->getTransactions()) {
      if (!transaction || transaction->id().empty()) {
        continue;
      }

      snapshot.transactionStatementById.emplace(transaction->id(),
                                                transaction->statementId());
    }
  }

  return snapshot;
}

SnapshotIds
loadCurrentIds(const core::application::workspace::WorkspaceSessionState &document) {
  SnapshotIds snapshot;
  const auto &catalog = document.catalog;
  const auto &workflow = document.workflow;
  snapshot.actors = collectSortedIds(catalog.actors());
  snapshot.properties = collectSortedIds(catalog.properties());
  snapshot.contracts = collectSortedIds(catalog.contracts());
  snapshot.statements = collectSortedIds(catalog.statements());
  snapshot.transactions = collectSortedIds(catalog.transactions());
  snapshot.analyses = collectSortedIds(catalog.analyses());
  snapshot.annuals = collectSortedIds(catalog.annuals());
  snapshot.statementDrafts = collectSortedIds(workflow.statementDrafts);
  snapshot.transactionDrafts = collectSortedIds(workflow.transactionDrafts);
  snapshot.importLogs = collectSortedIds(workflow.importLogs);
  snapshot.exportLogs = collectSortedIds(workflow.exportLogs);

  for (const auto &transaction : catalog.transactions()) {
    if (!transaction || transaction->id().empty()) {
      continue;
    }

    snapshot.transactionStatementById.emplace(transaction->id(),
                                              transaction->statementId());
  }

  return snapshot;
}

template <typename RepoPtr, typename RemoveFn>
void removeMissingIds(const RepoPtr &repo,
                      const std::vector<std::string> &existingIds,
                      const IdSet &currentIds, RemoveFn &&remove,
                      std::vector<std::string> *deletedIds = nullptr) {
  if (!repo) {
    return;
  }

  for (const auto &id : existingIds) {
    if (currentIds.contains(id)) {
      continue;
    }

    remove(*repo, id);
    if (deletedIds) {
      deletedIds->push_back(id);
    }
  }
}

void removeMissingTransactions(
    const core::storage::RepositoryBundle &repos, const SnapshotIds &existing,
    const SnapshotIds &current,
    const std::vector<std::string> &deletedStatements,
    std::vector<std::string> &deletedTransactions) {
  if (!repos.transactions) {
    return;
  }

  const IdSet currentTransactionIds = toIdSet(current.transactions);
  const IdSet deletedStatementIds(deletedStatements.begin(),
                                  deletedStatements.end());

  for (const auto &transactionId : existing.transactions) {
    if (currentTransactionIds.contains(transactionId)) {
      continue;
    }

    const auto statementIt =
        existing.transactionStatementById.find(transactionId);
    if (statementIt != existing.transactionStatementById.end() &&
        deletedStatementIds.contains(statementIt->second)) {
      continue;
    }

    repos.transactions->removeTransaction(transactionId);
    deletedTransactions.push_back(transactionId);
  }
}

void syncStatementTransactions(
    sqlite3 *db,
    const core::application::workspace::WorkspaceSessionState &document) {
  if (!db) {
    return;
  }

  persistence::StmtGuard deleteStmt(db,
                                    "DELETE FROM statement_transactions;");
  if (!deleteStmt) {
    return;
  }
  deleteStmt.step();

  persistence::StmtGuard insertStmt(
      db,
      "INSERT OR REPLACE INTO statement_transactions (statement_id, transaction_id, position) VALUES (?, ?, ?);");
  if (!insertStmt) {
    return;
  }

  for (const auto &statement : document.catalog.statements()) {
    if (!statement || statement->id().empty()) {
      continue;
    }

    const auto &transactionIds = statement->transactionIds();
    for (std::size_t i = 0; i < transactionIds.size(); ++i) {
      const auto &transactionId = transactionIds[i];
      if (transactionId.empty()) {
        continue;
      }

      insertStmt.reset();
      insertStmt.bindText(1, statement->id());
      insertStmt.bindText(2, transactionId);
      insertStmt.bindInt(3, static_cast<int>(i));
      insertStmt.step();
    }
  }
}

} // namespace

WorkspaceStateStore::WorkspaceStateStore(std::shared_ptr<SqliteDb> db)
    : db_(std::move(db)) {
  if (!db_) {
    throw std::runtime_error("db is null");
  }
}

core::application::workspace::WorkspaceSessionState WorkspaceStateStore::load() {
  auto repos = createRepositoryBundle(db_);
  core::application::WorkspaceStateManager manager(std::move(repos));
  return manager.load();
}

core::domain::DeletionImpact WorkspaceStateStore::save(
    const core::application::workspace::WorkspaceSessionState &document) {
  core::domain::DeletionImpact impact;

  auto repos = createRepositoryBundle(db_);
  SqliteTransaction transaction(db_->handle());

  const auto existing = loadExistingIds(repos);
  const auto current = loadCurrentIds(document);

  removeMissingIds(
      repos.statements, existing.statements, toIdSet(current.statements),
      [](auto &repo, const std::string &id) { repo.removeStatement(id); },
      &impact.deletedStatementIds);

  removeMissingTransactions(repos, existing, current,
                            impact.deletedStatementIds,
                            impact.deletedTransactionIds);

  removeMissingIds(
      repos.contracts, existing.contracts, toIdSet(current.contracts),
      [](auto &repo, const std::string &id) { repo.removeContract(id); },
      &impact.deletedContractIds);
  removeMissingIds(
      repos.actors, existing.actors, toIdSet(current.actors),
      [](auto &repo, const std::string &id) { repo.removeActor(id); },
      &impact.deletedActorIds);
  removeMissingIds(
      repos.properties, existing.properties, toIdSet(current.properties),
      [](auto &repo, const std::string &id) { repo.removeProperty(id); },
      &impact.deletedPropertyIds);
  removeMissingIds(
      repos.analyses, existing.analyses, toIdSet(current.analyses),
      [](auto &repo, const std::string &id) { repo.removeAnalysis(id); },
      &impact.deletedAnalysisIds);
  removeMissingIds(
      repos.annuals, existing.annuals, toIdSet(current.annuals),
      [](auto &repo, const std::string &id) { repo.removeAnnual(id); },
      &impact.deletedAnnualIds);

  removeMissingIds(
      repos.statementDrafts, existing.statementDrafts,
      toIdSet(current.statementDrafts),
      [](auto &repo, const std::string &id) { repo.removeStatementDraft(id); },
      &impact.deletedStatementDraftIds);
  removeMissingIds(
      repos.transactionDrafts, existing.transactionDrafts,
      toIdSet(current.transactionDrafts),
      [](auto &repo, const std::string &id) {
        repo.removeTransactionDraft(id);
      },
      &impact.deletedTransactionDraftIds);
  removeMissingIds(
      repos.importLogs, existing.importLogs, toIdSet(current.importLogs),
      [](auto &repo, const std::string &id) { repo.removeImportLog(id); },
      &impact.deletedImportLogIds);
  removeMissingIds(
      repos.exportLogs, existing.exportLogs, toIdSet(current.exportLogs),
      [](auto &repo, const std::string &id) { repo.removeExportLog(id); },
      &impact.deletedExportLogIds);

  core::application::WorkspaceStateManager manager(std::move(repos));
  manager.save(document);
  syncStatementTransactions(db_->handle(), document);

  transaction.commit();
  return impact;
}
