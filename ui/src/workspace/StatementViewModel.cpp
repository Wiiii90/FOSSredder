/**
 * @file ui/src/workspace/StatementViewModel.cpp
 * @brief Implementation of the UI StatementViewModel component.
 */

#include "ui/workspace/StatementViewModel.h"

#include <QVariant>

#include "ui/shared/payload/PayloadKeys.h"

namespace ui {

StatementViewModel::StatementViewModel(QObject *parent) : Base(parent) {}

QVariant StatementViewModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &s = itemAtRow(index.row());
  if (!s)
    return {};

  switch (role) {
  case IdRole:
    return QString::fromStdString(s->id);
  case NameRole:
    return QString::fromStdString(s->name);
  default:
    return {};
  }
}

QHash<int, QByteArray> StatementViewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  return roles;
}

void StatementViewModel::setStatements(
    std::vector<core::ports::workspace::StatementSnapshot> statements) {
  setValueItems(std::move(statements));
}

} // namespace ui
