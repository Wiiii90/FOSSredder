/**
 * @file core/src/utils/TransientId.h
 * @brief Declares private unique identifier helpers for transient import/job
 * artifacts.
 */

#pragma once

#include <string>

namespace core::utils {

/**
 * @brief Generates a transient unique identifier for import and job artifacts.
 */
std::string makeTransientId();

} // namespace core::utils
