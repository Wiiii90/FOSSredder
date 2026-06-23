/**
 * @file core/include/core/domain/values/EntityName.h
 * @brief Small value object for normalized entity display names.
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <string>
#include <utility>

namespace core::domain {

class EntityName {
public:
    const std::string& value() const noexcept;

    EntityName() = default;
    /**
     * @brief Creates a normalized entity name from raw input.
     * @param value Raw entity name text.
     */
    explicit EntityName(std::string value);

    /**
     * @brief Returns the maximum supported entity name length.
     * @return Maximum accepted character count.
     */
    static constexpr std::size_t maxLength() noexcept;

    /**
     * @brief Trims leading and trailing whitespace from a name.
     * @param value Raw name text.
     * @return Trimmed name text or an empty string.
     */
    static std::string normalize(std::string value);

    /**
     * @brief Checks whether a raw name is valid.
     * @param value Raw name text.
     * @return `true` when the normalized text is non-empty and within the length limit.
     */
    static bool isValid(const std::string& value);

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no normalized name is stored.
     */
    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
