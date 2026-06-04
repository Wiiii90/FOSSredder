/**
 * @file ui/src/workflows/AnnualWorkflow.cpp
 * @brief Orchestrates annual UI flows via core helpers and the annual adapter.
 */

#include "ui/workflows/AnnualWorkflow.h"

#include "core/ports/usecases/annual/AnnualRequest.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"

#include <utility>

namespace ui {

AnnualWorkflow::AnnualWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::adapters::AnnualAdapter> annualAdapter, QObject *parent)
    : QObject(parent), stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      annualAdapter_(std::move(annualAdapter)) {}

QVariantMap AnnualWorkflow::computeAnnual(const QString &annualId) const {
  if (!annualAdapter_ || annualId.trimmed().isEmpty()) {
    return {};
  }
  observability::traceWorkflow(
      observability::origins::workflow::annual::kCompute,
      "Annual compute submitted",
      {{"annualId", annualId.trimmed().toStdString()}});
  try {
    core::ports::annual::AnnualRequest request;
    request.annualId = strings::toStdString(annualId.trimmed());
    return annualAdapter_->mapAnnualResult(
        annualAdapter_->runAnnual(stateSnapshot(), request));
  } catch (...) {
    return {};
  }
}

QVariantMap AnnualWorkflow::computeAnnualPreview(const QString &annualId,
                                               const QStringList &analysisIds,
                                               int year) const {
  if (!annualAdapter_) {
    return {};
  }
  observability::traceWorkflow(
      observability::origins::workflow::annual::kPreview,
      "Annual preview compute submitted",
      {{"annualId", annualId.trimmed().toStdString()},
       {observability::context::kCount, std::to_string(analysisIds.size())}});
  try {
    core::ports::annual::AnnualRequest request;
    request.annualId = strings::toStdString(annualId.trimmed());
    request.previewYear = year;
    request.previewAnalysisIds.reserve(analysisIds.size());
    for (const auto &id : analysisIds) {
      request.previewAnalysisIds.push_back(strings::toStdString(id));
    }
    return annualAdapter_->mapAnnualResult(
        annualAdapter_->runAnnual(stateSnapshot(), request));
  } catch (...) {
    return {};
  }
}

core::ports::workspace::WorkspaceSnapshot AnnualWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
