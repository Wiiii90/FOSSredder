/**
 * @file ui/include/ui/shared/observability/Trace.h
 * @brief Declarations for the UI Trace component.
 */

#pragma once

#include <string>
#include <utility>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/shared/observability/ErrorCodes.h"

namespace ui::observability {

namespace context {

inline constexpr auto kArtifactCount = "artifactCount";
inline constexpr auto kColumn = "column";
inline constexpr auto kCount = "count";
inline constexpr auto kError = "error";
inline constexpr auto kException = "exception";
inline constexpr auto kFile = "file";
inline constexpr auto kFirstFile = "firstFile";
inline constexpr auto kFormat = "format";
inline constexpr auto kIncludeFormulas = "includeFormulas";
inline constexpr auto kLine = "line";
inline constexpr auto kLocale = "locale";
inline constexpr auto kPath = "path";
inline constexpr auto kQueuedCount = "queuedCount";
inline constexpr auto kRunRoot = "runRoot";
inline constexpr auto kStatus = "status";
inline constexpr auto kUrl = "url";

} // namespace context

inline void reportFlow(core::errors::ErrorSeverity severity, const char *code,
                       const char *origin, std::string message,
                       core::errors::ErrorContext context = {}) {
  core::errors::report(severity, code, origin, std::move(message),
                       std::move(context));
}

inline void reportFlow(core::errors::ErrorSeverity severity, const char *origin,
                       std::string message,
                       core::errors::ErrorContext context = {}) {
  reportFlow(severity, core::errors::codes::GenericError, origin,
             std::move(message), std::move(context));
}

} // namespace ui::observability
