/**
 * @file core/include/core/errors/ErrorCodes.h
 * @brief Defines shared structured error codes.
 */

#pragma once

namespace core::errors::codes {

inline constexpr const char *GenericError = "GENERIC_ERROR";

inline constexpr const char *ExceptionError = "EXCEPTION_ERROR";
inline constexpr const char *ExceptionStd = "EXCEPTION_STD";
inline constexpr const char *ExceptionNonStd = "EXCEPTION_NON_STD";

inline constexpr const char *ConfigDbOpenFailed = "CFG_DB_OPEN_FAILED";

} // namespace core::errors::codes
