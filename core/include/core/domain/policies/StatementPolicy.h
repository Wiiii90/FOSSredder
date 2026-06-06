/**
 * @file core/include/core/domain/policies/StatementPolicy.h
 * @brief Shared statement membership and normalization helpers.
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <iterator>
#include <string>
#include <vector>
#include <utility>

namespace core::domain::policies::statement {

/**
 * @brief Trims leading and trailing whitespace from text.
 * @param value Raw text.
 * @return Trimmed text or an empty string.
 */
std::string trimCopy(std::string value);

/**
 * @brief Normalizes a transaction identifier.
 * @param value Raw transaction id.
 * @return Normalized transaction id.
 */
std::string normalizeId(std::string value);

/**
 * @brief Normalizes and deduplicates transaction identifiers.
 * @param values Transaction id list to mutate.
 */
void normalizeIds(std::vector<std::string>& values);

/**
 * @brief Checks whether a statement already contains a transaction id.
 * @param transactionIds Transaction id list to inspect.
 * @param value Transaction id to search for.
 * @return `true` when the id exists.
 */
bool containsTransactionId(const std::vector<std::string>& transactionIds, const std::string& value);

/**
 * @brief Returns the index of a transaction id.
 * @param transactionIds Transaction id list to inspect.
 * @param value Transaction id to search for.
 * @return Matching index or the list size when not found.
 */
std::size_t indexOfTransactionId(const std::vector<std::string>& transactionIds, const std::string& value);

/**
 * @brief Checks whether a transaction id list contains duplicates.
 * @param transactionIds Transaction id list to inspect.
 * @return `true` when all non-empty ids are unique.
 */
bool hasUniqueTransactionIds(const std::vector<std::string>& transactionIds);

} // namespace core::domain::policies::statement
