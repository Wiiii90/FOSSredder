/**
 * @file core/src/export/ExportService.cpp
 * @brief Dispatches export requests to format-specific exporters.
 */

#include "core/application/export/ExportService.h"

#include "ObjectExportExecutor.h"
#include "core/application/export/CsvExporter.h"
#include "core/application/export/XlsxExporter.h"
#include "core/application/workspace/WorkspaceSnapshotCatalogMapper.h"
#include "core/constants/export.h"

#include <set>
#include <unordered_map>

namespace core::application::exporting {

namespace export_ports = core::ports::exporting;

namespace {

std::string dedupeKey(const export_ports::ExportObjectRequest &item) {
  return item.annualId + "|" + item.objectId + "|" +
         std::to_string(static_cast<int>(item.format));
}

export_ports::AnalysisExportFormat
analysisFormatFromString(const std::string &value) {
  if (value == "xlsx")
    return export_ports::AnalysisExportFormat::Xlsx;
  if (value == "jpg" || value == "jpeg")
    return export_ports::AnalysisExportFormat::Jpg;
  if (value == "png")
    return export_ports::AnalysisExportFormat::Png;
  return export_ports::AnalysisExportFormat::Csv;
}

std::vector<export_ports::ExportObjectRequest>
expandObjectRequests(const core::domain::catalog::WorkspaceCatalog &state,
                     const export_ports::ExportRequest &request) {
  std::vector<export_ports::ExportObjectRequest> out;
  std::unordered_map<std::string, std::shared_ptr<core::domain::Analysis>>
      analysisById;
  for (const auto &analysis : state.analyses()) {
    if (!analysis || analysis->id().empty())
      continue;
    analysisById.emplace(analysis->id(), analysis);
  }

  std::set<std::string> seen;
  for (const auto &item : request.objectRequests) {
    if (item.type == export_ports::ExportObjectType::Analysis) {
      if (!item.objectId.empty()) {
        const std::string key = dedupeKey(item);
        if (seen.find(key) != seen.end())
          continue;
        seen.insert(key);
        out.push_back(item);
      }
      continue;
    }

    for (const auto &annual : state.annuals()) {
      if (!annual || annual->id() != item.objectId)
        continue;
      for (const auto &analysisId : annual->analysisIds()) {
        const auto it = analysisById.find(analysisId);
        if (it == analysisById.end() || !it->second)
          continue;
        export_ports::ExportObjectRequest expanded;
        expanded.type = export_ports::ExportObjectType::Analysis;
        expanded.objectId = analysisId;
        expanded.annualId = annual->id();
        expanded.name = it->second->name();
        expanded.format = analysisFormatFromString(it->second->exportFormat());
        const std::string key = dedupeKey(expanded);
        if (seen.find(key) != seen.end())
          continue;
        seen.insert(key);
        out.push_back(std::move(expanded));
      }
      break;
    }
  }

  return out;
}

} // namespace

ExportService::ExportService(
    std::shared_ptr<core::ports::archive::IArchive> archive,
    std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter,
    std::shared_ptr<
        core::ports::analysis_image_renderer::IAnalysisImageRenderer>
        imageRenderer)
    : archive_(std::move(archive)), xlsxWriter_(std::move(xlsxWriter)),
      imageRenderer_(std::move(imageRenderer)) {}

export_ports::ExportResult
ExportService::runExport(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    export_ports::ExportRequest request) const {
  return exportData(core::application::workspace::toWorkspaceCatalog(workspace),
                    request);
}

export_ports::ExportResult ExportService::exportData(
    const core::domain::catalog::WorkspaceCatalog &state,
    const export_ports::ExportRequest &request) const {
  if (!request.objectRequests.empty()) {
    auto normalized = request;
    normalized.objectRequests = expandObjectRequests(state, request);
    if (normalized.progressCallback) {
      normalized.progressCallback(0.05, "Preparing export");
      normalized.progressCallback(0.20, "Resolving export objects");
    }
    auto result = exportObjectRequests(state, normalized, archive_, xlsxWriter_,
                                       imageRenderer_);
    if (normalized.progressCallback) {
      normalized.progressCallback(result.success ? 1.0 : 0.95,
                                  result.success ? "Finished" : "Failed");
    }
    return result;
  }

  switch (request.format) {
  case export_ports::ExportFormat::Csv:
    return CsvExporter{}.exportData(state, request);
  case export_ports::ExportFormat::Xlsx:
    return XlsxExporter{xlsxWriter_}.exportData(state, request);
  }

  return export_ports::ExportResult{
      false,
      export_ports::ExportStatus::UnsupportedFormat,
      request.format,
      {},
      std::string(core::constants::exportFlow::kErrorUnsupportedFormat),
      std::string(core::constants::exportFlow::kMessageUnsupportedFormat)};
}

} // namespace core::application::exporting
