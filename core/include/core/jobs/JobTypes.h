/**
 * @file core/include/core/jobs/JobTypes.h
 * @brief Defines shared job identifiers, states, events, and snapshots.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace core::jobs {

/**
 * @brief Stable identifier for one tracked job.
 */
using JobId = std::string;

/**
 * @brief Classifies the high-level type of job.
 */
enum class JobKind {
    Generic,
};

/**
 * @brief Runtime lifecycle state of a job.
 */
enum class JobState {
    Pending,
    Running,
    Paused,
    Finished,
    Failed,
    Canceled,
};

/**
 * @brief Generic stage marker used by job progress events.
 */
enum class JobStage {
    None,
};

/**
 * @brief Event emitted when a job changes state or progress.
 */
struct JobEvent {
    JobId jobId;
    JobKind kind = JobKind::Generic;
    JobState state = JobState::Pending;
    JobStage stage = JobStage::None;

    double progress = 0.0;
    std::string message;

    int pageIndex = -1;
    int pageCount = -1;

    double elapsedSeconds = 0.0;
};

/**
 * @brief Snapshot of the latest known job state.
 */
struct JobSnapshot {
    JobId jobId;
    JobKind kind = JobKind::Generic;
    JobState state = JobState::Pending;
    JobStage stage = JobStage::None;

    double progress = 0.0;
    std::string message;

    int pageIndex = -1;
    int pageCount = -1;

    std::string error;
};

/**
 * @brief Identifier for a job event subscription.
 */
using SubscriptionId = std::uint64_t;

/**
 * @brief Callback invoked for published job events.
 */
using JobEventCallback = std::function<void(const JobEvent&)>;

}
