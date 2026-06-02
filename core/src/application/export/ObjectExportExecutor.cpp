#include "ObjectExportExecutor.h"

#include "core/application/analysis/AnalysisService.h"
#include "core/constants/analysis.h"
#include "core/constants/export.h"
#include "core/ports/analysis-image-renderer/IAnalysisImageRenderer.h"
#include "core/ports/archive/IArchive.h"
#include "core/ports/xlsx-writer/IXlsxWriter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace {

using core::ports::exporting::ExportObjectRequest;
using core::ports::exporting::AnalysisExportFormat;
using core::ports::exporting::ExportObjectType;
using core::ports::exporting::ExportStatus;
using core::ports::exporting::PackageFormat;

std::string safeFilePart(const std::string &text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
    out.push_back(ok ? c : '_');
  }
  return out.empty() ? "entry" : out;
}

std::string extensionFor(AnalysisExportFormat format) {
  switch (format) {
  case AnalysisExportFormat::Csv:
    return "csv";
  case AnalysisExportFormat::Xlsx:
    return "xlsx";
  case AnalysisExportFormat::Jpg:
    return "jpg";
  case AnalysisExportFormat::Png:
    return "png";
  }
  return "dat";
}

bool writeCsvTable(const std::filesystem::path &outputPath,
                   const std::vector<std::vector<std::string>> &rows) {
  std::ofstream stream(outputPath, std::ios::binary);
  if (!stream)
    return false;
  const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  stream.write(reinterpret_cast<const char *>(bom), sizeof(bom));
  for (const auto &row : rows) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (i > 0)
        stream << ';';
      stream << row[i];
    }
    stream << '\n';
  }
  return !!stream;
}

std::vector<std::vector<std::string>>
normalizedRowsForExport(const core::ports::analysis::AnalysisResult &result) {
  std::vector<std::vector<std::string>> rows;
  if (result.type == core::constants::analysis::plotTypes::kPie) {
    rows.push_back({"Category", "Value"});
    for (const auto &row : result.table) {
      if (row.empty())
        continue;
      rows.push_back(row.size() == 1
                         ? std::vector<std::string>{row[0], "0"}
                         : std::vector<std::string>{row[0], row[1]});
    }
    return rows;
  }
  if (result.type == core::constants::analysis::plotTypes::kHistogram) {
    rows.push_back({"Month", "Total"});
    for (const auto &row : result.table) {
      if (row.empty())
        continue;
      std::string month = row[0];
      std::string totalValue = row.size() > 1 ? row[1] : "0";
      rows.push_back({month, totalValue});
    }
    return rows;
  }
  return result.table;
}

std::unordered_map<std::string, std::string>
propertyNameById(const core::domain::catalog::WorkspaceCatalog &state) {
  std::unordered_map<std::string, std::string> out;
  out.reserve(state.properties().size());
  for (const auto &property : state.properties()) {
    if (!property || property->id().empty())
      continue;
    out[property->id()] =
        property->name().empty() ? property->id() : property->name();
  }
  return out;
}

