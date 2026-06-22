/**
 * @file core/src/export/CsvExporter.cpp
 * @brief Implements CSV export for the property/contract-type matrix.
 */

#include "core/application/export/CsvExporter.h"

#include "PropertyContractMatrix.h"
#include "core/constants/export.h"
#include "core/constants/localization.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string escapeCsv(const std::string &value, char separator) {
  bool requiresQuoting = false;
  for (const char c : value) {
    if (c == '"' || c == '\n' || c == '\r' || c == separator) {
      requiresQuoting = true;
      break;
    }
  }
  if (!requiresQuoting) {
    return value;
  }

  std::string escaped;
  escaped.push_back('"');
  for (const char c : value) {
    if (c == '"') {
      escaped.push_back('"');
    }
    escaped.push_back(c);
  }
  escaped.push_back('"');
  return escaped;
}

} // namespace

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
  result.actualFormat = export_ports::ExportFormat::Csv;
  result.resolvedOutputPath = request.outputPath;
  result.status = export_ports::ExportStatus::Canceled;
  result.message = "Canceled";
  return result;
}

} // namespace

export_ports::ExportResult
CsvExporter::exportData(
    const core::domain::catalog::WorkspaceCatalog &state,
    const export_ports::ExportRequest &request) const {
  export_ports::ExportResult result;
  result.actualFormat = export_ports::ExportFormat::Csv;
  result.resolvedOutputPath = request.outputPath;

  if (shouldStop(request)) {
    return canceledResult(request);
  }

  if (request.outputPath.empty()) {
    result.status = export_ports::ExportStatus::InvalidInput;
    result.errorCode =
        std::string(core::constants::exportFlow::errors::kOutputPathEmpty);
    result.message =
        std::string(core::constants::exportFlow::messages::kOutputPathEmpty);
    return result;
  }
  const auto matrix = internal::buildPropertyContractMatrix(state);

  char decimalSeparator = '.';
  if (request.locale.starts_with(
          core::constants::localization::languages::kGerman)) {
    decimalSeparator = ',';
  }
  constexpr char separator = ';';

  std::ofstream output(request.outputPath, std::ios::binary);
  if (!output) {
    result.status = export_ports::ExportStatus::WriteFailed;
    result.errorCode =
        std::string(core::constants::exportFlow::errors::kFileOpenFailed);
    result.message =
        std::string(core::constants::exportFlow::messages::kFileOpenFailed);
    return result;
  }

  const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  output.write(reinterpret_cast<const char *>(bom), sizeof(bom));

  const auto formatAmount = [&](double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    std::string text = stream.str();
    if (decimalSeparator != '.') {
      for (auto &c : text) {
        if (c == '.') {
          c = decimalSeparator;
        }
      }
    }
    return text;
  };

  output << std::string(core::constants::exportFlow::labels::kPropertyHeader);
  for (const auto &contractType : matrix.contractTypes) {
    output << separator << escapeCsv(contractType, separator);
  }
  output << separator
         << std::string(core::constants::exportFlow::labels::kTotal) << "\n";

  for (const auto &propertyName : matrix.propertyNames) {
    if (shouldStop(request)) {
      return canceledResult(request);
    }
    output << escapeCsv(propertyName, separator);
    double rowSum = 0.0;
    const auto propertyIt = matrix.amountsByProperty.find(propertyName);
    for (const auto &contractType : matrix.contractTypes) {
      double value = 0.0;
      if (propertyIt != matrix.amountsByProperty.end()) {
        const auto contractIt = propertyIt->second.find(contractType);
        if (contractIt != propertyIt->second.end()) {
          value = contractIt->second;
        }
      }
      output << separator << formatAmount(value);
      rowSum += value;
    }
    output << separator << formatAmount(rowSum) << "\n";
  }

  output.close();
  if (!output) {
    result.status = export_ports::ExportStatus::WriteFailed;
    result.errorCode =
        std::string(core::constants::exportFlow::errors::kFileWriteFailed);
    result.message =
        std::string(core::constants::exportFlow::messages::kFileWriteFailed);
    return result;
  }

  result.status = export_ports::ExportStatus::Ok;
  result.success = true;
  return result;
}

} // namespace core::application::exporting
