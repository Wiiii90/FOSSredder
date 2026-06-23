/**
 * @file core/include/core/ports/usecases/annual/AnnualRequest.h
 * @brief Declares the annual query request port contract.
 */

#pragma once

#include <string>
#include <vector>

namespace core::ports::annual {

struct AnnualRequest {
    std::string annualId;
    std::vector<std::string> previewAnalysisIds;
    int previewYear = 0;

    /**
     * @brief Reports whether the request has no target or preview overrides.
     * @return True when all request fields are empty.
     */
    [[nodiscard]] bool empty() const noexcept {
        return annualId.empty() && previewAnalysisIds.empty() &&
               previewYear == 0;
    }
};

} // namespace core::ports::annual
