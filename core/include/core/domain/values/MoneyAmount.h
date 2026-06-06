/**
 * @file core/include/core/domain/values/MoneyAmount.h
 * @brief Value object for monetary transaction amounts.
 */

#pragma once

#include <cmath>

namespace core::domain {

class MoneyAmount {
public:
    double value() const noexcept;

    MoneyAmount() = default;
    /**
     * @brief Creates a monetary amount from a raw numeric value.
     * @param value Raw amount.
     */
    explicit MoneyAmount(double value);

    /**
     * @brief Checks whether a raw amount is finite.
     * @param value Raw amount.
     * @return `true` when the amount is finite.
     */
    static bool isValid(double value) noexcept {
        return std::isfinite(value);
    }
private:
    double value_ = 0.0;
};

} // namespace core::domain
