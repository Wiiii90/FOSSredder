/**
 * @file ui/include/ui/observability/Trace.h
 * @brief Declarations for the UI Trace component.
 */

#pragma once

#include <string>
#include <utility>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorEvent.h"
#include "core/errors/ErrorReporting.h"
#include "core/ports/diagnostics/IErrorReporter.h"
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

/**
 * @brief Reports whether UI trace emission is enabled.
 * @return True when trace events should be emitted.
 */
bool isTraceEnabled();
/**
 * @brief Emits a UI trace event.
 * @param layer UI layer name.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
void trace(const char* layer, const char* origin, std::string message,
           core::errors::ErrorContext context = {});

/**
 * @brief Emits an adapter trace event.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
inline void traceAdapter(const char* origin, std::string message,
                         core::errors::ErrorContext context = {}) {
  trace(layer::kAdapter, origin, std::move(message), std::move(context));
}

/**
 * @brief Emits a composition trace event.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
inline void traceComposition(const char* origin, std::string message,
                             core::errors::ErrorContext context = {}) {
  trace(layer::kComposition, origin, std::move(message), std::move(context));
}

/**
 * @brief Emits a view model trace event.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
inline void traceViewModel(const char* origin, std::string message,
                           core::errors::ErrorContext context = {}) {
  trace(layer::kViewModel, origin, std::move(message), std::move(context));
}

/**
 * @brief Emits a workflow trace event.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
inline void traceWorkflow(const char* origin, std::string message,
                          core::errors::ErrorContext context = {}) {
  trace(layer::kWorkflow, origin, std::move(message), std::move(context));
}

/**
 * @brief Emits a workspace trace event.
 * @param origin Trace origin.
 * @param message Trace message.
 * @param context Structured trace context.
 */
inline void traceWorkspace(const char* origin, std::string message,
                           core::errors::ErrorContext context = {}) {
  trace(layer::kWorkspace, origin, std::move(message), std::move(context));
}

/**
 * @brief Reports a UI flow error event.
 * @param severity Error severity.
 * @param code Error code.
 * @param origin Error origin.
 * @param message Error message.
 * @param context Structured error context.
 */
inline void reportFlow(core::ports::diagnostics::IErrorReporter* reporter,
                       core::errors::ErrorSeverity severity, const char* code,
                       const char* origin, std::string message,
                       core::errors::ErrorContext context = {}) {
  core::errors::report(reporter, severity, code, origin, std::move(message),
                       std::move(context));
}

/**
 * @brief Reports a generic UI flow error event.
 * @param severity Error severity.
 * @param origin Error origin.
 * @param message Error message.
 * @param context Structured error context.
 */
inline void reportFlow(core::ports::diagnostics::IErrorReporter* reporter,
                       core::errors::ErrorSeverity severity, const char* origin,
                       std::string message,
                       core::errors::ErrorContext context = {}) {
  reportFlow(reporter, severity, core::errors::codes::GenericError, origin,
             std::move(message), std::move(context));
}

} // namespace ui::observability
