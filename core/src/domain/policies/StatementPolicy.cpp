/**
 * @file core/src/domain/policies/StatementPolicy.cpp
 * @brief Shared statement membership helpers.
 */

#include "core/domain/policies/StatementPolicy.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <utility>

namespace core::domain::policies::statement {

std::string trimCopy(std::string value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

std::string normalizeId(std::string value) {
    return trimCopy(std::move(value));
}

void normalizeIds(std::vector<std::string>& values) {
    std::vector<std::string> unique;
    unique.reserve(values.size());
    for (auto& value : values) {
        value = normalizeId(std::move(value));
        if (value.empty()) {
            continue;
        }
        if (std::find(unique.begin(), unique.end(), value) != unique.end()) {
            continue;
        }
        unique.push_back(value);
    }
    values = std::move(unique);
}

bool containsTransactionId(const std::vector<std::string>& transactionIds, const std::string& value) {
    const auto normalized = normalizeId(value);
    return std::find(transactionIds.begin(), transactionIds.end(), normalized) != transactionIds.end();
}

std::size_t indexOfTransactionId(const std::vector<std::string>& transactionIds, const std::string& value) {
    const auto normalized = normalizeId(value);
    const auto it = std::find(transactionIds.begin(), transactionIds.end(), normalized);
    if (it == transactionIds.end()) {
        return transactionIds.size();
    }
    return static_cast<std::size_t>(std::distance(transactionIds.begin(), it));
}

bool hasUniqueTransactionIds(const std::vector<std::string>& transactionIds) {
    std::vector<std::string> unique;
    unique.reserve(transactionIds.size());
    for (const auto& transactionId : transactionIds) {
        if (transactionId.empty()) {
            continue;
        }
        if (std::find(unique.begin(), unique.end(), transactionId) != unique.end()) {
            return false;
        }
        unique.push_back(transactionId);
    }
    return true;
}

} // namespace core::domain::policies::statement
