/**
 * @file core/include/core/domain/values/FilterSpec.h
 * @brief Value object for analysis filter specifications.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace core::domain {

class FilterSpec {
public:
    const std::string& value() const noexcept;

    FilterSpec() = default;
    /**
     * @brief Creates a normalized filter specification from raw input.
     * @param value Raw filter specification text.
     */
    explicit FilterSpec(std::string value);

    /**
     * @brief Returns the maximum supported filter specification length.
     * @return Maximum accepted character count.
     */
    static constexpr std::size_t maxLength() noexcept;

    /**
     * @brief Trims leading and trailing whitespace from a filter specification.
     * @param value Raw filter specification text.
     * @return Trimmed filter specification text or an empty string.
     */
    static std::string normalize(std::string value);

    /**
     * @brief Checks whether a raw filter specification is valid.
     * @param value Raw filter specification text.
     * @return `true` when the normalized text is non-empty and within the length limit.
     */
    static bool isValid(const std::string& value);

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no normalized filter specification is stored.
     */
    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
