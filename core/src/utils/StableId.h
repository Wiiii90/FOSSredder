/**
 * @file core/src/utils/StableId.h
 * @brief Declares private stable identifier helpers for `core` entity creation.
 */

#pragma once

#include <string>

namespace core::utils {

/**
 * @brief Generates a stable hex identifier for persisted entities.
 */
std::string makeStableId();

}
