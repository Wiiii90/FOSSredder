/**
 * @file core/include/core/domain/values/Year.h
 * @brief Value object for annual year validation.
 */

#pragma once

namespace core::domain {

class Year {
public:
    int value() const noexcept;

    Year() = default;
    /**
     * @brief Creates a normalized year value from raw input.
     * @param value Raw year.
     */
    explicit Year(int value);

    /**
     * @brief Returns the minimum supported year.
     * @return Smallest accepted year.
     */
    static constexpr int minValue() noexcept;
    /**
     * @brief Returns the maximum supported year.
     * @return Largest accepted year.
     */
    static constexpr int maxValue() noexcept;

    /**
     * @brief Checks whether a raw year is valid.
     * @param value Raw year.
     * @return `true` when the year lies within the supported range.
     */
    static bool isValid(int value) noexcept;

    /**
     * @brief Normalizes a raw year value.
     * @param value Raw year.
     * @return The year when valid, otherwise `0`.
     */
    static int normalize(int value) noexcept;

    /**
     * @brief Checks whether the stored value is empty.
     * @return `true` when no valid year is stored.
     */
    bool empty() const noexcept;

private:
    int value_ = 0;
};

} // namespace core::domain
