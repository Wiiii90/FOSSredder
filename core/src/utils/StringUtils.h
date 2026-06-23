/**
 * @file core/src/utils/StringUtils.h
 * @brief Declares private string helpers shared across `core` implementation
 * files.
 */

#pragma once

#include <string>
#include <vector>

namespace core::utils {

/**
 * @brief Removes leading and trailing ASCII whitespace from a string.
 */
std::string trim(std::string s);

/**
 * @brief Converts ASCII letters in a string to lowercase.
 */
std::string lowerAscii(std::string s);

/**
 * @brief Collapses consecutive whitespace to single spaces and trims the
 * result.
 */
std::string collapseWhitespace(std::string s);

/**
 * @brief Splits a string on ASCII whitespace.
 */
std::vector<std::string> splitWhitespace(const std::string& s);

} // namespace core::utils
