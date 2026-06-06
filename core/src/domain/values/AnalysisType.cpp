/**
 * @file core/src/domain/values/AnalysisType.cpp
 * @brief Domain value semantics for analysis types.
 */

#include "core/domain/values/AnalysisType.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

namespace {

constexpr std::size_t kMaxLength = 64;

} // namespace

AnalysisType::AnalysisType(std::string value)
    : value_(normalize(std::move(value))) {}

const std::string& AnalysisType::value() const noexcept { return value_; }

constexpr std::size_t AnalysisType::maxLength() noexcept {
    return kMaxLength;
}

std::string AnalysisType::normalize(std::string value) {
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

bool AnalysisType::isValid(const std::string& value) {
    const auto normalized = normalize(value);
    return !normalized.empty() && normalized.size() <= maxLength();
}

bool AnalysisType::empty() const noexcept {
    return value_.empty();
}

} // namespace core::domain
