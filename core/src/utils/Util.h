/**
 * @file core/src/utils/Util.h
 * @brief Declares private string helpers shared across `core` implementation files.
 */

#pragma once

#include <string>
#include <utility>
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
 * @brief Collapses consecutive whitespace to single spaces and trims the result.
 */
std::string collapseWhitespace(std::string s);

/**
 * @brief Splits a string on ASCII whitespace.
 */
std::vector<std::string> splitWhitespace(const std::string& s);

/**
 * @brief Returns the rightmost x-coordinate from a set of word spans.
 */
int rightEdgeFromWordSpans(const std::vector<std::pair<int, int>>& wordSpans);

}
