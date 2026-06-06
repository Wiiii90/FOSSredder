/**
 * @file ui/src/workspace/WorkspaceStore.cpp
 * @brief Implements the UI workspace snapshot store.
 */

#include "ui/workspace/WorkspaceStore.h"

#include <algorithm>
#include <utility>

#include "ui/util/StringConversions.h"

namespace ui {

namespace {

template<typename Row>
void removeRowsByIds(std::vector<Row>& rows,
                     const std::vector<std::string>& ids) {
  if (ids.empty() || rows.empty()) {
    return;
  }
  rows.erase(std::remove_if(rows.begin(), rows.end(),
                            [&](const Row& row) {
                              return std::find(ids.begin(), ids.end(),
                                               row.id) != ids.end();
                            }),
             rows.end());
}

void removeTransactionsByIds(
    std::vector<core::ports::workspace::StatementSnapshot>& statements,
    const std::vector<std::string>& transactionIds) {
  if (transactionIds.empty()) {
    return;
  }
  for (auto& statement : statements) {
    auto& ids = statement.transactionIds;
    ids.erase(std::remove_if(ids.begin(), ids.end(),
                             [&](const std::string& id) {
                               return std::find(transactionIds.begin(),
                                                transactionIds.end(),
                                                id) != transactionIds.end();
                             }),
              ids.end());
  }
}

} // namespace

WorkspaceStore::WorkspaceStore(QObject* parent) : QObject(parent) {}

void WorkspaceStore::setWorkspacePorts(
    core::ports::workspace::IWorkspaceWriter* writer,
    core::ports::workspace::IWorkspaceReader* reader) {
  writer_ = writer;
  reader_ = reader;
  if (reader_) {
    loadFromState(reader_->workspaceSnapshot());
  }
}

void WorkspaceStore::loadFromState(
    const core::ports::workspace::WorkspaceSnapshot& snapshot) {
  snapshot_ = snapshot;
  bumpDataRevision();
}

void WorkspaceStore::applyDeletionImpact(
    const core::ports::workspace::DeletionImpact& impact) {
  removeRowsByIds(snapshot_.transactions, impact.deletedTransactionIds);
  removeTransactionsByIds(snapshot_.statements, impact.deletedTransactionIds);
  removeRowsByIds(snapshot_.statements, impact.deletedStatementIds);
  removeRowsByIds(snapshot_.properties, impact.deletedPropertyIds);
  removeRowsByIds(snapshot_.actors, impact.deletedActorIds);
  removeRowsByIds(snapshot_.contracts, impact.deletedContractIds);
  removeRowsByIds(snapshot_.analyses, impact.deletedAnalysisIds);
  removeRowsByIds(snapshot_.annuals, impact.deletedAnnualIds);
  removeRowsByIds(snapshot_.statementDrafts, impact.deletedStatementDraftIds);
  removeRowsByIds(snapshot_.importLogs, impact.deletedImportLogIds);
  removeRowsByIds(snapshot_.exportLogs, impact.deletedExportLogIds);

  for (auto& draft : snapshot_.statementDrafts) {
    removeRowsByIds(draft.transactions, impact.deletedTransactionDraftIds);
    auto& ids = draft.transactionIds;
    ids.erase(std::remove_if(
                  ids.begin(), ids.end(),
                  [&](const std::string& id) {
                    return std::find(impact.deletedTransactionDraftIds.begin(),
                                     impact.deletedTransactionDraftIds.end(),
                                     id) !=
                           impact.deletedTransactionDraftIds.end();
                  }),
              ids.end());
  }

  bumpDataRevision();
}

QString WorkspaceStore::currentPath() const {
  if (reader_) {
    return QString::fromStdString(reader_->currentPath());
  }
  return QString::fromStdString(snapshot_.currentPath);
}

void WorkspaceStore::bumpDataRevision() {
  ++dataRevision_;
  emit dataRevisionChanged();
}

} // namespace ui
