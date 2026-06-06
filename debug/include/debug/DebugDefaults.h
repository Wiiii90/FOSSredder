/**
 * @file debug/include/debug/DebugDefaults.h
 * @brief Declares shared debugger output and logging defaults.
 */

#pragma once

#include <cstddef>
#include <string_view>

namespace debug::defaults {

inline constexpr std::string_view kOutputDirectoryName = "debug_output";
inline constexpr std::string_view kDiagnosticsLogPath = "diagnostics/diagnostics.log";
inline constexpr std::string_view kDiagnosticsProcessName = "diagnostics";
inline constexpr std::string_view kImportProcessName = "import";
inline constexpr std::string_view kPopplerLogPrefix = "poppler/log";
inline constexpr std::string_view kPopplerMetadataPrefix = "poppler/meta/";
inline constexpr std::string_view kSpdlogFilePath = "logs/fossredder.log";
inline constexpr std::size_t kSpdlogFileSizeBytes = 1024 * 1024 * 5;
inline constexpr std::size_t kSpdlogFileCount = 3;

}
