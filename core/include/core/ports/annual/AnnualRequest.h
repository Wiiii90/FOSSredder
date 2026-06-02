/**
 * @file core/include/core/ports/annual/AnnualRequest.h
 * @brief Declares the annual query request port contract.
 */

#pragma once

#include <string>

namespace core::ports::annual {

struct AnnualRequest {
    std::string annualId;

    [[nodiscard]] bool empty() const noexcept {
        return annualId.empty();
    }
};

} // namespace core::ports::annual
