/**
 * @file core/include/core/utils/Time.h
 * @brief Declares shared UTC timestamp helpers for core and persistence.
 */

#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace core::utils {

/**
 * @brief Returns the current UTC timestamp in ISO-8601 format.
 */
inline std::string currentTimestampUtc()
{
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto nowTime = clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &nowTime);
#else
    gmtime_r(&nowTime, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

}
