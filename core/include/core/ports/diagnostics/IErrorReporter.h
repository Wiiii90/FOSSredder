/**
 * @file core/include/core/ports/diagnostics/IErrorReporter.h
 * @brief Declares the structured diagnostics reporting port.
 */

#pragma once

#include <exception>

#include "core/errors/ErrorEvent.h"
#include "core/errors/ErrorEventFactory.h"

namespace core::ports::diagnostics {

class IErrorReporter {
public:
  virtual ~IErrorReporter() = default;

  virtual void report(const core::errors::ErrorEvent &event) = 0;

  virtual void reportException(core::errors::ErrorSeverity severity,
                               const char *origin,
                               std::exception_ptr exception) {
    report(core::errors::makeExceptionEvent(severity, nullptr, origin,
                                            exception));
  }
};

} // namespace core::ports::diagnostics
