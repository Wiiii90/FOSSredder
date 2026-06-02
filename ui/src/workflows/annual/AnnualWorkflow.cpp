/**
 * @file ui/src/workflows/annual/AnnualWorkflow.cpp
 * @brief Orchestrates annual UI flows via core helpers and the annual adapter.
 */

#include "ui/workflows/annual/AnnualWorkflow.h"

#include "core/application/annual/AnnualWorkflowSupport.h"
#include "core/ports/annual/AnnualRequest.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/shared/util/StringConversions.h"

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
  try {
    const QString previewId =
        QString::fromLatin1(core::application::annual::kPreviewAnnualId);
    const QString resolvedAnnualId =
        annualId.trimmed().isEmpty() ? previewId : annualId.trimmed();

    std::vector<std::string> analysisIdsStd;
    analysisIdsStd.reserve(analysisIds.size());
    for (const auto &id : analysisIds) {
      analysisIdsStd.push_back(strings::toStdString(id));
    }

    const auto snapshot = core::application::annual::withPreviewAnnual(
        stateSnapshot(), strings::toStdString(resolvedAnnualId), analysisIdsStd,
        year);

    core::ports::annual::AnnualRequest request;
    request.annualId = strings::toStdString(resolvedAnnualId);
    return annualAdapter_->mapAnnualResult(
        annualAdapter_->runAnnual(snapshot, request));
  } catch (...) {
    return {};
  }
}

core::ports::workspace::WorkspaceSnapshot AnnualWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
