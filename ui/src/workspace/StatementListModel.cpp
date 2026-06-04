/**
 * @file ui/src/workspace/StatementListModel.cpp
 * @brief Implements the workspace statement list model.
 */

#include "ui/workspace/StatementListModel.h"

#include <QVariant>

#include "ui/presentation/PayloadKeys.h"

namespace ui {

StatementListModel::StatementListModel(QObject *parent) : Base(parent) {}

QVariant StatementListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const auto &s = itemAtRow(index.row());
  if (!s) {
    return {};
  }

  switch (role) {
  case IdRole:
    return QString::fromStdString(s->id);
  case NameRole:
    return QString::fromStdString(s->name);
  default:
    return {};
  }
}

QHash<int, QByteArray> StatementListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  return roles;
}

void StatementListModel::setStatements(
    std::vector<core::ports::workspace::StatementSnapshot> statements) {
  setValueItems(std::move(statements));
}

} // namespace ui
