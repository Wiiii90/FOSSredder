/**
 * @file ui/include/ui/workspace/TransactionListModel.h
 * @brief List model projecting workspace transaction snapshots for QML bindings.
 */

#pragma once

#include <QHash>
#include <QString>
#include <QVariant>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/RowListModel.h"

namespace ui {

class TransactionListModel
    : public models::RowListModel<core::ports::workspace::TransactionSnapshot> {
  Q_OBJECT
  using Base =
      models::RowListModel<core::ports::workspace::TransactionSnapshot>;

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    StatementIdRole,
    NameRole,
    BookingDateRole,
    ValutaRole,
    AmountRole,
    StatusRole,
    ActorIdRole,
    ContractIdRole,
    MetadataRole,
    TypeRole,
    AllocatableRole,
    PropertyIdsRole
  };

  explicit TransactionListModel(QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setTransactions(
      std::vector<core::ports::workspace::TransactionSnapshot> transactions);
  void setContractTypes(QHash<QString, QString> contractTypes,
                        bool notify = true);
  const std::vector<core::ports::workspace::TransactionSnapshot> &
  transactions() const {
    return rows();
  }
  int findRowById(const QString &id) const;
  void setTransactionAt(int row,
                        core::ports::workspace::TransactionSnapshot tx);
  void removeAt(int row);
  QVariantMap get(int index) const;

private:
  void rebuildIdIndex();
  QString contractTypeForTransaction(
      const core::ports::workspace::TransactionSnapshot &transaction) const;
  void fillTransactionMap(
      QVariantMap &map,
      const core::ports::workspace::TransactionSnapshot &transaction) const;
  QVector<int> allRoles() const;

  QHash<QString, int> idToRow_;
  QHash<QString, QString> contractTypeById_;
};

} // namespace ui
