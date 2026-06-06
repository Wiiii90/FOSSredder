/**
 * @file core/include/core/jobs/ImportJobSpec.h
 * @brief Declares the public import job submission contract.
 */

#pragma once

#include <string>

namespace core::jobs {

/**
 * @brief Describes an import-statement job submission.
 */
struct ImportStatementJobSpec {
    /**
     * @brief Source file path for the import job.
     */
    std::string sourcePath;

    /**
     * @brief Root directory for job outputs.
     */
    std::string runRoot;

    /**
     * @brief Prefix used when generating run identifiers.
     */
    std::string runIdPrefix;
};

}
