/**
 * @file core/include/core/errors/IErrorReporter.h
 * @brief Declares the structured error reporting interface.
 */

#pragma once

#include <exception>

#include "core/errors/ErrorEvent.h"
#include "core/errors/ErrorEventFactory.h"

namespace core::errors {

class IErrorReporter {
public:
  virtual ~IErrorReporter() = default;

  virtual void report(const ErrorEvent &event) = 0;

  virtual void reportException(ErrorSeverity severity, const char *origin,
                               std::exception_ptr exception) {
    report(makeExceptionEvent(severity, nullptr, origin, exception));
  }
};

} // namespace core::errors
