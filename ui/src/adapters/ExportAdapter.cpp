/**
 * @file ui/src/adapters/ExportAdapter.cpp
 * @brief Implements the export adapter used by UI workflows.
 */

#include "ui/adapters/ExportAdapter.h"

#include "core/constants/export.h"
#include "ui/shell/QmlContracts.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <utility>

namespace ui::adapters {
namespace {

core::ports::exporting::AnalysisExportFormat
analysisExportFormatFromString(const QString &value) {
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
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    core::ports::exporting::ExportRequest request) const {
  if (!runner_) {
    return {false,
            core::ports::exporting::ExportStatus::InternalError,
            request.format,
            request.outputPath,
            std::string(core::constants::exportFlow::errors::kRunnerUnavailable),
            std::string(
                core::constants::exportFlow::messages::kRunnerUnavailable)};
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
    core::ports::exporting::ExportRequest &request,
    const QString &selectionPayloadJson) const {
  if (selectionPayloadJson.isEmpty()) {
    return;
  }

  QJsonParseError parseError;
  const QJsonDocument payloadDoc =
      QJsonDocument::fromJson(selectionPayloadJson.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !payloadDoc.isObject()) {
    return;
  }

  const QJsonObject payloadObj = payloadDoc.object();
  const int packageFormatIndex =
      payloadObj.value(QStringLiteral("packageFormatIndex")).toInt(0);
  request.packageFormat = packageFormatIndex == 1
                              ? core::ports::exporting::PackageFormat::Zip
                              : core::ports::exporting::PackageFormat::None;

  const QJsonArray items = payloadObj.value(QStringLiteral("items")).toArray();
  request.objectRequests.clear();
  request.objectRequests.reserve(static_cast<std::size_t>(items.size()));
  for (const QJsonValue &value : items) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject item = value.toObject();
    const QString objectId =
        item.value(QStringLiteral("objectId")).toString();
    if (objectId.isEmpty()) {
      continue;
    }

    core::ports::exporting::ExportObjectRequest objectRequest;
    objectRequest.objectId = objectId.toStdString();
    objectRequest.name =
        item.value(QStringLiteral("objectName")).toString().toStdString();
    objectRequest.annualId =
        item.value(QStringLiteral("annualId")).toString().toStdString();

    const QString objectType =
        item.value(QStringLiteral("objectType")).toString().trimmed().toLower();
    objectRequest.type =
        objectType == QStringLiteral("annual")
            ? core::ports::exporting::ExportObjectType::Annual
            : core::ports::exporting::ExportObjectType::Analysis;
    objectRequest.format = analysisExportFormatFromString(
        item.value(QStringLiteral("exportType")).toString());
    request.objectRequests.push_back(std::move(objectRequest));
  }
}

core::ports::exporting::ExportRequest ExportAdapter::buildExportRequest(
    int formatIndex, const QString &path, bool includeFormulas,
    const QString &locale, const QString &selectionPayload) const {
  core::ports::exporting::ExportRequest request;
  request.format = exportFormatFromQmlIndex(formatIndex);
  request.outputPath = path.toStdString();
  request.includeFormulas = includeFormulas;
  request.locale = locale.toStdString();
  applySelectionPayload(request, selectionPayload);
  return request;
}

} // namespace ui::adapters
