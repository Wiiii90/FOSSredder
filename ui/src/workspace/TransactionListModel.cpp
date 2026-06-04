/**
 * @file ui/src/workspace/TransactionListModel.cpp
 * @brief Implements the workspace transaction list model.
 */

#include "ui/workspace/TransactionListModel.h"

#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"
#include <QVariant>

namespace ui {

namespace {

const QVector<int> &transactionRoles() {
  static const QVector<int> roles{
      TransactionListModel::IdRole,
      TransactionListModel::StatementIdRole,
      TransactionListModel::NameRole,
      TransactionListModel::BookingDateRole,
      TransactionListModel::ValutaRole,
      TransactionListModel::AmountRole,
      TransactionListModel::StatusRole,
      TransactionListModel::ActorIdRole,
      TransactionListModel::ContractIdRole,
      TransactionListModel::MetadataRole,
      TransactionListModel::TypeRole,
      TransactionListModel::AllocatableRole,
      TransactionListModel::PropertyIdsRole};
  return roles;
}

} // namespace

void TransactionListModel::rebuildIdIndex() {
  idToRow_.clear();
  const auto &items = rows();
  idToRow_.reserve(static_cast<int>(items.size()));
  for (int i = 0; i < static_cast<int>(items.size()); ++i) {
    const auto &t = items[static_cast<size_t>(i)];
    idToRow_.insert(QString::fromStdString(t.id), i);
  }
}

QVector<int> TransactionListModel::allRoles() const { return transactionRoles(); }

QString TransactionListModel::contractTypeForTransaction(
    const core::ports::workspace::TransactionSnapshot &transaction) const {
  if (transaction.contractId.empty()) {
    return {};
  }
  const QString contractId = QString::fromStdString(transaction.contractId);
  const auto it = contractTypeById_.find(contractId);
  return it == contractTypeById_.end() ? QString() : it.value();
}

void TransactionListModel::fillTransactionMap(
    QVariantMap &map,
    const core::ports::workspace::TransactionSnapshot &transaction) const {
  map[payload::keys::common::kId] = QString::fromStdString(transaction.id);
  map[payload::keys::common::kName] = QString::fromStdString(transaction.name);
  map[payload::keys::transaction::kBookingDate] =
      QString::fromStdString(transaction.bookingDate);
  map[payload::keys::transaction::kValuta] =
      QString::fromStdString(transaction.valuta);
  map[payload::keys::common::kAmount] = transaction.amount;
  map[payload::keys::common::kStatus] = transaction.status;
  map[payload::keys::transaction::kActorId] =
      QString::fromStdString(transaction.actorId);
  map[payload::keys::transaction::kContractId] =
      QString::fromStdString(transaction.contractId);
  map[payload::keys::common::kMetadata] = QString();
  map[payload::keys::common::kType] = contractTypeForTransaction(transaction);
  map[payload::keys::transaction::kAllocatable] = transaction.allocatable;
  map[payload::keys::transaction::kPropertyIds] =
      payload::mapper::toVariantStringList(transaction.propertyIds);
}

TransactionListModel::TransactionListModel(QObject *parent) : Base(parent) {}

QVariant TransactionListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const auto *entry = rowPtr(index.row());
  if (!entry) {
    return {};
  }
  const auto &t = *entry;

  switch (role) {
  case IdRole:
    return QString::fromStdString(t.id);
  case StatementIdRole:
    return QString::fromStdString(t.statementId);
  case NameRole:
    return QString::fromStdString(t.name);
  case BookingDateRole:
    return QString::fromStdString(t.bookingDate);
  case ValutaRole:
    return QString::fromStdString(t.valuta);
  case AmountRole:
    return t.amount;
  case StatusRole:
    return t.status;
  case ActorIdRole:
    return QString::fromStdString(t.actorId);
  case ContractIdRole:
    return QString::fromStdString(t.contractId);
  case MetadataRole:
    return QString();
  case TypeRole:
    return contractTypeForTransaction(t);
  case AllocatableRole:
    return t.allocatable;
  case PropertyIdsRole:
    return payload::mapper::toVariantStringList(t.propertyIds);
  default:
    return {};
  }
}

QHash<int, QByteArray> TransactionListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = payload::keys::common::kId.toUtf8();
  roles[StatementIdRole] = payload::keys::statement::kStatementId.toUtf8();
  roles[NameRole] = payload::keys::common::kName.toUtf8();
  roles[BookingDateRole] = payload::keys::transaction::kBookingDate.toUtf8();
  roles[ValutaRole] = payload::keys::transaction::kValuta.toUtf8();
  roles[AmountRole] = payload::keys::common::kAmount.toUtf8();
  roles[StatusRole] = payload::keys::common::kStatus.toUtf8();
  roles[ActorIdRole] = payload::keys::transaction::kActorId.toUtf8();
  roles[ContractIdRole] = payload::keys::transaction::kContractId.toUtf8();
  roles[MetadataRole] = payload::keys::common::kMetadata.toUtf8();
  roles[TypeRole] = payload::keys::common::kType.toUtf8();
  roles[AllocatableRole] = payload::keys::transaction::kAllocatable.toUtf8();
  roles[PropertyIdsRole] = payload::keys::transaction::kPropertyIds.toUtf8();
  return roles;
}

void TransactionListModel::setTransactions(
    std::vector<core::ports::workspace::TransactionSnapshot> transactions) {
  setRows(std::move(transactions));
  rebuildIdIndex();
}

void TransactionListModel::setContractTypes(
    QHash<QString, QString> contractTypes, bool notify) {
  if (contractTypeById_ == contractTypes) {
    return;
  }
  contractTypeById_ = std::move(contractTypes);
  if (notify) {
    emitAllRowsChanged({TypeRole});
  }
}

int TransactionListModel::findRowById(const QString &id) const {
  if (id.isEmpty()) {
    return -1;
  }
  const auto it = idToRow_.find(id);
  return it == idToRow_.end() ? -1 : it.value();
}

void TransactionListModel::setTransactionAt(
    int row, core::ports::workspace::TransactionSnapshot tx) {
  if (!hasRow(row)) {
    return;
  }
  replaceRow(row, std::move(tx));
  rebuildIdIndex();
  const QModelIndex modelIndex = index(row);
  emit dataChanged(modelIndex, modelIndex, allRoles());
}

void TransactionListModel::removeAt(int row) {
  if (!hasRow(row)) {
    return;
  }
  removeRow(row);
  rebuildIdIndex();
}

QVariantMap TransactionListModel::get(int index) const {
  QVariantMap m;
  const auto *entry = rowPtr(index);
  if (!entry) {
    return m;
  }
  fillTransactionMap(m, *entry);
  return m;
}

} // namespace ui
