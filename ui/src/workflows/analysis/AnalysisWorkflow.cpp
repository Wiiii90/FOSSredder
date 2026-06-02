/**
 * @file ui/src/workflows/analysis/AnalysisWorkflow.cpp
 * @brief Orchestrates analysis UI flows via core helpers and the analysis adapter.
 */

#include "ui/workflows/analysis/AnalysisWorkflow.h"

#include "core/application/analysis/AnalysisWorkflowSupport.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/ports/analysis/AnalysisRequest.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/text/Text.h"
#include "ui/shared/util/StringConversions.h"

#include <utility>

namespace ui {
namespace {

void reportMissingAnalysisService() {
  core::errors::report(
      core::errors::ErrorSeverity::Warning, core::errors::codes::GenericError,
      observability::origins::workflow::analysis::kCompute,
      ui::text::workflowErrors::analysisEngineUnavailable().toStdString());
}

void reportMissingAnalysisState() {
  core::errors::report(
      core::errors::ErrorSeverity::Warning, core::errors::codes::GenericError,
      observability::origins::workflow::analysis::kCompute,
      ui::text::workflowErrors::analysisViewModelUnavailable().toStdString());
}

std::vector<std::string> toStdStringList(const QStringList &values) {
  std::vector<std::string> out;
  out.reserve(values.size());
  for (const auto &value : values) {
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty()) {
      out.push_back(trimmed.toStdString());
    }
  }
  return out;
}

std::vector<core::application::analysis::AnalysisAdjustmentTransactionInput>
toAdjustmentTransactions(const QVariantList &transactions) {
  std::vector<core::application::analysis::AnalysisAdjustmentTransactionInput>
      out;
  out.reserve(static_cast<std::size_t>(transactions.size()));
  for (const auto &value : transactions) {
    const QVariantMap row = value.toMap();
    core::application::analysis::AnalysisAdjustmentTransactionInput item;
    item.id = row.value(QStringLiteral("id")).toString().trimmed().toStdString();
    item.amount = row.value(QStringLiteral("amount")).toDouble();
    if (!item.id.empty()) {
      out.push_back(std::move(item));
    }
  }
  return out;
}

} // namespace

AnalysisWorkflow::AnalysisWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::adapters::AnalysisAdapter> analysisAdapter,
    QObject *parent)
    : QObject(parent), stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      analysisAdapter_(std::move(analysisAdapter)) {}

core::ports::analysis::AnalysisRequest
AnalysisWorkflow::analysisRequest(const QString &analysisId,
                                  const QString &filterSpecification) const {
  core::ports::analysis::AnalysisRequest request;
  request.analysisId = analysisId.trimmed().toStdString();
  request.filterSpecification = filterSpecification.trimmed().toStdString();
  return request;
}

QString AnalysisWorkflow::analysisFilterSpec(
    const QString &dateField, const QString &dateMode, const QString &year,
    const QString &dateFrom, const QString &dateTo,
    const QStringList &propertyIds, const QStringList &contractTypes,
    const QString &allocatableMode) const {
  const auto selection = core::application::analysis::filterSelectionFromFields(
      dateField.trimmed().toLower().toStdString(),
      dateMode.trimmed().toLower().toStdString(), year.trimmed().toStdString(),
      dateFrom.trimmed().toStdString(), dateTo.trimmed().toStdString(),
      toStdStringList(propertyIds), toStdStringList(contractTypes),
      allocatableMode.trimmed().toLower().toStdString());
  return QString::fromStdString(
      core::ports::analysis::buildAnalysisFilterSpec(selection));
}

QVariantMap
AnalysisWorkflow::parseAnalysisFilterSpec(const QString &filterSpec) const {
  if (!analysisAdapter_) {
    return {};
  }
  return analysisAdapter_->mapFilterSelection(
      core::ports::analysis::parseAnalysisFilterSelection(
          filterSpec.toStdString()));
}

QString AnalysisWorkflow::analysisConfigJson(const QString &type,
                                             const QString &plotType,
                                             const QString &plotMeasure,
                                             const QStringList &propertyIds,
                                             const QStringList &contractTypes,
                                             double taxPercent) const {
  core::application::analysis::AnalysisConfigInput input;
  input.type = type.trimmed().toLower().toStdString();
  input.plotType = plotType.trimmed().toStdString();
  input.plotMeasure = plotMeasure.trimmed().toStdString();
  input.propertyIds = toStdStringList(propertyIds);
  input.contractTypes = toStdStringList(contractTypes);
  input.taxPercent = taxPercent;
  return QString::fromStdString(
      core::application::analysis::buildAnalysisConfigJson(input));
}

QString AnalysisWorkflow::analysisAdjustmentsJson(
    const QVariantList &transactions, const QStringList &selectedTransactionIds,
    double taxPercent) const {
  return QString::fromStdString(
      core::application::analysis::buildAnalysisAdjustmentsJson(
          toAdjustmentTransactions(transactions),
          toStdStringList(selectedTransactionIds), taxPercent));
}

QVariantMap AnalysisWorkflow::computeAnalysisPreview(
    const QString &analysisId, const QString &filterSpecification,
    bool includeCalcAdjustments, const QString &adjustmentsJson) const {
  if (!analysisAdapter_) {
    reportMissingAnalysisService();
    return {};
  }
  if (!stateSnapshotProvider_) {
    reportMissingAnalysisState();
    return {};
  }

  try {
    auto snapshot = stateSnapshot();
    core::application::analysis::applyAnalysisPreviewOverrides(
        snapshot, analysisId.trimmed().toStdString(), includeCalcAdjustments,
        core::application::analysis::parseAnalysisAdjustmentsJson(
            adjustmentsJson.toStdString()));

    const auto result = analysisAdapter_->runAnalysis(
        snapshot, analysisRequest(analysisId, filterSpecification));
    if (!result.found) {
      return {};
    }
    return analysisAdapter_->mapAnalysisResult(result);
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionError,
        observability::origins::workflow::analysis::kCompute,
        std::current_exception());
  }
  return {};
}

QVariantMap AnalysisWorkflow::previewTransactions(
    const QString &filterSpecification) const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  const auto snapshot = stateSnapshot();
  const auto filtered = analysisAdapter_->previewTransactions(
      snapshot, strings::toStdString(filterSpecification));
  return analysisAdapter_->mapPreviewTransactions(snapshot, filtered);
}

QStringList AnalysisWorkflow::contractTypes() const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  return analysisAdapter_->mapContractTypes(
      core::application::analysis::contractTypesFromSnapshot(stateSnapshot()));
}

core::ports::workspace::WorkspaceSnapshot AnalysisWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
