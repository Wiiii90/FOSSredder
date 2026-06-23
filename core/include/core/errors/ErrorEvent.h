/**
 * @file core/include/core/errors/ErrorEvent.h
 * @brief Declares structured error event payloads.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace core::errors {

/**
 * @brief Severity level for structured error events.
 */
enum class ErrorSeverity {
  Info,
  Warning,
  Error,
  Critical
};

/**
 * @brief Key/value context attached to an error event.
 */
using ErrorContext = std::vector<std::pair<std::string, std::string>>;

/**
 * @brief Structured error payload sent through diagnostics ports.
 */
struct ErrorEvent {
  ErrorSeverity severity = ErrorSeverity::Error;
  std::string origin;
  std::string message;
  std::string exceptionType;
  std::string code = "GENERIC_ERROR";
  ErrorContext context;
};

} // namespace core::errors
