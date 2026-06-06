/**
 * @file core/src/domain/values/Year.cpp
 * @brief Domain value semantics for years.
 */

#include "core/domain/values/Year.h"

namespace core::domain {

namespace {

constexpr int kMinValue = 1;
constexpr int kMaxValue = 9999;

} // namespace

Year::Year(int value)
    : value_(normalize(value)) {}

int Year::value() const noexcept { return value_; }

constexpr int Year::minValue() noexcept {
    return kMinValue;
}

constexpr int Year::maxValue() noexcept {
    return kMaxValue;
}

bool Year::isValid(int value) noexcept {
    return value >= minValue() && value <= maxValue();
}

int Year::normalize(int value) noexcept {
    return isValid(value) ? value : 0;
}

bool Year::empty() const noexcept {
    return value_ == 0;
}

} // namespace core::domain
