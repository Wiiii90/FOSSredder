/**
 * @file ui/src/workspace/AnalysisListModel.cpp
 * @brief Implementation of the UI AnalysisList component.
 */

#include "ui/workspace/AnalysisListModel.h"

#include "ui/presentation/PayloadKeys.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

namespace ui {

namespace {

QString emptyObjectJson() { return QStringLiteral("{}"); }

} // namespace

QString AnalysisList::serializeAdjustmentsJson(
    const core::ports::workspace::AnalysisSnapshot &analysis) {
  QJsonObject obj;
  for (const auto &[id, amount] : analysis.adjustments) {
    obj.insert(QString::fromStdString(id), amount);
  }
  return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void AnalysisList::refreshAdjustmentsCache() {
  adjustmentsJsonById_.clear();
  adjustmentsJsonById_.reserve(static_cast<int>(analyses().size()));
  for (const auto &analysis : analyses()) {
    if (!analysis)
      continue;
    refreshAdjustmentsCacheEntry(*analysis);
  }
}

void AnalysisList::refreshAdjustmentsCacheEntry(
    const core::ports::workspace::AnalysisSnapshot &analysis) {
  adjustmentsJsonById_.insert(QString::fromStdString(analysis.id),
                              serializeAdjustmentsJson(analysis));
}

AnalysisList::AnalysisList(QObject *parent) : Base(parent) {}

QVariant AnalysisList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &a = itemAtRow(index.row());
  if (!a)
    return {};

  const QString adjustmentsJson = adjustmentsJsonById_.value(
      QString::fromStdString(a->id), emptyObjectJson());

  switch (role) {
  case IdRole:
    return QString::fromStdString(a->id);
  case NameRole:
    return QString::fromStdString(a->name);
  case TypeRole:
    return QString::fromStdString(a->type);
  case ConfigRole:
    return QString::fromStdString(a->configJson);
  case FilterRole:
    return QString::fromStdString(a->filterSpec);
  case AdjustmentsRole:
    return adjustmentsJson;
  case ExportFormatRole:
    return QString::fromStdString(a->exportFormat);
  case IncludeCalcAdjustmentsRole:
    return a->includeCalculationAdjustments;
  case ExportStateRole:
    return QString::fromStdString(a->exportStateJson);
  case SnapshotTransactionsRole:
    return QString::fromStdString(a->snapshotTransactionsJson);
  case CreatedAtRole:
    return QString::fromStdString(a->createdAt);
  case UpdatedAtRole:
    return QString::fromStdString(a->updatedAt);
  default:
    return {};
  }
}

QHash<int, QByteArray> AnalysisList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
  roles[ConfigRole] = ui::payload::keys::analysis::kConfig.toUtf8();
  roles[FilterRole] = ui::payload::keys::analysis::kFilter.toUtf8();
  roles[AdjustmentsRole] = ui::payload::keys::analysis::kAdjustments.toUtf8();
  roles[ExportFormatRole] = ui::payload::keys::analysis::kExportFormat.toUtf8();
  roles[IncludeCalcAdjustmentsRole] =
      ui::payload::keys::analysis::kIncludeCalcAdjustments.toUtf8();
  roles[ExportStateRole] = ui::payload::keys::analysis::kExportState.toUtf8();
  roles[SnapshotTransactionsRole] =
      ui::payload::keys::analysis::kSnapshotTransactions.toUtf8();
  roles[CreatedAtRole] = ui::payload::keys::analysis::kCreatedAt.toUtf8();
  roles[UpdatedAtRole] = ui::payload::keys::analysis::kUpdatedAt.toUtf8();
  return roles;
}

void AnalysisList::setAnalyses(
    std::vector<core::ports::workspace::AnalysisSnapshot> analyses) {
  setValueItems(std::move(analyses));
  refreshAdjustmentsCache();
}

void AnalysisList::removeAt(int row) {
  const auto &analysis = itemAtRow(row);
  if (analysis)
    adjustmentsJsonById_.remove(QString::fromStdString(analysis->id));
  removeItemAt(row);
}

} // namespace ui
