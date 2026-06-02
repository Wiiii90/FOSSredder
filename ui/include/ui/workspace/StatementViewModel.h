/**
 * @file ui/include/ui/workspace/StatementViewModel.h
 * @brief Declarations for the UI StatementViewModel component.
 */

#pragma once

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/IndexedListModel.h"

namespace ui {

class StatementViewModel : public models::IndexedListModel<
                          core::ports::workspace::StatementSnapshot> {
  Q_OBJECT
  using Base =
      models::IndexedListModel<core::ports::workspace::StatementSnapshot>;

public:
  enum Roles { IdRole = Qt::UserRole + 1, NameRole };

  explicit StatementViewModel(QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setStatements(
      std::vector<core::ports::workspace::StatementSnapshot> statements);
  const std::vector<
      std::shared_ptr<core::ports::workspace::StatementSnapshot>> &
  statements() const {
    return items();
  }
  int findRowById(const QString &id) const { return findIndexedRow(id); }
  void removeAt(int row) { removeItemAt(row); }
};

} // namespace ui
