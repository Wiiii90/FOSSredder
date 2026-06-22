/**
 * @file persistence/src/RepositoryDiagnostics.h
 * @brief Declares private helpers for repository-level diagnostic reporting.
 */

#pragma once

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporting.h"

#include <memory>
#include <string>
#include <utility>

namespace persistence::diagnostics {

inline void report(
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& reporter,
    core::errors::ErrorSeverity severity, const char* origin,
    std::string message, const char* code = core::errors::codes::GenericError,
    core::errors::ErrorContext context = {}) {
  if (!reporter)
    return;

  core::errors::ErrorEvent event;
  event.severity = severity;
  event.origin = origin ? origin : std::string{};
  event.message = std::move(message);
  event.code = code ? code : core::errors::codes::GenericError;
  event.context = std::move(context);
  core::errors::report(reporter.get(), event);
}

} // namespace persistence::diagnostics
