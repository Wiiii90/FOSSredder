/**
 * @file ui/include/ui/workspace/WorkflowRunListModel.h
 * @brief Shared row-list behavior for workflow run history models.
 */

#pragma once

#include <QString>

#include <initializer_list>
#include <utility>
#include <vector>

#include "ui/workspace/RowListModel.h"

namespace ui::models {

template <typename Row>
class WorkflowRunListModel : public RowListModel<Row> {
private:
  using Base = RowListModel<Row>;

public:
  explicit WorkflowRunListModel(QObject *parent = nullptr) : Base(parent) {}

  bool upsertRun(const Row &row, std::initializer_list<int> changedRoles) {
    const int idx = findByLogId(row.logId);
    if (idx < 0) {
      Base::appendRow(row);
      return true;
    }
    Base::replaceRow(idx, row);
    Base::emitRowChanged(idx, changedRoles);
    return false;
  }

  int findByLogId(const QString &logId) const {
    if (logId.isEmpty())
      return -1;
    const auto &items = Base::rows();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
      if (items[static_cast<size_t>(i)].logId == logId)
        return i;
    }
    return -1;
  }

  Row at(int index) const {
    const auto *row = Base::rowPtr(index);
    return row ? *row : Row{};
  }

  std::vector<Row> snapshot() const { return Base::rows(); }

  void setRuns(std::vector<Row> runs) { Base::setRows(std::move(runs)); }

  void removeAt(int index) { Base::removeRow(index); }

  void clear() { Base::clearRows(); }
};

} // namespace ui::models
