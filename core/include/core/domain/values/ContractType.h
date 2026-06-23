/**
 * @file core/include/core/domain/values/ContractType.h
 * @brief Value object for contract type labels.
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <string>
#include <utility>

namespace core::domain {

class ContractType {
public:
    const std::string& value() const noexcept;

    ContractType() = default;
    /**
     * @brief Creates a normalized contract type from raw input.
     * @param value Raw contract type text.
     */
    explicit ContractType(std::string value);

    /**
     * @brief Returns the maximum supported contract type length.
     * @return Maximum accepted character count.
     */
    static constexpr std::size_t maxLength() noexcept;

    /**
     * @brief Trims leading and trailing whitespace from a contract type.
     * @param value Raw contract type text.
     * @return Trimmed contract type text or an empty string.
     */
    static std::string normalize(std::string value);

    /**
     * @brief Checks whether a raw contract type is valid.
     * @param value Raw contract type text.
     * @return `true` when the normalized text is non-empty and within the length limit.
     */
    static bool isValid(const std::string& value);

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no normalized contract type is stored.
     */
    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
