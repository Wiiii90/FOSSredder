/**
 * @file core/include/core/errors/ErrorReporterRegistry.h
 * @brief Declares the process-wide error reporter registry.
 */

#pragma once

#include <exception>
#include <memory>
#include <string>

#include "core/ports/diagnostics/IErrorReporter.h"

namespace core::errors {

void setGlobalErrorReporter(
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> reporter);
std::shared_ptr<core::ports::diagnostics::IErrorReporter>
globalErrorReporter();

void report(const ErrorEvent &event);
void report(ErrorSeverity severity, const char *code, const char *origin,
            std::string message, ErrorContext context = {});
void reportException(ErrorSeverity severity, const char *origin,
                     std::exception_ptr exception);
void reportException(ErrorSeverity severity, const char *code,
                     const char *origin, std::exception_ptr exception,
                     ErrorContext context = {});

} // namespace core::errors
