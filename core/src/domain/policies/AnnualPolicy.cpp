/**
 * @file core/src/domain/policies/AnnualPolicy.cpp
 * @brief Shared annual validation helpers.
 */

#include "core/domain/policies/AnnualPolicy.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <regex>
#include <utility>

namespace core::domain::policies::annual {

AnnualValidationIssue::AnnualValidationIssue() = default;
AnnualValidationIssue::AnnualValidationIssue(std::string transactionId, std::string reason)
    : transactionId_(std::move(transactionId)), reason_(std::move(reason)) {}
const std::string& AnnualValidationIssue::transactionId() const noexcept { return transactionId_; }
const std::string& AnnualValidationIssue::reason() const noexcept { return reason_; }

const std::vector<AnnualValidationIssue>& AnnualValidationResult::issues() const noexcept { return issues_; }
void AnnualValidationIssue::setTransactionId(std::string value) { transactionId_ = std::move(value); }
void AnnualValidationIssue::setReason(std::string value) { reason_ = std::move(value); }
void AnnualValidationResult::setIssues(std::vector<AnnualValidationIssue> value) { issues_ = std::move(value); }
void AnnualValidationResult::addIssue(std::string transactionId, std::string reason) { issues_.emplace_back(std::move(transactionId), std::move(reason)); }
bool AnnualValidationResult::isValid() const noexcept { return issues_.empty(); }
bool AnnualValidationResult::hasIssues() const noexcept { return !issues_.empty(); }

bool isValidYear(int year) { return core::domain::Year::isValid(year); }
bool isValidAnnual(int year, const std::vector<std::string>& analysisIds) { return isValidYear(year) && (!analysisIds.empty()); }

std::string normalizeId(std::string value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c) != 0; }).base();
    if (begin >= end) return {};
    return std::string(begin, end);
}

bool containsAnalysisId(const std::vector<std::string>& analysisIds, const std::string& analysisId) {
    const auto normalized = normalizeId(analysisId);
    return std::find(analysisIds.begin(), analysisIds.end(), normalized) != analysisIds.end();
}

bool hasUniqueAnalysisIds(const std::vector<std::string>& analysisIds) {
    std::vector<std::string> unique;
    unique.reserve(analysisIds.size());
    for (const auto& rawAnalysisId : analysisIds) {
        const auto analysisId = normalizeId(rawAnalysisId);
        if (analysisId.empty()) continue;
        if (std::find(unique.begin(), unique.end(), analysisId) != unique.end()) return false;
        unique.push_back(analysisId);
    }
    return true;
}

bool containsAllAnalysisIds(const std::vector<std::string>& analysisIds, const std::vector<std::string>& requiredIds) {
    return std::all_of(requiredIds.begin(), requiredIds.end(), [&](const std::string& id) { return containsAnalysisId(analysisIds, id); });
}

void normalizeIds(std::vector<std::string>& values) {
    std::vector<std::string> normalized;
    normalized.reserve(values.size());
    for (auto& rawValue : values) {
        auto value = normalizeId(std::move(rawValue));
        if (value.empty()) {
            continue;
        }
        if (std::find(normalized.begin(), normalized.end(), value) != normalized.end()) {
            continue;
        }
        normalized.push_back(std::move(value));
    }
    values = std::move(normalized);
}

std::optional<int> extractBookingYear(const std::string& bookingDate) {
    const auto trimmed = normalizeId(bookingDate);
    if (trimmed.empty()) return std::nullopt;
    static const std::regex isoYearPattern(R"(^(\d{4})-\d{2}-\d{2}$)");
    static const std::regex dottedYearPattern(R"(^\d{1,2}\.\d{1,2}\.(\d{4})$)");
    static const std::regex slashYearPattern(R"(^\d{1,2}/\d{1,2}/(\d{4})$)");
    static const std::regex standaloneYearPattern(R"((?:^|[^0-9])(\d{4})(?:[^0-9]|$))");
    std::smatch match;
    auto parseYear = [](const std::string& text) -> std::optional<int> {
        int value = 0;
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) return std::nullopt;
        return value;
    };
    if (std::regex_match(trimmed, match, isoYearPattern) && match.size() > 1) return parseYear(match[1].str());
    if (std::regex_match(trimmed, match, dottedYearPattern) && match.size() > 1) return parseYear(match[1].str());
    if (std::regex_match(trimmed, match, slashYearPattern) && match.size() > 1) return parseYear(match[1].str());
    if (std::regex_search(trimmed, match, standaloneYearPattern) && match.size() > 1) return parseYear(match[1].str());
    return std::nullopt;
}

AnnualValidationResult validateTransactionsForYear(int year, const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions) {
    AnnualValidationResult result;
    if (!isValidYear(year)) {
        result.addIssue("", "annual year is invalid");
        return result;
    }
    for (const auto& transaction : transactions) {
        if (!transaction) continue;
        const auto bookingYear = extractBookingYear(transaction->bookingDate());
        if (!bookingYear) {
            result.addIssue(transaction->id(), "booking date does not contain a valid year");
            continue;
        }
        if (*bookingYear != year) result.addIssue(transaction->id(), "booking date year does not match annual year");
    }
    return result;
}

} // namespace core::domain::policies::annual
