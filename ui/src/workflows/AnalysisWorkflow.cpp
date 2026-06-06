/**
 * @file ui/src/workflows/AnalysisWorkflow.cpp
 * @brief Orchestrates analysis UI flows via core helpers and the analysis
 * adapter.
 */

#include "ui/workflows/AnalysisWorkflow.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/i18n/Text.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"

#include <utility>

namespace ui {

AnalysisWorkflow::AnalysisWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter,
    QObject* parent)
    : QObject(parent), stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      analysisAdapter_(std::move(analysisAdapter)) {}

QString AnalysisWorkflow::analysisFilterSpec(
    const QString& dateField, const QString& dateMode, const QString& year,
    const QString& dateFrom, const QString& dateTo,
    const QStringList& propertyIds, const QStringList& contractTypes,
    const QString& allocatableMode) const {
  return analysisAdapter_
             ? analysisAdapter_->buildFilterSpec(dateField, dateMode, year,
                                                 dateFrom, dateTo, propertyIds,
                                                 contractTypes, allocatableMode)
             : QString{};
}

QVariantMap AnalysisWorkflow::analysisAdjustmentAmountsFromPercentText(
    const QVariantList& transactions, const QStringList& selectedTransactionIds,
    const QString& percentText) const {
  if (selectedTransactionIds.isEmpty()) {
    return {};
  }
  const double taxPercent =
      analysisAdapter_ ? analysisAdapter_->parseAnalysisPercentText(percentText)
                       : 0.0;
  return analysisAdapter_
             ? analysisAdapter_->buildAnalysisAdjustmentAmounts(
                   transactions, selectedTransactionIds, taxPercent)
             : QVariantMap{};
}

QVariantMap AnalysisWorkflow::computeAnalysisPreview(
    const QString& analysisId, const QString& filterSpec,
    bool includeAdjustments, const QVariantMap& adjustmentAmounts) const {
  if (!analysisAdapter_) {
    core::errors::report(
        core::errors::ErrorSeverity::Warning, core::errors::codes::GenericError,
        observability::origins::workflow::analysis::kCompute,
        ui::text::workflowErrors::analysisEngineUnavailable().toStdString());
    return {};
  }
  if (!stateSnapshotProvider_) {
    core::errors::report(
        core::errors::ErrorSeverity::Warning, core::errors::codes::GenericError,
        observability::origins::workflow::analysis::kCompute,
        ui::text::workflowErrors::analysisViewModelUnavailable().toStdString());
    return {};
  }

  observability::traceWorkflow(
      observability::origins::workflow::analysis::kCompute,
      "Analysis preview compute submitted",
      {{"analysisId", analysisId.trimmed().toStdString()}});

  try {
    auto snapshot = stateSnapshot();
    const auto adjustments =
        analysisAdapter_->adjustmentAmountsFromMap(adjustmentAmounts);
    analysisAdapter_->applyAnalysisPreviewOverrides(
        snapshot, analysisId.trimmed().toStdString(), includeAdjustments,
        adjustments);

    const auto result = analysisAdapter_->runAnalysis(
        snapshot,
        analysisAdapter_->buildAnalysisRequest(analysisId, filterSpec));
    if (!result.found) {
      return {};
    }
    QVariantMap payload = analysisAdapter_->mapAnalysisResult(result);
    payload.insert(
        QStringLiteral("tableState"),
        analysisAdapter_->mapAnalysisTableState(
            analysisAdapter_->projectTableState(
                result, adjustments, includeAdjustments,
                ui::text::analysis::unassignedContractType().toStdString())));
    return payload;
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionError,
        observability::origins::workflow::analysis::kCompute,
        std::current_exception());
  }
  return {};
}

QVariantMap
AnalysisWorkflow::previewTransactions(const QString& filterSpec) const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  observability::traceWorkflow(
      observability::origins::workflow::analysis::kPreview,
      "Analysis transaction preview submitted");
  return analysisAdapter_->mapPreviewResult(
      analysisAdapter_->previewTransactions(stateSnapshot(),
                                            strings::toStdString(filterSpec)));
}

QStringList AnalysisWorkflow::contractTypes() const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  return analysisAdapter_->mapContractTypes(
      analysisAdapter_->contractTypes(stateSnapshot()));
}

core::ports::workspace::WorkspaceSnapshot
AnalysisWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
