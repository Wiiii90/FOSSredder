/**
 * @file core/src/export/XlsxExporter.cpp
 * @brief Implements XLSX export for the property/contract-type matrix.
 */

#include "core/application/export/XlsxExporter.h"

#include "PropertyContractMatrix.h"
#include "ExportConstants.h"
#include "core/ports/infra/xlsx-writer/IXlsxWriter.h"

#include <vector>

namespace core::application::exporting {

namespace export_ports = core::ports::exporting;

namespace {

bool shouldStop(const export_ports::ExportRequest &request) {
  if (request.waitIfPaused) {
    request.waitIfPaused();
  }
  return request.shouldCancel && request.shouldCancel();
}

export_ports::ExportResult canceledResult(
    const export_ports::ExportRequest &request) {
  export_ports::ExportResult result;
  result.actualFormat = export_ports::ExportFormat::Xlsx;
  result.resolvedOutputPath = request.outputPath;
  result.status = export_ports::ExportStatus::Canceled;
  result.message = "Canceled";
  return result;
}

std::string columnRef(int column) {
  std::string ref;
  while (column > 0) {
    const int remainder = (column - 1) % 26;
    ref.insert(ref.begin(), static_cast<char>('A' + remainder));
    column = (column - 1) / 26;
  }
  return ref;
}

std::vector<std::vector<std::string>>
buildRows(const internal::PropertyContractMatrix &matrix,
          bool includeFormulas) {
  std::vector<std::vector<std::string>> rows;
  std::vector<std::string> header;
  header.push_back(
      std::string(core::application::exporting::constants::labels::kPropertyHeader));
  header.insert(header.end(), matrix.propertyNames.begin(),
                matrix.propertyNames.end());
  header.push_back(std::string(core::application::exporting::constants::labels::kTotal));
  rows.push_back(std::move(header));

  std::vector<double> columnSums(matrix.propertyNames.size(), 0.0);
  int rowIndex = 2;
  for (const auto &contractType : matrix.contractTypes) {
    std::vector<std::string> row;
    row.reserve(matrix.propertyNames.size() + 2);
    row.push_back(contractType);

    double rowSum = 0.0;
    for (size_t column = 0; column < matrix.propertyNames.size(); ++column) {
      double value = 0.0;
      const auto propertyIt =
          matrix.amountsByProperty.find(matrix.propertyNames[column]);
      if (propertyIt != matrix.amountsByProperty.end()) {
        const auto contractIt = propertyIt->second.find(contractType);
        if (contractIt != propertyIt->second.end()) {
          value = contractIt->second;
        }
      }
      row.push_back(std::to_string(value));
      columnSums[column] += value;
      rowSum += value;
    }
    if (includeFormulas && !matrix.propertyNames.empty()) {
      row.push_back(
          "=SUM(" + columnRef(2) + std::to_string(rowIndex) + ":" +
          columnRef(static_cast<int>(1 + matrix.propertyNames.size())) +
          std::to_string(rowIndex) + ")");
    } else {
      row.push_back(std::to_string(rowSum));
    }
    rows.push_back(std::move(row));
    ++rowIndex;
  }

  if (!matrix.propertyNames.empty()) {
    std::vector<std::string> totalRow;
    totalRow.reserve(matrix.propertyNames.size() + 2);
    totalRow.push_back(
        std::string(core::application::exporting::constants::labels::kTotal));

    double grandTotal = 0.0;
    for (double value : columnSums) {
      totalRow.push_back(std::to_string(value));
      grandTotal += value;
    }
    if (includeFormulas && !matrix.contractTypes.empty()) {
      totalRow.push_back(
          "=SUM(" +
          columnRef(static_cast<int>(2 + matrix.propertyNames.size())) +
          "2:" + columnRef(static_cast<int>(2 + matrix.propertyNames.size())) +
          std::to_string(rowIndex - 1) + ")");
    } else {
      totalRow.push_back(std::to_string(grandTotal));
    }
    rows.push_back(std::move(totalRow));
  }

  return rows;
}

} // namespace

XlsxExporter::XlsxExporter(
    std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> writer)
    : writer_(std::move(writer)) {}

export_ports::ExportResult
XlsxExporter::exportData(
    const core::domain::catalog::WorkspaceCatalog &state,
    const export_ports::ExportRequest &request) const {
  export_ports::ExportResult result;
  result.actualFormat = export_ports::ExportFormat::Xlsx;
  result.resolvedOutputPath = request.outputPath;

  try {
    if (shouldStop(request)) {
      return canceledResult(request);
    }
    if (request.outputPath.empty()) {
      result.status = export_ports::ExportStatus::InvalidInput;
      result.errorCode =
          std::string(core::application::exporting::constants::errors::kOutputPathEmpty);
      result.message =
          std::string(core::application::exporting::constants::messages::kOutputPathEmpty);
      return result;
    }
    if (!writer_) {
      result.status = export_ports::ExportStatus::XlsxGenerationFailed;
      result.errorCode = std::string(
          core::application::exporting::constants::errors::kXlsxGenerationFailed);
      result.message = std::string(
          core::application::exporting::constants::messages::kXlsxGenerationFailed);
      return result;
    }

    const auto matrix = internal::buildPropertyContractMatrix(state);
    if (shouldStop(request)) {
      return canceledResult(request);
    }
    const auto rows = buildRows(matrix, request.includeFormulas);
    if (shouldStop(request)) {
      return canceledResult(request);
    }

    if (!writer_->writeTable(request.outputPath, rows, "Export")) {
      result.status = export_ports::ExportStatus::XlsxGenerationFailed;
      result.errorCode = std::string(
          core::application::exporting::constants::errors::kXlsxGenerationFailed);
      result.message = std::string(
          core::application::exporting::constants::messages::kXlsxGenerationFailed);
      return result;
    }

    result.status = export_ports::ExportStatus::Ok;
    result.success = true;
  } catch (...) {
    result.status = export_ports::ExportStatus::XlsxGenerationFailed;
    result.errorCode =
        std::string(core::application::exporting::constants::errors::kXlsxGenerationFailed);
    result.message = std::string(
        core::application::exporting::constants::messages::kXlsxGenerationFailed);
  }

  return result;
}

} // namespace core::application::exporting
