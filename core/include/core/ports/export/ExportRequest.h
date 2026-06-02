/**
 * @file core/include/core/ports/export/ExportRequest.h
 * @brief Declares export use-case request DTOs and related port types.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace core::ports::exporting {

enum class ExportFormat {
    Csv,
    Xlsx
};

enum class AnalysisExportFormat {
    Csv,
    Xlsx,
    Jpg,
    Png
};

enum class PackageFormat {
    None,
    Zip
};

enum class ExportObjectType {
    Analysis,
    Annual
};

enum class ExportStatus {
    InternalError,
    UnsupportedFormat,
    InvalidInput,
    WriteFailed,
    XlsxGenerationFailed,
    ArchiveFailed,
    Ok
};

/**
 * @brief Describes one persisted analysis or annual export target.
 */
struct ExportObjectRequest {
    ExportObjectType type = ExportObjectType::Analysis;
    std::string objectId;
    std::string annualId;
    AnalysisExportFormat format = AnalysisExportFormat::Csv;
    std::string name;
};

/**
 * @brief Describes one export run against a workspace snapshot.
 */
struct ExportRequest {
    ExportFormat format = ExportFormat::Xlsx;
    std::string outputPath;
    bool includeFormulas = true;
    std::string locale;
    PackageFormat packageFormat = PackageFormat::None;
    std::vector<ExportObjectRequest> objectRequests;
    std::function<void(double, const std::string&)> progressCallback;
};

} // namespace core::ports::exporting
