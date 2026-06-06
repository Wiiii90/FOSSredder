/**
 * @file core/src/domain/values/MoneyAmount.cpp
 * @brief Domain value semantics for money amounts.
 */

#include "core/domain/values/MoneyAmount.h"

#include <cmath>

namespace core::domain {

MoneyAmount::MoneyAmount(double value)
    : value_(value) {}

double MoneyAmount::value() const noexcept { return value_; }

} // namespace core::domain
