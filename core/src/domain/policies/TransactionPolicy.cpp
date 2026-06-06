/**
 * @file core/src/domain/policies/TransactionPolicy.cpp
 * @brief Shared transaction validation and normalization helpers.
 */

#include "core/domain/policies/TransactionPolicy.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain::policies::transaction {

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

bool hasValidBookingDate(const std::string& value) {
    return core::domain::BookingDate::isValid(value);
}

bool hasValidAmount(double value) {
    return core::domain::MoneyAmount::isValid(value);
}

bool hasStatementAssignment(const std::string& statementId) {
    return !trimCopy(statementId).empty();
}

std::string normalizeText(std::string value) {
    return trimCopy(std::move(value));
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

bool canFinalizeFromDraft(const std::string& bookingDate, double amount, const std::string& statementId) {
    return hasValidBookingDate(bookingDate) && hasValidAmount(amount) && hasStatementAssignment(statementId);
}

bool statusCanAdvance(int current, int next) {
    return next >= current;
}

bool hasPropertyIds(const std::vector<std::string>& propertyIds) {
    return !propertyIds.empty();
}

} // namespace core::domain::policies::transaction
