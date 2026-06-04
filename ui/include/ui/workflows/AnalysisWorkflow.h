/**
 * @file ui/include/ui/workflows/AnalysisWorkflow.h
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
      QObject* parent = nullptr);

  QString analysisFilterSpec(const QString& dateField, const QString& dateMode,
                             const QString& year, const QString& dateFrom,
                             const QString& dateTo,
                             const QStringList& propertyIds,
                             const QStringList& contractTypes,
                             const QString& allocatableMode) const;
  QVariantMap parseAnalysisFilterSpec(const QString& filterSpec) const;

  QString analysisConfigJson(const QString& type, const QString& plotType,
                             const QString& plotMeasure,
                             const QStringList& propertyIds,
                             const QStringList& contractTypes,
                             double taxPercent) const;
  /**
   * @brief Parses analysis configuration JSON into a UI map.
   * @param configJson Serialized analysis configuration.
   * @return Parsed configuration map or an empty map.
   */
  QVariantMap analysisConfig(const QString& configJson) const;
  /**
   * @brief Parses serialized export state into a UI map.
   * @param exportStateJson Serialized export state.
   * @return Parsed export state or an empty map.
   */
  QVariantMap exportState(const QString& exportStateJson) const;
  /**
   * @brief Normalizes serialized export state to a compact JSON object string.
   * @param exportStateJson Serialized export state.
   * @return Compact JSON object string or "{}".
   */
  QString normalizedExportStateJson(const QString& exportStateJson) const;
  /**
   * @brief Serializes export state for workspace persistence.
   * @param exportState Export state map.
   * @return Compact JSON object string or "{}".
   */
  QString normalizedExportStateJson(const QVariantMap& exportState) const;

  QString analysisAdjustmentsJson(const QVariantList& transactions,
                                  const QStringList& selectedTransactionIds,
                                  double taxPercent) const;
  /**
   * @brief Serializes UI adjustment amounts for workspace and runner calls.
   * @param adjustments Adjustment amount map keyed by transaction id.
   * @return Compact JSON object string.
   */
  QString analysisAdjustmentsJson(const QVariantMap& adjustments) const;

  QString analysisAdjustmentsJsonFromPercentText(
      const QVariantList& transactions,
      const QVariantList& selectedTransactionIds,
      const QString& percentText) const;

  QVariantMap computeAnalysisPreview(const QString& analysisId,
                                     const QString& filterSpecification,
                                     bool includeAdjustments,
                                     const QString& adjustmentsJson) const;
  /**
   * @brief Parses analysis adjustments JSON into a UI map.
   * @param adjustmentsJson Serialized adjustment map.
   * @return Parsed adjustment map or an empty map.
   */
  QVariantMap analysisAdjustments(const QString& adjustmentsJson) const;

  QVariantMap previewTransactions(const QString& filterSpec) const;

  QStringList contractTypes() const;

  static QString plotTypeFromSubtypeIndex(int plotSubtypeIndex);
  /**
   * @brief Parses a serialized transaction snapshot into UI rows.
   * @param snapshotTransactionsJson Serialized transaction snapshot.
   * @return Transaction snapshot rows.
   */
  QVariantList transactionSnapshot(const QString& snapshotTransactionsJson) const;
  QString transactionSnapshotJson(const QVariantList& transactions) const;
  QVariantList adjustmentIds(const QVariantMap& adjustments) const;
  QString renderedPreviewSource(const QVariantMap& analysisResult,
                                int revision) const;

private:
  core::ports::analysis::AnalysisRequest
  analysisRequest(const QString& analysisId,
                  const QString& filterSpecification) const;
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter_;
};

} // namespace ui
