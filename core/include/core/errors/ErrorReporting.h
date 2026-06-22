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

/**
 * @brief Report an already constructed error event when a reporter exists.
 * @param reporter Optional reporter port.
 * @param event Event to report.
 */
inline void report(core::ports::diagnostics::IErrorReporter *reporter,
                   const ErrorEvent &event) {
  if (reporter)
    reporter->report(event);
}

/**
 * @brief Construct and report a structured error event.
 * @param reporter Optional reporter port.
 * @param severity Severity to store on the event.
 * @param code Optional structured error code.
 * @param origin Optional origin string describing the reporting site.
 * @param message Human-readable error message.
 * @param context Additional event context.
 */
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

/**
 * @brief Report an exception through the reporter's default exception handling.
 * @param reporter Optional reporter port.
 * @param severity Severity to apply to the exception event.
 * @param origin Optional origin string describing the reporting site.
 * @param exception Exception pointer to report.
 */
inline void reportException(core::ports::diagnostics::IErrorReporter *reporter,
                            ErrorSeverity severity, const char *origin,
                            std::exception_ptr exception) {
  if (reporter)
    reporter->reportException(severity, origin, exception);
}

/**
 * @brief Build and report a structured exception event.
 * @param reporter Optional reporter port.
 * @param severity Severity to apply to the exception event.
 * @param code Optional structured error code.
 * @param origin Optional origin string describing the reporting site.
 * @param exception Exception pointer to inspect.
 * @param context Additional event context.
 */
inline void reportException(core::ports::diagnostics::IErrorReporter *reporter,
                            ErrorSeverity severity, const char *code,
                            const char *origin, std::exception_ptr exception,
                            ErrorContext context = {}) {
  if (reporter)
    reporter->report(makeExceptionEvent(severity, code, origin, exception,
                                        std::move(context)));
}

} // namespace core::errors
