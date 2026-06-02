/**
 * @file ui/include/ui/workspace/AnnualListModel.h
 * @brief Declarations for the UI AnnualList component.
 */

#pragma once

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/IndexedListModel.h"

namespace ui {

class AnnualList
    : public models::IndexedListModel<core::ports::workspace::AnnualSnapshot> {
  Q_OBJECT
  using Base = models::IndexedListModel<core::ports::workspace::AnnualSnapshot>;

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    YearRole,
    VerificationRole
  };

  explicit AnnualList(QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setAnnuals(std::vector<core::ports::workspace::AnnualSnapshot> annuals);
  const std::vector<std::shared_ptr<core::ports::workspace::AnnualSnapshot>> &
  annuals() const {
    return items();
  }
  int findRowById(const QString &id) const { return findIndexedRow(id); }

  void removeAt(int row) { removeItemAt(row); }
};

} // namespace ui
