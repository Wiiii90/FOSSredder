/**
 * @file ui/include/ui/workspace/RowListModel.h
 * @brief Declarations for the UI RowListModel component.
 */

#pragma once

#include <QAbstractListModel>
#include <QVector>

#include <initializer_list>
#include <utility>
#include <vector>

namespace ui::models {

template <typename Row> class RowListModel : public QAbstractListModel {
public:
  explicit RowListModel(QObject *parent = nullptr)
      : QAbstractListModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0;
    return static_cast<int>(rows_.size());
  }

protected:
  const std::vector<Row> &rows() const noexcept { return rows_; }

  void setRows(std::vector<Row> rows) {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int appendRow(Row row) {
    const int index = static_cast<int>(rows_.size());
    beginInsertRows(QModelIndex(), index, index);
    rows_.push_back(std::move(row));
    endInsertRows();
    return index;
  }

  void replaceRow(int index, Row row) {
    if (!hasRow(index))
      return;
    rows_[static_cast<size_t>(index)] = std::move(row);
  }

  void removeRow(int index) {
    if (!hasRow(index))
      return;
    beginRemoveRows(QModelIndex(), index, index);
    rows_.erase(rows_.begin() + index);
    endRemoveRows();
  }

  void clearRows() {
    beginResetModel();
    rows_.clear();
    endResetModel();
  }

  bool hasRow(int index) const noexcept {
    return index >= 0 && index < static_cast<int>(rows_.size());
  }

  const Row *rowPtr(int index) const noexcept {
    return hasRow(index) ? &rows_[static_cast<size_t>(index)] : nullptr;
  }

  Row *rowPtr(int index) noexcept {
    return hasRow(index) ? &rows_[static_cast<size_t>(index)] : nullptr;
  }

  void emitRowChanged(int index, std::initializer_list<int> roles = {}) {
    if (!hasRow(index))
      return;
    const QModelIndex modelIndex = this->index(index);
    emit this->dataChanged(modelIndex, modelIndex,
                           QVector<int>(roles.begin(), roles.end()));
  }

  void emitAllRowsChanged(std::initializer_list<int> roles = {}) {
    if (rows_.empty())
      return;
    emit this->dataChanged(this->index(0),
                           this->index(static_cast<int>(rows_.size()) - 1),
                           QVector<int>(roles.begin(), roles.end()));
  }

private:
  std::vector<Row> rows_;
};

} // namespace ui::models
