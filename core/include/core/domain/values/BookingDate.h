/**
 * @file core/include/core/domain/values/BookingDate.h
 * @brief Value object for normalized booking dates.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace core::domain {

class BookingDate {
public:
    const std::string& value() const noexcept;

    BookingDate() = default;
    explicit BookingDate(std::string value);

    static constexpr std::size_t maxLength() noexcept;

    static std::string normalize(std::string value);

    static bool isValid(const std::string& value);

    bool empty() const noexcept;

private:
    std::string value_;
};

} // namespace core::domain
