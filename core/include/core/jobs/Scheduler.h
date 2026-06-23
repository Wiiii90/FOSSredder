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
    /**
     * @brief Create a limiter with at least one available slot.
     * @param slots Maximum number of concurrent acquisitions.
     */
    explicit SlotLimiter(std::size_t slots);

    /**
     * @brief Block until a slot is available and acquire it.
     */
    void acquire();

    /**
     * @brief Release one acquired slot and wake a waiting caller.
     */
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

    /**
     * @brief Create a scheduler with bounded workers and queue capacity.
     * @param workers Number of worker threads to start.
     * @param queueCapacity Maximum number of pending tasks.
     * @param errorReporter Reporter used for uncaught task exceptions.
     */
    Scheduler(std::size_t workers,
              std::size_t queueCapacity,
              std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);

    /**
     * @brief Stop workers and destroy the scheduler.
     */
    ~Scheduler();

    /**
     * @brief Copying a scheduler is disabled.
     */
    Scheduler(const Scheduler&) = delete;

    /**
     * @brief Copy-assigning a scheduler is disabled.
     * @return Reference to this scheduler.
     */
    Scheduler& operator=(const Scheduler&) = delete;

    /**
     * @brief Enqueue a task, blocking when the queue is full.
     * @param t Task to run on a worker thread.
     */
    void enqueue(Task t);

    /**
     * @brief Stop accepting new tasks and join all worker threads.
     */
    void stop();

private:
    /**
     * @brief Worker loop that drains queued tasks until the scheduler stops.
     */
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
