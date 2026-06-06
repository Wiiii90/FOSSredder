/**
 * @file core/src/jobs/Scheduler.cpp
 * @brief Implements the bounded worker queue and slot limiter used by import jobs.
 */

#include "core/jobs/Scheduler.h"

#include "core/errors/ErrorReporterRegistry.h"

#include <algorithm>

namespace core::jobs {

namespace {

constexpr std::size_t kMinimumWorkerCount = 1;

} // namespace

SlotLimiter::SlotLimiter(std::size_t slots)
    : slots_(std::max<std::size_t>(kMinimumWorkerCount, slots))
    , available_(std::max<std::size_t>(kMinimumWorkerCount, slots))
{
}

void SlotLimiter::acquire()
{
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [&]() { return available_ > 0; });
    --available_;
}

void SlotLimiter::release()
{
    {
        std::lock_guard<std::mutex> g(m_);
        available_ = std::min(slots_, available_ + 1);
    }
    cv_.notify_one();
}

Scheduler::Scheduler(std::size_t workers, std::size_t queueCapacity)
    : cap_(std::max<std::size_t>(kMinimumWorkerCount, queueCapacity))
{
    workers = std::max<std::size_t>(kMinimumWorkerCount, workers);
    workers_.reserve(workers);
    for (std::size_t i = 0; i < workers; ++i) {
        workers_.emplace_back([this]() { workerLoop(); });
    }
}

Scheduler::~Scheduler()
{
    stop();
}

void Scheduler::stop()
{
    {
        std::lock_guard<std::mutex> g(m_);
        if (stopping_) return;
        stopping_ = true;
    }

    cvNotEmpty_.notify_all();
    cvNotFull_.notify_all();

    for (auto& w : workers_) {
        try {
            if (w.joinable()) w.join();
        } catch (...) {
            core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                          "core::jobs::Scheduler::stop::join",
                                          std::current_exception());
        }
    }
    workers_.clear();
}

void Scheduler::enqueue(Task t)
{
    if (!t) return;

    std::unique_lock<std::mutex> lk(m_);
    cvNotFull_.wait(lk, [&]() { return stopping_ || q_.size() < cap_; });
    if (stopping_) return;
    q_.push_back(std::move(t));
    lk.unlock();
    cvNotEmpty_.notify_one();
}

void Scheduler::workerLoop()
{
    while (true) {
        Task t;
        {
            std::unique_lock<std::mutex> lk(m_);
            cvNotEmpty_.wait(lk, [&]() { return stopping_ || !q_.empty(); });
            if (stopping_ && q_.empty()) return;
            t = std::move(q_.front());
            q_.pop_front();
        }
        cvNotFull_.notify_one();

        try {
            if (t) t();
        } catch (...) {
            core::errors::reportException(core::errors::ErrorSeverity::Error,
                                          "core::jobs::Scheduler::workerLoop::task",
                                          std::current_exception());
        }
    }
}

}
