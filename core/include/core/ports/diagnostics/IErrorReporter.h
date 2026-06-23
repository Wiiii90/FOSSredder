/**
 * @file core/include/core/ports/diagnostics/IErrorReporter.h
 * @brief Declares the structured diagnostics reporting port.
 */

#pragma once

#include <exception>

#include "core/errors/ErrorEvent.h"
#include "core/errors/ErrorEventFactory.h"

namespace core::ports::diagnostics {

/**
 * @brief Port for reporting structured diagnostic error events.
 */
class IErrorReporter {
public:
  /**
   * @brief Destroy the error reporter port.
   */
  virtual ~IErrorReporter() = default;

  /**
   * @brief Report a structured error event.
   * @param event Event to report.
   */
  virtual void report(const core::errors::ErrorEvent &event) = 0;

  /**
   * @brief Report an exception as a structured error event.
   * @param severity Severity to apply to the exception event.
   * @param origin Optional origin string describing the reporting site.
   * @param exception Exception pointer to inspect.
   */
  virtual void reportException(core::errors::ErrorSeverity severity,
                               const char *origin,
                               std::exception_ptr exception) {
    report(core::errors::makeExceptionEvent(severity, nullptr, origin,
                                            exception));
  }
};

} // namespace core::ports::diagnostics
