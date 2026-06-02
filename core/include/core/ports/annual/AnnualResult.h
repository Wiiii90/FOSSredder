/**
 * @file core/include/core/ports/annual/AnnualResult.h
 * @brief Declares the annual query result port contract.
 */

#pragma once

#include <string>
#include <vector>

namespace core::ports::annual {

struct AnnualRowResult {
    std::string key;
    std::string transactionId;
    std::string transactionName;
    std::string bookingDate;
    double amount = 0.0;
    int status = 0;
    bool allocatable = false;
    std::string contractId;
    std::string contractType;
    std::string statementId;
    bool missingLive = false;
    bool mixedYear = false;
    bool calcVariant = false;
    int duplicateCount = 1;
    std::vector<std::string> sourceAnalysisIds;
    std::vector<std::string> sourceAnalysisNames;
};

struct AnnualStatsResult {
    int assignedAnalysisCount = 0;
    int snapshotTransactionCount = 0;
    int missingFromYear = 0;
    int mixedYear = 0;
    int duplicateCount = 0;
    int missingLive = 0;
    int neutral = 0;
    int unverified = 0;
    int verified = 0;
    int completed = 0;
};

struct AnnualResult {
    std::string annualId;
    std::string annualName;
    int year = 0;
    AnnualStatsResult stats;
    std::vector<AnnualRowResult> deduplicated;
    std::vector<AnnualRowResult> similar;
    std::vector<AnnualRowResult> divergent;
    std::vector<AnnualRowResult> workspaceOnly;

    [[nodiscard]] bool empty() const noexcept {
        return deduplicated.empty() && similar.empty() && divergent.empty() &&
               workspaceOnly.empty();
    }
};

} // namespace core::ports::annual
