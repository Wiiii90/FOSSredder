/**
 * @file ui/src/workflows/AnalysisWorkflow.cpp
 * @brief Orchestrates analysis UI flows via core helpers and the analysis adapter.
 */

#include "ui/workflows/AnalysisWorkflow.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/i18n/Text.h"
#include "ui/util/StringConversions.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QVariant>

namespace ui {
namespace {

constexpr auto kPie = "pie";
constexpr auto kHistogram = "histogram";

std::optional<double> parsePercentInput(const QString& text) {
  QString normalized = text.trimmed();
  if (normalized.isEmpty()) {
    return std::nullopt;
  }

  if (normalized.endsWith(QLatin1Char('%'))) {
    normalized.chop(1);
    normalized = normalized.trimmed();
  }
  normalized.remove(QLatin1Char(' '));

  bool ok = false;
  const double direct = normalized.toDouble(&ok);
  if (ok && std::isfinite(direct)) {
    return direct;
  }

  const int lastComma = normalized.lastIndexOf(QLatin1Char(','));
  const int lastDot = normalized.lastIndexOf(QLatin1Char('.'));
  if (lastComma >= 0 && lastDot >= 0) {
    if (lastComma > lastDot) {
      normalized.remove(QLatin1Char('.'));
      normalized.replace(QLatin1Char(','), QLatin1Char('.'));
    } else {
      normalized.remove(QLatin1Char(','));
    }
  } else if (lastComma >= 0) {
    normalized.replace(QLatin1Char(','), QLatin1Char('.'));
  }

  const double parsed = normalized.toDouble(&ok);
  if (ok && std::isfinite(parsed)) {
    return parsed;
  }
  return std::nullopt;
}

QStringList stringList(const QVariantList& values) {
  QStringList out;
  out.reserve(values.size());
  for (const QVariant& value : values) {
    const QString string = value.toString();
    if (!string.isEmpty()) {
      out.push_back(string);
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
  return analysisAdapter_
             ? analysisAdapter_->buildFilterSpec(
                   dateField, dateMode, year, dateFrom, dateTo, propertyIds,
                   contractTypes, allocatableMode)
             : QString{};
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
  return analysisAdapter_
             ? analysisAdapter_->buildAnalysisConfigJson(
                   type, plotType, plotMeasure, propertyIds, contractTypes,
                   taxPercent)
             : QString{};
}

QVariantMap AnalysisWorkflow::analysisConfig(const QString &configJson) const {
  const QJsonDocument document = QJsonDocument::fromJson(configJson.toUtf8());
  return document.isObject() ? document.object().toVariantMap() : QVariantMap{};
}

QVariantMap AnalysisWorkflow::exportState(const QString &exportStateJson) const {
  const QJsonDocument document =
      QJsonDocument::fromJson(exportStateJson.toUtf8());
  return document.isObject() ? document.object().toVariantMap() : QVariantMap{};
}

QString AnalysisWorkflow::normalizedExportStateJson(
    const QString &exportStateJson) const {
  return normalizedExportStateJson(exportState(exportStateJson));
}

QString AnalysisWorkflow::normalizedExportStateJson(
    const QVariantMap &exportState) const {
  return QString::fromUtf8(QJsonDocument::fromVariant(exportState)
                               .toJson(QJsonDocument::Compact));
}

QString AnalysisWorkflow::analysisAdjustmentsJson(
    const QVariantList &transactions, const QStringList &selectedTransactionIds,
    double taxPercent) const {
  return QString::fromStdString(
      analysisAdapter_ ? analysisAdapter_->buildAnalysisAdjustmentsJson(
                             transactions, selectedTransactionIds, taxPercent)
                       : std::string("{}"));
}

QString AnalysisWorkflow::analysisAdjustmentsJson(
    const QVariantMap &adjustments) const {
  return QString::fromUtf8(QJsonDocument::fromVariant(adjustments)
                               .toJson(QJsonDocument::Compact));
}

QString AnalysisWorkflow::analysisAdjustmentsJsonFromPercentText(
    const QVariantList &transactions, const QVariantList &selectedTransactionIds,
    const QString &percentText) const {
  if (selectedTransactionIds.isEmpty()) {
    return QStringLiteral("{}");
  }
  const double taxPercent = parsePercentInput(percentText).value_or(0.0);
  return analysisAdjustmentsJson(transactions,
                                 stringList(selectedTransactionIds), taxPercent);
}

QString AnalysisWorkflow::plotTypeFromSubtypeIndex(int plotSubtypeIndex) {
  return plotSubtypeIndex == 1 ? QString::fromLatin1(kHistogram)
                               : QString::fromLatin1(kPie);
}

QVariantList AnalysisWorkflow::transactionSnapshot(
    const QString &snapshotTransactionsJson) const {
  const QJsonDocument document =
      QJsonDocument::fromJson(snapshotTransactionsJson.toUtf8());
  return document.isArray() ? document.array().toVariantList() : QVariantList{};
}

QString AnalysisWorkflow::transactionSnapshotJson(
    const QVariantList &transactions) const {
  QJsonArray snapshot;
  for (const QVariant &value : transactions) {
    snapshot.push_back(QJsonObject::fromVariantMap(value.toMap()));
  }
  return QString::fromUtf8(
      QJsonDocument(snapshot).toJson(QJsonDocument::Compact));
}

QVariantList AnalysisWorkflow::adjustmentIds(
    const QVariantMap &adjustments) const {
  QVariantList ids;
  ids.reserve(adjustments.size());
  for (auto it = adjustments.cbegin(); it != adjustments.cend(); ++it) {
    if (!it.key().trimmed().isEmpty()) {
      ids.push_back(it.key());
    }
  }
  return ids;
}

QString AnalysisWorkflow::renderedPreviewSource(
    const QVariantMap &analysisResult, int revision) const {
  const QVariantList artifacts =
      analysisResult.value(QStringLiteral("artifacts")).toList();
  if (artifacts.isEmpty()) {
    return {};
  }

  QString renderedSource;
  const QString rawPath = artifacts.first().toString().trimmed();
  if (rawPath.startsWith(QStringLiteral("file:")) ||
      rawPath.startsWith(QStringLiteral("qrc:")) ||
      rawPath.startsWith(QStringLiteral("http:")) ||
      rawPath.startsWith(QStringLiteral("https:")) ||
      rawPath.startsWith(QStringLiteral("data:"))) {
    renderedSource = rawPath;
  } else if (!rawPath.isEmpty()) {
    renderedSource = QUrl::fromLocalFile(rawPath).toString();
  }
  if (renderedSource.isEmpty() ||
      renderedSource.startsWith(QStringLiteral("data:")) ||
      renderedSource.startsWith(QStringLiteral("qrc:"))) {
    return renderedSource;
  }

  QUrl previewUrl(renderedSource);
  previewUrl.setQuery(QStringLiteral("v=%1").arg(revision));
  return previewUrl.toString();
}

QVariantMap AnalysisWorkflow::computeAnalysisPreview(
    const QString &analysisId, const QString &filterSpecification,
    bool includeAdjustments, const QString &adjustmentsJson) const {
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
    analysisAdapter_->applyAnalysisPreviewOverrides(
        snapshot, analysisId.trimmed().toStdString(), includeAdjustments,
        adjustmentsJson.toStdString());

    const auto result = analysisAdapter_->runAnalysis(
        snapshot, analysisRequest(analysisId, filterSpecification));
    if (!result.found) {
      return {};
    }
    QVariantMap payload = analysisAdapter_->mapAnalysisResult(result);
    payload.insert(
        QStringLiteral("tableState"),
        analysisAdapter_->mapAnalysisTableState(
            analysisAdapter_->projectTableState(
                result, adjustmentsJson.toStdString(), includeAdjustments,
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
AnalysisWorkflow::analysisAdjustments(const QString &adjustmentsJson) const {
  const QJsonDocument document = QJsonDocument::fromJson(adjustmentsJson.toUtf8());
  return document.isObject() ? document.object().toVariantMap() : QVariantMap{};
}

QVariantMap AnalysisWorkflow::previewTransactions(
    const QString &filterSpecification) const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  observability::traceWorkflow(
      observability::origins::workflow::analysis::kPreview,
      "Analysis transaction preview submitted");
  return analysisAdapter_->mapPreviewResult(analysisAdapter_->previewTransactions(
      stateSnapshot(), strings::toStdString(filterSpecification)));
}

QStringList AnalysisWorkflow::contractTypes() const {
  if (!analysisAdapter_ || !stateSnapshotProvider_) {
    return {};
  }
  return analysisAdapter_->mapContractTypes(
      analysisAdapter_->contractTypes(stateSnapshot()));
}

core::ports::workspace::WorkspaceSnapshot AnalysisWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
