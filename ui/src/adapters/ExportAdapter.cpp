/**
 * @file ui/src/adapters/ExportAdapter.cpp
 * @brief Implements the export adapter used by UI workflows.
 */

#include "ui/adapters/ExportAdapter.h"

#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/shell/QmlContracts.h"

#include <QVariantList>

#include <utility>

namespace ui::adapters {
namespace exportKeys = ui::payload::keys::exportSelection;

namespace {

inline constexpr auto kRunnerUnavailableCode = "runnerUnavailable";
inline constexpr auto kRunnerUnavailableMessage = "Export runner unavailable.";

core::ports::exporting::AnalysisExportFormat
analysisExportFormatFromString(const QString& value) {
  const QString normalized = value.trimmed().toLower();
  if (normalized == QStringLiteral("xlsx"))
    return core::ports::exporting::AnalysisExportFormat::Xlsx;
  if (normalized == QStringLiteral("jpg") ||
      normalized == QStringLiteral("jpeg"))
    return core::ports::exporting::AnalysisExportFormat::Jpg;
  if (normalized == QStringLiteral("png"))
    return core::ports::exporting::AnalysisExportFormat::Png;
  return core::ports::exporting::AnalysisExportFormat::Csv;
}

} // namespace

ExportAdapter::ExportAdapter(
    std::shared_ptr<core::ports::exporting::IExportRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::exporting::ExportResult ExportAdapter::runExport(
    const core::ports::workspace::WorkspaceSnapshot& workspace,
    core::ports::exporting::ExportRequest request) const {
  observability::traceAdapter(
      "ExportAdapter::runExport", "Export runner invoked",
      {{observability::context::kPath, request.outputPath},
       {"objectCount", std::to_string(request.objectRequests.size())}});
  if (!runner_) {
    return {false,
            core::ports::exporting::ExportStatus::InternalError,
            request.format,
            request.outputPath,
            std::string(kRunnerUnavailableCode),
            std::string(kRunnerUnavailableMessage)};
  }

  return runner_->runExport(workspace, std::move(request));
}

core::ports::exporting::ExportFormat
ExportAdapter::exportFormatFromQmlIndex(int formatIndex) const {
  switch (static_cast<ui::qml::contracts::ExportFormat>(formatIndex)) {
    case ui::qml::contracts::ExportFormat::Csv:
      return core::ports::exporting::ExportFormat::Csv;
    case ui::qml::contracts::ExportFormat::Xlsx:
      return core::ports::exporting::ExportFormat::Xlsx;
  }
  return core::ports::exporting::ExportFormat::Csv;
}

void ExportAdapter::applySelectionPayload(
    core::ports::exporting::ExportRequest& request,
    const QVariantMap& selectionPayload) const {
  if (selectionPayload.isEmpty()) {
    return;
  }

  const int packageFormatIndex =
      selectionPayload.value(exportKeys::kPackageFormatIndex).toInt();
  request.packageFormat = packageFormatIndex == 1
                              ? core::ports::exporting::PackageFormat::Zip
                              : core::ports::exporting::PackageFormat::None;

  const QVariantList items =
      selectionPayload.value(exportKeys::kItems).toList();
  request.objectRequests.clear();
  request.objectRequests.reserve(static_cast<std::size_t>(items.size()));
  for (const QVariant& value : items) {
    const QVariantMap item = value.toMap();
    if (item.isEmpty()) {
      continue;
    }
    const QString objectId = item.value(exportKeys::kObjectId).toString();
    if (objectId.isEmpty()) {
      continue;
    }

    core::ports::exporting::ExportObjectRequest objectRequest;
    objectRequest.objectId = objectId.toStdString();
    objectRequest.name =
        item.value(exportKeys::kObjectName).toString().toStdString();
    objectRequest.annualId =
        item.value(exportKeys::kAnnualId).toString().toStdString();

    const QString objectType =
        item.value(exportKeys::kObjectType).toString().trimmed().toLower();
    objectRequest.type =
        objectType == exportKeys::kAnnual
            ? core::ports::exporting::ExportObjectType::Annual
            : core::ports::exporting::ExportObjectType::Analysis;
    objectRequest.format = analysisExportFormatFromString(
        item.value(exportKeys::kExportType).toString());
    request.objectRequests.push_back(std::move(objectRequest));
  }
}

core::ports::exporting::ExportRequest
ExportAdapter::buildExportRequest(int formatIndex, const QString& path,
                                  bool includeFormulas, const QString& locale,
                                  const QVariantMap& selectionPayload) const {
  core::ports::exporting::ExportRequest request;
  request.format = exportFormatFromQmlIndex(formatIndex);
  request.outputPath = path.toStdString();
  request.includeFormulas = includeFormulas;
  request.locale = locale.toStdString();
  applySelectionPayload(request, selectionPayload);
  return request;
}

} // namespace ui::adapters
