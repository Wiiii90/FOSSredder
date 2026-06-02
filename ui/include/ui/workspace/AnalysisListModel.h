/**
 * @file ui/include/ui/workspace/AnalysisListModel.h
 * @brief Declarations for the UI AnalysisList component.
 */

#pragma once

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/IndexedListModel.h"

namespace ui {

class AnalysisList : public models::IndexedListModel<
                         core::ports::workspace::AnalysisSnapshot> {
  Q_OBJECT
  using Base =
      models::IndexedListModel<core::ports::workspace::AnalysisSnapshot>;

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    TypeRole,
    ConfigRole,
    FilterRole,
    AdjustmentsRole,
    ExportFormatRole,
    IncludeCalcAdjustmentsRole,
    ExportStateRole,
    SnapshotTransactionsRole,
    CreatedAtRole,
    UpdatedAtRole
  };

  explicit AnalysisList(QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void
  setAnalyses(std::vector<core::ports::workspace::AnalysisSnapshot> analyses);
  const std::vector<std::shared_ptr<core::ports::workspace::AnalysisSnapshot>> &
  analyses() const {
    return items();
  }
  int findRowById(const QString &id) const { return findIndexedRow(id); }

  void removeAt(int row);

private:
  static QString serializeAdjustmentsJson(
      const core::ports::workspace::AnalysisSnapshot &analysis);
  void refreshAdjustmentsCache();
  void refreshAdjustmentsCacheEntry(
      const core::ports::workspace::AnalysisSnapshot &analysis);

  QHash<QString, QString> adjustmentsJsonById_;
};

} // namespace ui
