/**
 * @file core/src/utils/TransientId.cpp
 * @brief Implements private unique identifier helpers for transient import/job artifacts.
 */

#include "TransientId.h"

#include <atomic>
#include <chrono>
#include <cstdint>

namespace core::utils {

std::string makeTransientId()
{
    static std::atomic<std::uint32_t> counter{0};
    const auto now = std::chrono::system_clock::now();
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    const std::uint32_t c = counter.fetch_add(1, std::memory_order_relaxed);
    return std::to_string(us) + "_" + std::to_string(c);
}

}
