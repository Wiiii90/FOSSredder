/**
 * @file core/src/domain/values/BookingDate.cpp
 * @brief Domain value semantics for booking dates.
 */

#include "core/domain/values/BookingDate.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

namespace {

constexpr std::size_t kMaxLength = 32;

} // namespace

BookingDate::BookingDate(std::string value)
    : value_(normalize(std::move(value))) {}

const std::string& BookingDate::value() const noexcept { return value_; }

constexpr std::size_t BookingDate::maxLength() noexcept {
    return kMaxLength;
}

std::string BookingDate::normalize(std::string value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

bool BookingDate::isValid(const std::string& value) {
    const auto normalized = normalize(value);
    return !normalized.empty() && normalized.size() <= maxLength();
}

bool BookingDate::empty() const noexcept {
    return value_.empty();
}

} // namespace core::domain
