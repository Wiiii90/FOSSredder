/**
 * @file core/include/core/domain/values/ExportFormat.h
 * @brief Value object for export format labels.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace core::domain {

class ExportFormat {
public:
    const std::string& value() const noexcept;

    ExportFormat() = default;
    /**
     * @brief Creates a normalized export format from raw input.
     * @param value Raw export format text.
     */
    explicit ExportFormat(std::string value);

    /**
     * @brief Returns the maximum supported export format length.
     * @return Maximum accepted character count.
     */
    static constexpr std::size_t maxLength() noexcept;

    /**
     * @brief Trims leading and trailing whitespace from an export format.
     * @param value Raw export format text.
     * @return Trimmed export format text or an empty string.
     */
    static std::string normalize(std::string value);

    /**
     * @brief Checks whether a raw export format is valid.
     * @param value Raw export format text.
     * @return `true` when the normalized text is non-empty and within the length limit.
     */
    static bool isValid(const std::string& value);

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no normalized export format is stored.
     */
    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
