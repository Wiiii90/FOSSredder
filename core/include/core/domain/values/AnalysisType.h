/**
 * @file core/include/core/domain/values/AnalysisType.h
 * @brief Value object for analysis type labels.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace core::domain {

class AnalysisType {
public:
    const std::string& value() const noexcept;

    AnalysisType() = default;
    /**
     * @brief Creates a normalized analysis type from raw input.
     * @param value Raw analysis type text.
     */
    explicit AnalysisType(std::string value);

    /**
     * @brief Returns the maximum supported analysis type length.
     * @return Maximum accepted character count.
     */
    static constexpr std::size_t maxLength() noexcept;

    /**
     * @brief Trims leading and trailing whitespace from an analysis type.
     * @param value Raw analysis type text.
     * @return Trimmed analysis type text or an empty string.
     */
    static std::string normalize(std::string value);

    /**
     * @brief Checks whether a raw analysis type is valid.
     * @param value Raw analysis type text.
     * @return `true` when the normalized text is non-empty and within the length limit.
     */
    static bool isValid(const std::string& value);

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no normalized analysis type is stored.
     */
    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
