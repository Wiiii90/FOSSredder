#pragma once

#include <string_view>

namespace core::constants::exportFlow {

inline constexpr std::string_view kErrorUnsupportedFormat = "EXPORT_UNSUPPORTED_FORMAT";
inline constexpr std::string_view kMessageUnsupportedFormat = "Unsupported export format";

namespace errors {
inline constexpr std::string_view kOutputPathEmpty = "EXPORT_OUTPUT_PATH_EMPTY";
inline constexpr std::string_view kStateMissing = "EXPORT_STATE_MISSING";
inline constexpr std::string_view kFileOpenFailed = "EXPORT_FILE_OPEN_FAILED";
inline constexpr std::string_view kFileWriteFailed = "EXPORT_FILE_WRITE_FAILED";
inline constexpr std::string_view kArchiveFailed = "EXPORT_ARCHIVE_FAILED";
inline constexpr std::string_view kXlsxGenerationFailed = "EXPORT_XLSX_GENERATION_FAILED";
inline constexpr std::string_view kInternalError = "EXPORT_INTERNAL_ERROR";
inline constexpr std::string_view kRunnerUnavailable = "EXPORT_RUNNER_UNAVAILABLE";
} // namespace errors

namespace messages {
inline constexpr std::string_view kOutputPathEmpty = "Output path is empty";
inline constexpr std::string_view kStateMissing = "State snapshot is missing";
inline constexpr std::string_view kFileOpenFailed = "Failed to open export output file";
inline constexpr std::string_view kFileWriteFailed = "Failed while writing export output file";
inline constexpr std::string_view kArchiveFailed = "Failed to package export output";
inline constexpr std::string_view kXlsxGenerationFailed = "XLSX generation failed";
inline constexpr std::string_view kInternalError = "Unexpected error during export";
inline constexpr std::string_view kRunnerUnavailable = "Export runner is not configured";
} // namespace messages

namespace packaging {
inline constexpr std::string_view kZipExtension = ".zip";
} // namespace packaging

namespace labels {
inline constexpr std::string_view kPropertyHeader = "Gebäude";
inline constexpr std::string_view kTotal = "Summe";
inline constexpr std::string_view kUnassigned = "(Unassigned)";
} // namespace labels

namespace selectionPayload {
inline constexpr std::string_view kPackageFormatIndex = "packageFormatIndex";
inline constexpr std::string_view kItems = "items";
inline constexpr std::string_view kObjectType = "objectType";
inline constexpr std::string_view kObjectId = "objectId";
inline constexpr std::string_view kObjectName = "objectName";
inline constexpr std::string_view kAnnualId = "annualId";
inline constexpr std::string_view kExportType = "exportType";
inline constexpr std::string_view kAnnualObjectType = "annual";
inline constexpr std::string_view kAnalysisObjectType = "analysis";
} // namespace selectionPayload

} // namespace core::constants::exportFlow
