/**
 * @file ui/include/ui/workflows/analysis/AnalysisWorkflow.h
 * @brief Declares the UI workflow for analysis creation and computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {
class AnalysisAdapter;
}

namespace core::ports::analysis {
struct AnalysisRequest;
}

namespace ui {

/**
 * @brief Coordinates analysis persistence and computations for UI states.
 */
class AnalysisWorkflow : public QObject {
  Q_OBJECT
public:
  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;

  explicit AnalysisWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter,
      QObject *parent = nullptr);

  QString analysisFilterSpec(const QString &dateField, const QString &dateMode,
                             const QString &year, const QString &dateFrom,
                             const QString &dateTo,
                             const QStringList &propertyIds,
                             const QStringList &contractTypes,
                             const QString &allocatableMode) const;
  QVariantMap parseAnalysisFilterSpec(const QString &filterSpec) const;

  QString analysisConfigJson(const QString &type, const QString &plotType,
                             const QString &plotMeasure,
                             const QStringList &propertyIds,
                             const QStringList &contractTypes,
                             double taxPercent) const;

  QString analysisAdjustmentsJson(const QVariantList &transactions,
                                  const QStringList &selectedTransactionIds,
                                  double taxPercent) const;

  QVariantMap computeAnalysisPreview(const QString &analysisId,
                                     const QString &filterSpecification,
                                     bool includeCalcAdjustments,
                                     const QString &adjustmentsJson) const;

  QVariantMap previewTransactions(const QString &filterSpec) const;

  QStringList contractTypes() const;

private:
  core::ports::analysis::AnalysisRequest
  analysisRequest(const QString &analysisId,
                  const QString &filterSpecification) const;
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter_;
};

} // namespace ui
