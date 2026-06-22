/**
 * @file core/include/core/errors/ErrorEventFactory.h
 * @brief Declares helpers for turning exceptions into structured error events.
 */

#pragma once

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorEvent.h"

#include <exception>
#include <string>
#include <typeinfo>
#include <utility>

namespace core::errors {

/**
 * @brief Convert an error severity to a stable lowercase label.
 * @param severity Severity value to convert.
 * @return String label for the severity.
 */
inline const char *severityToString(ErrorSeverity severity) noexcept {
  switch (severity) {
  case ErrorSeverity::Info:
    return "info";
  case ErrorSeverity::Warning:
    return "warning";
  case ErrorSeverity::Error:
    return "error";
  case ErrorSeverity::Critical:
    return "critical";
  }

  return "error";
}

/**
 * @brief Build a structured event from an exception pointer.
 * @param severity Severity to store on the event.
 * @param code Optional structured error code.
 * @param origin Optional origin string describing the reporting site.
 * @param exception Exception pointer to inspect.
 * @param context Additional event context.
 * @return Structured error event describing the exception.
 */
inline ErrorEvent makeExceptionEvent(ErrorSeverity severity, const char *code,
                                     const char *origin,
                                     std::exception_ptr exception,
                                     ErrorContext context = {}) {
  ErrorEvent event;
  event.severity = severity;
  event.origin = origin ? origin : std::string{};
  event.code = code ? code : codes::ExceptionError;
  event.context = std::move(context);

  if (!exception) {
    event.message = "unknown exception";
    event.exceptionType = "unknown";
    return event;
  }

  try {
    std::rethrow_exception(exception);
  } catch (const std::exception &ex) {
    event.message = ex.what();
    event.exceptionType = typeid(ex).name();
    if (!code)
      event.code = codes::ExceptionStd;
  } catch (...) {
    event.message = "non-std exception";
    event.exceptionType = "unknown";
    if (!code)
      event.code = codes::ExceptionNonStd;
  }

  return event;
}

} // namespace core::errors
