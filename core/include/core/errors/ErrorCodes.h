/**
 * @file core/include/core/errors/ErrorCodes.h
 * @brief Defines shared structured error codes.
 */

#pragma once

namespace core::errors::codes {

/**
 * @brief Fallback code for generic structured errors.
 */
inline constexpr const char *GenericError = "GENERIC_ERROR";

/**
 * @brief Fallback code for exception-backed errors.
 */
inline constexpr const char *ExceptionError = "EXCEPTION_ERROR";

/**
 * @brief Code used when a standard exception is captured.
 */
inline constexpr const char *ExceptionStd = "EXCEPTION_STD";

/**
 * @brief Code used when a non-standard exception is captured.
 */
inline constexpr const char *ExceptionNonStd = "EXCEPTION_NON_STD";

/**
 * @brief Code used when opening the configuration database fails.
 */
inline constexpr const char *ConfigDbOpenFailed = "CFG_DB_OPEN_FAILED";

/**
 * @brief Code used for Qt debug messages.
 */
inline constexpr const char *QtDebug = "QT_DEBUG";

/**
 * @brief Code used for Qt informational messages.
 */
inline constexpr const char *QtInfo = "QT_INFO";

/**
 * @brief Code used for Qt warning messages.
 */
inline constexpr const char *QtWarning = "QT_WARNING";

/**
 * @brief Code used for Qt critical messages.
 */
inline constexpr const char *QtCritical = "QT_CRITICAL";

/**
 * @brief Code used for Qt fatal messages.
 */
inline constexpr const char *QtFatal = "QT_FATAL";

} // namespace core::errors::codes
