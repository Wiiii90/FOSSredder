/**
 * @file app/include/app/Environment.h
 * @brief Declares application-local environment bootstrap helpers.
 */

#pragma once

#include <string>

namespace app::runtime {

/**
 * @brief Loads key-value pairs from an environment file.
 * @param path Path to the environment file.
 * @param overwrite Whether existing process environment values are replaced.
 * @return True when the file was opened and processed.
 */
bool loadDotEnv(const std::string& path, bool overwrite = false);

} // namespace app::runtime
