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

#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {
class AnalysisAdapter;
}

namespace ui {

/**
 * @brief Coordinates analysis preview and computation use cases for UI state.
 */
class AnalysisWorkflow : public QObject {
  Q_OBJECT
public:
  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;

  /**
   * @brief Creates an analysis workflow.
   * @param stateSnapshotProvider Provider for the current workspace snapshot.
   * @param analysisAdapter Adapter used to invoke the core analysis runner.
   * @param errorReporter Error reporter used for workflow diagnostics.
   * @param parent Optional Qt parent.
   */
  explicit AnalysisWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter,
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
      QObject* parent = nullptr);

  /**
   * @brief Builds a serialized filter specification from UI fields.
   * @param dateField Selected date field.
   * @param dateMode Selected date mode.
   * @param year Selected year text.
   * @param dateFrom Start date text.
   * @param dateTo End date text.
   * @param propertyIds Selected property ids.
   * @param contractTypes Selected contract types.
   * @param allocatableMode Selected allocatable mode.
   * @return Serialized filter specification.
   */
  QString analysisFilterSpec(const QString& dateField, const QString& dateMode,
                             const QString& year, const QString& dateFrom,
                             const QString& dateTo,
                             const QStringList& propertyIds,
                             const QStringList& contractTypes,
                             const QString& allocatableMode) const;
  /**
   * @brief Builds adjustment amounts from selected transactions and percent
   * text.
   * @param transactions Preview transaction rows containing id and amount.
   * @param selectedTransactionIds Transaction ids selected for adjustment.
   * @param percentText User-entered percent text.
   * @return Adjustment amount map keyed by transaction id.
   */
  QVariantMap analysisAdjustmentAmountsFromPercentText(
      const QVariantList& transactions,
      const QStringList& selectedTransactionIds,
      const QString& percentText) const;

  /**
   * @brief Computes the current analysis preview.
   * @param analysisId Analysis id.
   * @param filterSpec Serialized filter specification.
   * @param includeAdjustments Whether calculation adjustments are included.
   * @param adjustmentAmounts Adjustment amount map keyed by transaction id.
   * @return QML analysis result payload.
   */
  QVariantMap
  computeAnalysisPreview(const QString& analysisId, const QString& filterSpec,
                         bool includeAdjustments,
                         const QVariantMap& adjustmentAmounts) const;
  /**
   * @brief Builds a transaction preview for the current filter.
   * @param filterSpec Serialized filter specification.
   * @return QML preview payload.
   */
  QVariantMap previewTransactions(const QString& filterSpec) const;

  /**
   * @brief Returns contract types available in the current workspace snapshot.
   * @return Contract type labels.
   */
  QStringList contractTypes() const;

private:
  /**
   * @brief Returns the current workspace snapshot.
   * @return Workspace snapshot or an empty snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter_;
  std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

} // namespace ui
