/**
 * @file ui/src/workspace/AnnualListModel.cpp
 * @brief Implementation of the UI AnnualList component.
 */

#include "ui/workspace/AnnualListModel.h"

#include <QVariant>

#include "ui/presentation/PayloadKeys.h"

namespace ui {

AnnualList::AnnualList(QObject *parent) : Base(parent) {}

QVariant AnnualList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &a = itemAtRow(index.row());
  if (!a)
    return {};

  switch (role) {
  case IdRole:
    return QString::fromStdString(a->id);
  case NameRole:
    return QString::fromStdString(a->name);
  case YearRole:
    return a->year;
  default:
    return {};
  }
}

QHash<int, QByteArray> AnnualList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::annual::kName.toUtf8();
  roles[YearRole] = ui::payload::keys::annual::kYear.toUtf8();
  return roles;
}

void AnnualList::setAnnuals(
    std::vector<core::ports::workspace::AnnualSnapshot> annuals) {
  setValueItems(std::move(annuals));
}

} // namespace ui
