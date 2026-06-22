/**
 * @file core/include/core/errors/ErrorReporting.h
 * @brief Provides shared helpers for reporting structured errors and exceptions.
 */

#pragma once

#include <exception>

#include "core/errors/ErrorEventFactory.h"
#include "core/ports/diagnostics/IErrorReporter.h"

namespace core::errors {

inline void report(core::ports::diagnostics::IErrorReporter *reporter,
                   const ErrorEvent &event) {
  if (reporter)
    reporter->report(event);
}

inline void reportException(core::ports::diagnostics::IErrorReporter *reporter,
                            ErrorSeverity severity, const char *origin,
                            std::exception_ptr exception) {
  if (reporter)
    reporter->reportException(severity, origin, exception);
}

} // namespace core::errors
