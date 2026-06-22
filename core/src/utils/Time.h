/**
 * @file core/src/utils/Time.h
 * @brief Declares private UTC timestamp helpers for core implementation code.
 */

#pragma once

#include <string>

namespace core::utils {

/**
 * @brief Returns the current UTC timestamp in ISO-8601 format.
 */
std::string currentTimestampUtc();

} // namespace core::utils
