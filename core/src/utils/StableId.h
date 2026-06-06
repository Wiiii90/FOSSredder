/**
 * @file core/src/utils/StableId.h
 * @brief Declares private stable identifier helpers for `core` entity creation.
 */

#pragma once

#include "core/constants/ids.h"

#include <random>
#include <string>

namespace core::utils {

/**
 * @brief Generates a stable hex identifier for persisted entities.
 */
inline std::string makeStableId()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(core::constants::ids::kHexAlphabet.size() - 1));

    std::string out;
    out.reserve(core::constants::ids::kStableHexLength);
    for (std::size_t i = 0; i < core::constants::ids::kStableHexLength; ++i) {
        out.push_back(core::constants::ids::kHexAlphabet[static_cast<std::size_t>(dist(rng))]);
    }
    return out;
}

}
