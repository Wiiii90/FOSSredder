/**
 * @file core/src/utils/Time.cpp
 * @brief Implements private UTC timestamp helpers for core implementation code.
 */

#include "Time.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace core::utils {

std::string currentTimestampUtc()
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
