/**
 * @file core/include/core/application/import/ImportRequest.h
 * @brief Declares the import execution request contract.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace core { namespace jobs { class Scheduler; class SlotLimiter; } }

namespace core::application::importing {

/**
 * @brief Describes one import execution request.
 */
struct ImportRequest {
    std::string sourcePath;
    std::string runRoot;
    std::string runIdPrefix;
    std::string jobId;
    std::function<void(double, const std::string&)> progressCallback;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
    std::shared_ptr<std::atomic<bool>> pauseFlag;
    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;
};

}
