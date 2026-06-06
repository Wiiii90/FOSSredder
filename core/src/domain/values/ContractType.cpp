/**
 * @file core/src/domain/values/ContractType.cpp
 * @brief Domain value semantics for contract types.
 */

#include "core/domain/values/ContractType.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

namespace {

constexpr std::size_t kMaxLength = 255;

} // namespace

ContractType::ContractType(std::string value)
    : value_(normalize(std::move(value))) {}

const std::string& ContractType::value() const noexcept { return value_; }

constexpr std::size_t ContractType::maxLength() noexcept {
    return kMaxLength;
}

std::string ContractType::normalize(std::string value) {
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

bool ContractType::isValid(const std::string& value) {
    const auto normalized = normalize(value);
    return !normalized.empty() && normalized.size() <= maxLength();
}

bool ContractType::empty() const noexcept {
    return value_.empty();
}

} // namespace core::domain
