/**
 * @file core/include/core/domain/policies/AnnualPolicy.h
 * @brief Shared annual validation helpers.
 */

#pragma once

#include <charconv>
#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <vector>
#include <utility>

#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Year.h"

namespace core::domain::policies::annual {

class AnnualValidationIssue {
public:
    AnnualValidationIssue();
    AnnualValidationIssue(std::string transactionId, std::string reason);
    [[nodiscard]] const std::string& transactionId() const noexcept;
    [[nodiscard]] const std::string& reason() const noexcept;
    void setTransactionId(std::string value);
    void setReason(std::string value);
private:
    std::string transactionId_;
    std::string reason_;
};

class AnnualValidationResult {
public:
    [[nodiscard]] const std::vector<AnnualValidationIssue>& issues() const noexcept;
    void setIssues(std::vector<AnnualValidationIssue> value);
    void addIssue(std::string transactionId, std::string reason);
    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] bool hasIssues() const noexcept;
private:
    std::vector<AnnualValidationIssue> issues_;
};

/**
 * @brief Checks whether a year is valid for annual aggregates.
 * @param year Raw year value.
 * @return `true` when the year lies within the supported range.
 */
bool isValidYear(int year);

/**
 * @brief Checks whether an annual aggregate is structurally valid.
 * @param year Raw year value.
 * @param analysisIds Analysis id list.
 * @return `true` when both year and analysis ids are valid.
 */
bool isValidAnnual(int year, const std::vector<std::string>& analysisIds);

inline std::string normalizeId(std::string value);

/**
 * @brief Checks whether an analysis id exists in a list.
 * @param analysisIds Analysis id list to inspect.
 * @param analysisId Analysis id to search for.
 * @return `true` when the id exists.
 */
bool containsAnalysisId(const std::vector<std::string>& analysisIds, const std::string& analysisId);

/**
 * @brief Checks whether all analysis ids are unique.
 * @param analysisIds Analysis id list to inspect.
 * @return `true` when every non-empty id appears only once.
 */
bool hasUniqueAnalysisIds(const std::vector<std::string>& analysisIds);

/**
 * @brief Checks whether a list contains all required analysis ids.
 * @param analysisIds Analysis id list to inspect.
 * @param requiredIds Required analysis ids.
 * @return `true` when every required id is present.
 */
bool containsAllAnalysisIds(const std::vector<std::string>& analysisIds,
                            const std::vector<std::string>& requiredIds);

/**
 * @brief Trims leading and trailing whitespace from an id.
 * @param value Raw id text.
 * @return Trimmed id text or an empty string.
 */
std::string normalizeId(std::string value);

/**
 * @brief Normalizes and deduplicates analysis ids.
 * @param values Analysis id list to mutate.
 */
void normalizeIds(std::vector<std::string>& values);

/**
 * @brief Extracts the year component from a booking date.
 * @param bookingDate Raw booking date text.
 * @return Parsed year when available.
 */
std::optional<int> extractBookingYear(const std::string& bookingDate);

/**
 * @brief Validates that transaction booking years match the annual year.
 * @param year Annual year.
 * @param transactions Transactions to validate.
 * @return Validation result with optional issues.
 */
AnnualValidationResult validateTransactionsForYear(int year,
                                                   const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions);

} // namespace core::domain::policies::annual
