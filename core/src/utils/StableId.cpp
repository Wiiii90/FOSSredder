/**
 * @file core/src/utils/StableId.cpp
 * @brief Implements private stable identifier helpers for `core` entity creation.
 */

#include "StableId.h"

#include <cstddef>
#include <random>
#include <string_view>

namespace core::utils {

namespace {

inline constexpr std::size_t kStableHexLength = 32;
inline constexpr std::string_view kHexAlphabet = "0123456789abcdef";

}

std::string makeStableId()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(kHexAlphabet.size() - 1));

    std::string out;
    out.reserve(kStableHexLength);
    for (std::size_t i = 0; i < kStableHexLength; ++i) {
        out.push_back(kHexAlphabet[static_cast<std::size_t>(dist(rng))]);
    }
    return out;
}

}
