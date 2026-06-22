/**
 * @file core/include/core/jobs/Scheduler.h
 * @brief Declares a bounded worker queue and a simple slot limiter for import tasks.
 */

#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace core::jobs {

/**
 * @brief Limits concurrent access to a bounded number of slots.
 */
class SlotLimiter {
public:
    explicit SlotLimiter(std::size_t slots);

    void acquire();
    void release();

private:
    std::size_t slots_;
    std::size_t available_;
    std::mutex m_;
    std::condition_variable cv_;
};

/**
 * @brief Runs background tasks on a bounded worker queue.
 */
class Scheduler {
public:
    using Task = std::function<void()>;

    Scheduler(std::size_t workers,
              std::size_t queueCapacity,
              std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void enqueue(Task t);
    void stop();

private:
    void workerLoop();

    std::deque<Task> q_;
    std::size_t cap_;
    std::mutex m_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;

    bool stopping_ = false;
    std::vector<std::thread> workers_;
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

}
