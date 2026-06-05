/**
 * @file ui/src/workspace/WorkspaceSelectors.cpp
 * @brief Implements read-only workspace selectors for QML payloads.
 */

#include "ui/workspace/WorkspaceSelectors.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"
#include "ui/util/StringConversions.h"
#include "ui/workspace/WorkspacePayloads.h"

namespace ui {

namespace {

template<typename Rows, typename Project>
QVariantList projectRows(const Rows& rows, Project project) {
  QVariantList out;
  out.reserve(static_cast<int>(rows.size()));
  for (const auto& row : rows) {
    out.push_back(project(row));
  }
  return out;
}

template<typename Rows>
bool containsId(const Rows& rows, const QString& id) {
  const std::string target = strings::toStdString(id.trimmed());
  if (target.empty()) {
    return false;
  }
  return std::any_of(rows.begin(), rows.end(), [&](const auto& row) {
    return row.id == target;
  });
}

void prependEmptyDropdownRow(QVariantList& rows, const QString& display) {
  rows.push_front(workspace_payloads::dropdownRow({}, display));
}

std::vector<std::string> toStdStringVector(const QVariantList& values) {
  std::vector<std::string> out;
  out.reserve(static_cast<std::size_t>(values.size()));
  for (const auto& value : values) {
    const QString text = value.toString().trimmed();
    if (!text.isEmpty()) {
      out.push_back(strings::toStdString(text));
    }
  }
  return out;
}

QVariantList toVariantList(const std::vector<std::string>& values) {
  QVariantList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) {
    const QString text = QString::fromStdString(value).trimmed();
    if (!text.isEmpty()) {
      out.push_back(text);
    }
  }
  return out;
}

} // namespace

WorkspaceSelectors::WorkspaceSelectors(WorkspaceStore& store, QObject* parent)
    : QObject(parent), store_(store) {}

int WorkspaceSelectors::dataRevision() const noexcept {
  return store_.dataRevision();
}

QVariantList WorkspaceSelectors::actorRows() const {
  return projectRows(store_.snapshot().actors, workspace_payloads::actorRow);
}

QVariantList WorkspaceSelectors::propertyRows() const {
  return projectRows(store_.snapshot().properties,
                     workspace_payloads::propertyRow);
}

QVariantList WorkspaceSelectors::contractRows() const {
  return projectRows(store_.snapshot().contracts,
                     workspace_payloads::contractRow);
}

QVariantList WorkspaceSelectors::analysisRows() const {
  return projectRows(store_.snapshot().analyses,
                     workspace_payloads::analysisRow);
}

QVariantList WorkspaceSelectors::annualRows() const {
  return projectRows(store_.snapshot().annuals, workspace_payloads::annualRow);
}

QVariantList WorkspaceSelectors::statementRows() const {
  return projectRows(store_.snapshot().statements,
                     workspace_payloads::statementRow);
}

QVariantList WorkspaceSelectors::statementRowsWithTransactions() const {
  QVariantList rows = statementRows();
  for (QVariant& rowValue : rows) {
    QVariantMap row = rowValue.toMap();
    const QString statementId =
        row.value(payload::keys::common::kId).toString();
    row.insert(QStringLiteral("transactions"),
               statementTransactionRows(statementId));
    rowValue = row;
  }
  return rows;
}

QVariantList
WorkspaceSelectors::statementTransactionRows(const QString& statementId) const {
  QVariantList out;
  if (statementId.isEmpty()) {
    return out;
  }
  const std::string target = strings::toStdString(statementId);
  for (const auto& transaction : store_.snapshot().transactions) {
    if (transaction.statementId == target) {
      out.push_back(workspace_payloads::statementTransactionRow(transaction));
    }
  }
  return out;
}

QVariantMap WorkspaceSelectors::transactionRowById(const QString& id) const {
  const std::string target = strings::toStdString(id.trimmed());
  if (target.empty()) {
    return {};
  }
  const auto& transactions = store_.snapshot().transactions;
  const auto it = std::find_if(transactions.begin(), transactions.end(),
                               [&](const auto& tx) {
                                 return tx.id == target;
                               });
  if (it == transactions.end()) {
    return {};
  }
  return workspace_payloads::transactionRow(
      *it, contractTypeForId(QString::fromStdString(it->contractId)));
}

QVariantMap WorkspaceSelectors::transactionFormWithCatalogSelection(
    const QVariantMap& transaction, const QVariantMap& changes) const {
  if (!store_.reader()) {
    return transaction;
  }

  core::ports::workspace::TransactionCatalogSelectionChange change;
  change.current.actorId = strings::toStdString(
      transaction.value(payload::keys::transaction::kActorId)
          .toString()
          .trimmed());
  change.current.contractId = strings::toStdString(
      transaction.value(payload::keys::transaction::kContractId)
          .toString()
          .trimmed());
  change.current.propertyIds = toStdStringVector(
      transaction.value(payload::keys::transaction::kPropertyIds).toList());

  change.contractChanged =
      changes.contains(payload::keys::transaction::kContractId);
  change.actorChanged = changes.contains(payload::keys::transaction::kActorId);
  change.propertiesChanged =
      changes.contains(payload::keys::transaction::kPropertyIds);
  change.contractId = strings::toStdString(
      changes.value(payload::keys::transaction::kContractId)
          .toString()
          .trimmed());
  change.actorId = strings::toStdString(
      changes.value(payload::keys::transaction::kActorId).toString().trimmed());
  change.propertyIds = toStdStringVector(
      changes.value(payload::keys::transaction::kPropertyIds).toList());

  const auto selection = store_.reader()->transactionCatalogSelection(change);

  QVariantMap next = transaction;
  next.insert(payload::keys::transaction::kActorId,
              QString::fromStdString(selection.actorId));
  next.insert(payload::keys::transaction::kContractId,
              QString::fromStdString(selection.contractId));
  next.insert(payload::keys::transaction::kPropertyIds,
              toVariantList(selection.propertyIds));
  return next;
}

