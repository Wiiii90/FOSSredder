#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace core::jobs {

using JobId = std::string;

enum class JobKind {
    ImportStatement,
    Export,
};

enum class JobState {
    Pending,
    Running,
    Paused,
    Finished,
    Failed,
    Canceled,
};

enum class JobStage {
    None,
    Render,
    Extract,
    Detect,
    Ocr,
    Parse,
    Finalize,
};

struct JobEvent {
    JobId jobId;
    JobKind kind = JobKind::ImportStatement;
    JobState state = JobState::Pending;
    JobStage stage = JobStage::None;

    double progress = 0.0;
    std::string message;

    int pageIndex = -1;
    int pageCount = -1;

    double elapsedSeconds = 0.0;
};

struct JobSnapshot {
    JobId jobId;
    JobKind kind = JobKind::ImportStatement;
    JobState state = JobState::Pending;
    JobStage stage = JobStage::None;

    double progress = 0.0;
    std::string message;

    int pageIndex = -1;
    int pageCount = -1;

    std::string error;
};

using SubscriptionId = std::uint64_t;
using JobEventCallback = std::function<void(const JobEvent&)>;

}
