/**
 * @file core/include/core/ports/usecases/analysis/AnalysisRequest.h
 * @brief Declares analysis use-case request DTOs and filter-spec helpers.
 */

#pragma once

#include <string>
#include <vector>

namespace core::ports::analysis {

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

struct AnalysisRequest {
    std::string analysisId;
    AnalysisFilterSelection filter;

    /**
     * @brief Reports whether the request has no target.
     * @return True when no analysis id is set.
     */
    [[nodiscard]] bool empty() const noexcept {
        return analysisId.empty();
    }
};

struct AnalysisConfigInput {
    std::string type;
    std::string plotType;
    std::string plotMeasure;
    std::vector<std::string> propertyIds;
    std::vector<std::string> contractTypes;
    double taxPercent = 0.0;
};

struct AnalysisAdjustmentTransactionInput {
    std::string id;
    double amount = 0.0;
};

/**
 * @brief Parses a serialized analysis filter specification.
 * @param filterSpec Serialized filter specification.
 * @return Parsed filter selection.
 */
[[nodiscard]] AnalysisFilterSelection
parseAnalysisFilterSelection(const std::string& filterSpec);
/**
 * @brief Serializes an analysis filter selection.
 * @param selection Filter selection.
 * @return Serialized filter specification.
 */
[[nodiscard]] std::string
buildAnalysisFilterSpec(const AnalysisFilterSelection& selection);

} // namespace core::ports::analysis
