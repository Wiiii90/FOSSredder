/**
 * @file core/src/domain/values/FilterSpec.cpp
 * @brief Domain value semantics for filter specifications.
 */

#include "core/domain/values/FilterSpec.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

namespace {

constexpr std::size_t kMaxLength = 1024;

} // namespace

FilterSpec::FilterSpec(std::string value)
    : value_(normalize(std::move(value))) {}

const std::string& FilterSpec::value() const noexcept { return value_; }

constexpr std::size_t FilterSpec::maxLength() noexcept {
    return kMaxLength;
}

std::string FilterSpec::normalize(std::string value) {
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

bool FilterSpec::isValid(const std::string& value) {
    const auto normalized = normalize(value);
    return !normalized.empty() && normalized.size() <= maxLength();
}

bool FilterSpec::empty() const noexcept {
    return value_.empty();
}

} // namespace core::domain
