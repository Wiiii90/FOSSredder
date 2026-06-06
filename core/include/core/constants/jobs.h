#pragma once

#include <cstddef>
#include <string_view>

namespace core::constants::jobs {

inline constexpr std::size_t kFallbackWorkerCount = 4;
inline constexpr std::size_t kQueueCapacity = 128;
inline constexpr std::size_t kJobHistoryLimit = 64;
inline constexpr std::size_t kOcrWorkerDivisor = 2;

namespace messages {
inline constexpr std::string_view kQueued = "Queued";
inline constexpr std::string_view kRunning = "Running";
inline constexpr std::string_view kFinished = "Finished";
inline constexpr std::string_view kFailed = "Failed";
inline constexpr std::string_view kCanceled = "Canceled";
inline constexpr std::string_view kImportServiceUnavailable = "core::domain::Statement import service not available";
inline constexpr std::string_view kUnknownError = "Unknown error";
} // namespace messages

} // namespace core::constants::jobs
