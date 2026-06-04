/**
 * @file ui/include/ui/observability/Trace.h
 * @brief Declarations for the UI Trace component.
 */

#pragma once

#include <string>
#include <utility>

#include "core/errors/ErrorEvent.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/observability/ErrorCodes.h"

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
inline constexpr auto kId = "id";
inline constexpr auto kLine = "line";
inline constexpr auto kLocale = "locale";
inline constexpr auto kName = "name";
inline constexpr auto kOperation = "operation";
inline constexpr auto kPath = "path";
inline constexpr auto kQueuedCount = "queuedCount";
inline constexpr auto kRunRoot = "runRoot";
inline constexpr auto kSelectionId = "selectionId";
inline constexpr auto kStatus = "status";
inline constexpr auto kUrl = "url";

} // namespace context

namespace layer {

inline constexpr auto kAdapter = "adapter";
inline constexpr auto kComposition = "composition";
inline constexpr auto kViewModel = "viewmodel";
inline constexpr auto kWorkflow = "workflow";
inline constexpr auto kWorkspace = "workspace";

} // namespace layer

bool isTraceEnabled();
void trace(const char *layer, const char *origin, std::string message,
           core::errors::ErrorContext context = {});

inline void traceAdapter(const char *origin, std::string message,
                         core::errors::ErrorContext context = {}) {
  trace(layer::kAdapter, origin, std::move(message), std::move(context));
}

inline void traceComposition(const char *origin, std::string message,
                             core::errors::ErrorContext context = {}) {
  trace(layer::kComposition, origin, std::move(message), std::move(context));
}

inline void traceViewModel(const char *origin, std::string message,
                           core::errors::ErrorContext context = {}) {
  trace(layer::kViewModel, origin, std::move(message), std::move(context));
}

inline void traceWorkflow(const char *origin, std::string message,
                          core::errors::ErrorContext context = {}) {
  trace(layer::kWorkflow, origin, std::move(message), std::move(context));
}

inline void traceWorkspace(const char *origin, std::string message,
                           core::errors::ErrorContext context = {}) {
  trace(layer::kWorkspace, origin, std::move(message), std::move(context));
}

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
