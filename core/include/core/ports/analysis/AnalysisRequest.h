/**
 * @file core/include/core/ports/analysis/AnalysisRequest.h
 * @brief Declares analysis use-case request DTOs and filter-spec helpers.
 */

#pragma once

#include <string>
#include <vector>

namespace core::ports::analysis {

struct AnalysisRequest {
    std::string analysisId;
    std::string filterSpecification;

    [[nodiscard]] bool empty() const noexcept {
        return analysisId.empty() && filterSpecification.empty();
    }
};

struct AnalysisFilterSelection {
    std::string dateField = "bookingDate";
    std::string dateMode = "year";
    std::string year;
    std::string dateFrom;
    std::string dateTo;
    std::vector<std::string> propertyIds;
    bool propertyIdsUnassigned = false;
    std::vector<std::string> contractTypes;
    bool contractTypesUnassigned = false;
    std::string allocatableMode = "all";
};

[[nodiscard]] AnalysisFilterSelection
parseAnalysisFilterSelection(const std::string& filterSpec);
[[nodiscard]] std::string
buildAnalysisFilterSpec(const AnalysisFilterSelection& selection);

} // namespace core::ports::analysis
