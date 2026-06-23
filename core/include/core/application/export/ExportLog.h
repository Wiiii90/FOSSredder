/**
 * @file core/include/core/application/export/ExportLog.h
 * @brief Declares the persisted metadata record for an export run.
 */

#pragma once

#include <string>
#include <vector>

namespace core::application::exporting {

/**
 * @brief Stores metadata about one export run for persistence and workflow tracking.
 */
class ExportLog {
public:
    std::string id;
    std::string time;
    std::string targetPath;
    std::string status;
    std::string message;
    std::vector<std::string> annualIds;
    std::vector<std::string> analysisIds;
};

} // namespace core::application::exporting
