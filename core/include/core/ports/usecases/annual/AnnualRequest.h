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

    [[nodiscard]] bool empty() const noexcept {
        return annualId.empty() && previewAnalysisIds.empty() &&
               previewYear == 0;
    }
};

} // namespace core::ports::annual
