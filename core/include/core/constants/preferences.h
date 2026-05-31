#pragma once

#include <string_view>

namespace core::constants::preferences {

inline constexpr std::string_view kOrganizationName = "FOSSRedder";
inline constexpr std::string_view kApplicationName = "FOSSRedder";

namespace keys {
inline constexpr std::string_view kLanguage = "general/language";
inline constexpr std::string_view kThemeMode = "general/themeMode";
inline constexpr std::string_view kImportDefaultPath = "import/defaultPath";
inline constexpr std::string_view kImportPoppler = "import/poppler";
inline constexpr std::string_view kImportOpenCv = "import/opencv";
inline constexpr std::string_view kImportTesseract = "import/tesseract";
inline constexpr std::string_view kImportParser = "import/parser";
inline constexpr std::string_view kImportMatcher = "import/matcher";
inline constexpr std::string_view kExportDefaultDirectory = "export/defaultDirectory";
inline constexpr std::string_view kExportArchiveFormat = "export/archiveFormat";
inline constexpr std::string_view kExportIncludeFormulas = "export/includeFormulas";
inline constexpr std::string_view kAnalysisDefaultDateMode = "analysis/defaultDateMode";
inline constexpr std::string_view kAnalysisDefaultYear = "analysis/defaultYear";
inline constexpr std::string_view kToolbarShowBooking = "workspace/toolbar/showBooking";
inline constexpr std::string_view kToolbarShowActors = "workspace/toolbar/showActors";
inline constexpr std::string_view kToolbarShowProperties = "workspace/toolbar/showProperties";
inline constexpr std::string_view kToolbarShowContracts = "workspace/toolbar/showContracts";
inline constexpr std::string_view kToolbarShowImport = "workspace/toolbar/showImport";
inline constexpr std::string_view kToolbarShowExport = "workspace/toolbar/showExport";
inline constexpr std::string_view kToolbarShowAnalysis = "workspace/toolbar/showAnalysis";
inline constexpr std::string_view kToolbarShowAnnual = "workspace/toolbar/showAnnual";
inline constexpr std::string_view kToolbarShowSettings = "workspace/toolbar/showSettings";
} // namespace keys

} // namespace core::constants::preferences