QVariantList WorkspaceSelectors::actorDropdownRows() const {
  QVariantList rows = actorRows();
  prependEmptyDropdownRow(rows, tr("No actor"));
  return rows;
}

QVariantList WorkspaceSelectors::propertyDropdownRows() const {
  return propertyRows();
}

QVariantList WorkspaceSelectors::contractDropdownRows() const {
  QVariantList rows = contractRows();
  prependEmptyDropdownRow(rows, tr("No contract"));
  return rows;
}

QVariantList WorkspaceSelectors::importLogRows() const {
  return projectRows(store_.snapshot().importLogs,
                     workspace_payloads::importLogRow);
}

QVariantList WorkspaceSelectors::exportLogRows() const {
  return projectRows(store_.snapshot().exportLogs,
                     workspace_payloads::exportLogRow);
}

QStringList WorkspaceSelectors::attachedImportDraftIds() const {
  QStringList ids;
  const auto& logs = store_.snapshot().importLogs;
  ids.reserve(static_cast<int>(logs.size()));
  for (const auto& entry : logs) {
    if (!entry.draftAttached) {
      continue;
    }
    const QString draftId = QString::fromStdString(entry.draftId);
    const QString logId = QString::fromStdString(entry.id);
    const QString id = !draftId.isEmpty() ? draftId : logId;
    if (id.isEmpty() || ids.contains(id)) {
      continue;
    }
    ids.push_back(id);
  }
  return ids;
}

QString WorkspaceSelectors::exportLogTargetPath(const QString& id) const {
  const auto& logs = store_.snapshot().exportLogs;
  const QString trimmedId = id.trimmed();
  if (!trimmedId.isEmpty()) {
    const std::string idStd = strings::toStdString(trimmedId);
    const auto it =
        std::find_if(logs.begin(), logs.end(), [&](const auto& log) {
          return log.id == idStd;
        });
    if (it != logs.end()) {
      return QString::fromStdString(it->targetPath);
    }
  }
  return {};
}

QString WorkspaceSelectors::actorIdByName(const QString& name) const {
  if (!store_.reader()) {
    return {};
  }
  return QString::fromStdString(
      store_.reader()->actorIdentityByName(strings::toStdString(name)).id);
}

QString WorkspaceSelectors::propertyIdByName(const QString& name) const {
  if (!store_.reader()) {
    return {};
  }
  return QString::fromStdString(
      store_.reader()->propertyIdentityByName(strings::toStdString(name)).id);
}

QString WorkspaceSelectors::contractIdBySignature(
    const QString& name, const QString& type, const QStringList& actorIds,
    const QStringList& propertyIds) const {
  if (!store_.reader()) {
    return {};
  }
  return QString::fromStdString(
      store_.reader()
          ->contractIdentityBySignature(
              strings::toStdString(name), strings::toStdString(type),
              strings::toStdList(actorIds), strings::toStdList(propertyIds))
          .id);
}

QString WorkspaceSelectors::nextContractName() const {
  return store_.reader()
             ? QString::fromStdString(store_.reader()->nextContractName())
             : QStringLiteral("Contract 1");
}

bool WorkspaceSelectors::hasActorId(const QString& id) const {
  return containsId(store_.snapshot().actors, id);
}

bool WorkspaceSelectors::hasPropertyId(const QString& id) const {
  return containsId(store_.snapshot().properties, id);
}

bool WorkspaceSelectors::hasContractId(const QString& id) const {
  return containsId(store_.snapshot().contracts, id);
}

bool WorkspaceSelectors::hasStatementId(const QString& id) const {
  return containsId(store_.snapshot().statements, id);
}

bool WorkspaceSelectors::hasTransactionId(const QString& id) const {
  return containsId(store_.snapshot().transactions, id);
}

bool WorkspaceSelectors::hasAnalysisId(const QString& id) const {
  return containsId(store_.snapshot().analyses, id);
}

bool WorkspaceSelectors::hasAnnualId(const QString& id) const {
  return containsId(store_.snapshot().annuals, id);
}

QString WorkspaceSelectors::contractTypeForId(const QString& contractId) const {
  const std::string target = strings::toStdString(contractId.trimmed());
  if (target.empty()) {
    return {};
  }
  const auto& contracts = store_.snapshot().contracts;
  const auto it = std::find_if(contracts.begin(), contracts.end(),
                               [&](const auto& contract) {
                                 return contract.id == target;
                               });
  return it == contracts.end() ? QString() : QString::fromStdString(it->type);
}

} // namespace ui
