/**
 * @file core/include/core/errors/ErrorReporting.h
 * @brief Provides shared helpers for reporting structured errors and exceptions.
 */

#pragma once

#include <exception>
#include <string>
#include <utility>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorEventFactory.h"
#include "core/ports/diagnostics/IErrorReporter.h"

namespace core::errors {

inline void report(core::ports::diagnostics::IErrorReporter *reporter,
                   const ErrorEvent &event) {
  if (reporter)
    reporter->report(event);
}

inline void report(core::ports::diagnostics::IErrorReporter *reporter,
                   ErrorSeverity severity, const char *code,
                   const char *origin, std::string message,
                   ErrorContext context = {}) {
  ErrorEvent event;
  event.severity = severity;
  event.code = code ? code : codes::GenericError;
  event.origin = origin ? origin : std::string{};
  event.message = std::move(message);
  event.context = std::move(context);
  report(reporter, event);
}

inline void reportException(core::ports::diagnostics::IErrorReporter *reporter,
                            ErrorSeverity severity, const char *origin,
                            std::exception_ptr exception) {
  if (reporter)
    reporter->reportException(severity, origin, exception);
}

inline void reportException(core::ports::diagnostics::IErrorReporter *reporter,
                            ErrorSeverity severity, const char *code,
                            const char *origin, std::exception_ptr exception,
                            ErrorContext context = {}) {
  if (reporter)
    reporter->report(makeExceptionEvent(severity, code, origin, exception,
                                        std::move(context)));
}

} // namespace core::errors