std::vector<std::vector<std::string>>
normalizedRowsForExport(const core::ports::analysis::AnalysisResult &result,
                        const core::domain::catalog::WorkspaceCatalog &state) {
  if (result.type != core::constants::analysis::kTypeTab) {
    return normalizedRowsForExport(result);
  }
  const std::string unassigned =
      std::string(core::constants::exportFlow::labels::kUnassigned);
  std::set<std::string> contractTypes;
  std::map<std::string, std::map<std::string, double>> amountsByProperty;
  const auto nameById = propertyNameById(state);

  for (const auto &tx : result.transactions) {
    const std::string contractType =
        !tx.contractType.empty() ? tx.contractType : unassigned;
    contractTypes.insert(contractType);
    if (tx.propertyIds.empty()) {
      amountsByProperty[unassigned][contractType] += tx.amount;
      continue;
    }
    bool hasMappedProperty = false;
    for (size_t i = 0; i < tx.propertyIds.size(); ++i) {
      const auto &propertyId = tx.propertyIds[i];
      if (propertyId.empty())
        continue;
      std::string propertyName;
      if (i < tx.propertyNames.size() && !tx.propertyNames[i].empty())
        propertyName = tx.propertyNames[i];
      else {
        const auto it = nameById.find(propertyId);
        propertyName = it != nameById.end() && !it->second.empty() ? it->second
                                                                   : propertyId;
      }
      if (propertyName.empty())
        continue;
      amountsByProperty[propertyName][contractType] += tx.amount;
      hasMappedProperty = true;
    }
    if (!hasMappedProperty)
      amountsByProperty[unassigned][contractType] += tx.amount;
  }

  auto formatAmount = [](double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
  };
  std::vector<std::string> orderedContractTypes(contractTypes.begin(),
                                                contractTypes.end());
  std::stable_sort(orderedContractTypes.begin(), orderedContractTypes.end(),
                   [&](const std::string &lhs, const std::string &rhs) {
                     const bool lhsUnassigned = lhs == unassigned;
                     const bool rhsUnassigned = rhs == unassigned;
                     if (lhsUnassigned != rhsUnassigned)
                       return !lhsUnassigned;
                     return lhs < rhs;
                   });
  std::vector<std::string> orderedProperties;
  for (const auto &[propertyName, _] : amountsByProperty)
    orderedProperties.push_back(propertyName);
  std::stable_sort(orderedProperties.begin(), orderedProperties.end(),
                   [&](const std::string &lhs, const std::string &rhs) {
                     const bool lhsUnassigned = lhs == unassigned;
                     const bool rhsUnassigned = rhs == unassigned;
                     if (lhsUnassigned != rhsUnassigned)
                       return !lhsUnassigned;
                     return lhs < rhs;
                   });

  std::vector<std::vector<std::string>> rows;
  std::vector<std::string> header;
  header.push_back(
      std::string(core::constants::exportFlow::labels::kPropertyHeader));
  for (const auto &contractType : orderedContractTypes)
    header.push_back(contractType);
  header.push_back(std::string(core::constants::exportFlow::labels::kTotal));
  rows.push_back(std::move(header));

  std::vector<double> totalsByContract(orderedContractTypes.size(), 0.0);
  double grandTotal = 0.0;
  for (const auto &propertyName : orderedProperties) {
    const auto &byContract = amountsByProperty[propertyName];
    std::vector<std::string> row;
    row.push_back(propertyName);
    double rowTotal = 0.0;
    for (size_t i = 0; i < orderedContractTypes.size(); ++i) {
      const auto it = byContract.find(orderedContractTypes[i]);
      const double value = it != byContract.end() ? it->second : 0.0;
      row.push_back(formatAmount(value));
      rowTotal += value;
      totalsByContract[i] += value;
    }
    row.push_back(formatAmount(rowTotal));
    rows.push_back(std::move(row));
    grandTotal += rowTotal;
  }
  std::vector<std::string> totalRow;
  totalRow.push_back(std::string(core::constants::exportFlow::labels::kTotal));
  for (const double value : totalsByContract)
    totalRow.push_back(formatAmount(value));
  totalRow.push_back(formatAmount(grandTotal));
  rows.push_back(std::move(totalRow));
  return rows;
}

std::unordered_map<std::string, std::string>
annualFolderNames(const core::domain::catalog::WorkspaceCatalog &state) {
  std::unordered_map<std::string, std::string> folderByAnnualId;
  std::set<std::string> usedFolderNames;
  for (const auto &annual : state.annuals()) {
    if (!annual || annual->id().empty())
      continue;
    const std::string baseName =
        safeFilePart(!annual->name().empty() ? annual->name() : annual->id());
    std::string candidate = baseName;
    int idx = 2;
    while (usedFolderNames.find(candidate) != usedFolderNames.end()) {
      candidate = baseName + "_" + std::to_string(idx++);
    }
    usedFolderNames.insert(candidate);
    folderByAnnualId[annual->id()] = candidate;
  }
  return folderByAnnualId;
}

std::filesystem::path outputPathForItem(
    const std::filesystem::path &baseOutput,
    const std::unordered_map<std::string, std::string> &folderByAnnualId,
    const ExportObjectRequest &item) {
  const std::string fileStem =
      safeFilePart(!item.name.empty() ? item.name : item.objectId);
  const std::string ext = extensionFor(item.format);
  if (item.annualId.empty())
    return baseOutput / (fileStem + "." + ext);
  const auto it = folderByAnnualId.find(item.annualId);
  const std::string annualFolder =
      it != folderByAnnualId.end() ? it->second : safeFilePart(item.annualId);
  return baseOutput / annualFolder / (fileStem + "." + ext);
}

} // namespace

