/**
 * @file core/src/domain/values/ExportFormat.cpp
 * @brief Domain value semantics for export formats.
 */

#include "core/domain/values/ExportFormat.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

namespace {

constexpr std::size_t kMaxLength = 64;

} // namespace

ExportFormat::ExportFormat(std::string value)
    : value_(normalize(std::move(value))) {}

const std::string& ExportFormat::value() const noexcept { return value_; }

constexpr std::size_t ExportFormat::maxLength() noexcept {
    return kMaxLength;
}

std::string ExportFormat::normalize(std::string value) {
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

bool ExportFormat::isValid(const std::string& value) {
    const auto normalized = normalize(value);
    return !normalized.empty() && normalized.size() <= maxLength();
}

bool ExportFormat::empty() const noexcept {
    return value_.empty();
}

} // namespace core::domain