namespace core::application::exporting {

core::ports::exporting::ExportResult exportObjectRequests(
    const core::domain::catalog::WorkspaceCatalog &state,
    const core::ports::exporting::ExportRequest &request,
    const std::shared_ptr<core::ports::archive::IArchive> &archive,
    const std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> &xlsxWriter,
    const std::shared_ptr<
        core::ports::analysis_image_renderer::IAnalysisImageRenderer>
        &imageRenderer) {
  core::ports::exporting::ExportResult result;
  result.actualFormat = request.format;
  result.resolvedOutputPath = request.outputPath;

  if (request.outputPath.empty()) {
    result.status = ExportStatus::InvalidInput;
    result.errorCode =
        std::string(core::constants::exportFlow::errors::kOutputPathEmpty);
    result.message =
        std::string(core::constants::exportFlow::messages::kOutputPathEmpty);
    return result;
  }

  try {
    std::filesystem::path baseOutput =
        std::filesystem::path(request.outputPath);
    if (baseOutput.has_extension())
      baseOutput = baseOutput.parent_path() / baseOutput.stem();
    if (!std::filesystem::exists(baseOutput))
      std::filesystem::create_directories(baseOutput);

    const auto folderByAnnualId = annualFolderNames(state);
    core::application::analysis::AnalysisService analysisService;
    std::vector<ExportObjectRequest> analysisItems;
    for (const auto &item : request.objectRequests) {
      if (item.type == ExportObjectType::Analysis && !item.objectId.empty())
        analysisItems.push_back(item);
    }

    const std::size_t total = analysisItems.size();
    for (size_t i = 0; i < total; ++i) {
      const auto &item = analysisItems[i];
      if (request.progressCallback) {
        request.progressCallback(
            0.25 + (0.55 *
                    (static_cast<double>(i) / std::max<std::size_t>(1, total))),
            "Exporting analysis " + std::to_string(i + 1) + "/" +
                std::to_string(total));
      }
      const auto computed = analysisService.runAnalysisById(
          state, item.objectId);
      if (!computed.found)
        continue;
      const std::filesystem::path outputFile =
          outputPathForItem(baseOutput, folderByAnnualId, item);
      std::filesystem::create_directories(outputFile.parent_path());

      bool ok = false;
      const auto tableRows =
          normalizedRowsForExport(computed, state);
      switch (item.format) {
      case AnalysisExportFormat::Csv:
        ok = writeCsvTable(outputFile, tableRows);
        break;
      case AnalysisExportFormat::Xlsx:
        ok = xlsxWriter &&
             xlsxWriter->writeTable(outputFile, tableRows, "Analysis");
        break;
      case AnalysisExportFormat::Jpg:
      case AnalysisExportFormat::Png:
        ok = imageRenderer &&
             imageRenderer->writeAnalysisImage(
                 outputFile,
                 computed.type.empty() ? item.objectId : computed.type,
                 computed);
        break;
      }
      if (!ok) {
        result.status = ExportStatus::WriteFailed;
        result.errorCode =
            std::string(core::constants::exportFlow::errors::kFileWriteFailed);
        result.message = std::string(
            core::constants::exportFlow::messages::kFileWriteFailed);
        return result;
      }
    }

    if (request.packageFormat == PackageFormat::Zip) {
      if (request.progressCallback)
        request.progressCallback(0.88, "Packaging export");
      const std::filesystem::path archivePath =
          baseOutput.string() +
          std::string(core::constants::exportFlow::packaging::kZipExtension);
      if (!archive ||
          !archive->create(baseOutput, archivePath, request.packageFormat)) {
        result.status = ExportStatus::ArchiveFailed;
        result.errorCode =
            std::string(core::constants::exportFlow::errors::kArchiveFailed);
        result.message =
            std::string(core::constants::exportFlow::messages::kArchiveFailed);
        return result;
      }
      result.resolvedOutputPath = archivePath.string();
    }

    if (request.progressCallback)
      request.progressCallback(0.95, "Finalizing export");
    result.status = ExportStatus::Ok;
    result.success = true;
    return result;
  } catch (...) {
    result.status = ExportStatus::InternalError;
    result.errorCode =
        std::string(core::constants::exportFlow::errors::kInternalError);
    result.message =
        std::string(core::constants::exportFlow::messages::kInternalError);
    return result;
  }
}

} // namespace core::application::exporting
