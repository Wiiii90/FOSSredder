/**
 * @file core/src/jobs/JobSystem.cpp
 * @brief Implements the generic job system facade.
 */

#include "core/jobs/JobSystem.h"

#include "JobManager.h"
#include "core/jobs/Scheduler.h"

#include <algorithm>
#include <thread>
#include <utility>

namespace {

constexpr unsigned int kSingleCpuFallbackThreshold = 2;
constexpr std::size_t kSingleWorkerFallback = 1;
constexpr std::size_t kFallbackWorkerCount = 4;
constexpr std::size_t kQueueCapacity = 128;
constexpr std::size_t kSlotLimiterWorkerDivisor = 2;

std::size_t defaultWorkers()
{
    const auto hc = std::thread::hardware_concurrency();
    if (hc == 0) return kFallbackWorkerCount;
    if (hc <= kSingleCpuFallbackThreshold) return kSingleWorkerFallback;
    return static_cast<std::size_t>(hc - 1);
}

std::size_t resolveWorkerCount(std::size_t workers)
{
    return workers == 0 ? defaultWorkers() : workers;
}

}

namespace core::jobs {

class JobSystem::Impl {
public:
    Impl(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
         std::size_t workers)
        : manager(errorReporter)
        , scheduler(resolveWorkerCount(workers), kQueueCapacity, std::move(errorReporter))
        , slotLimiter(std::max<std::size_t>(std::size_t{1}, resolveWorkerCount(workers) / kSlotLimiterWorkerDivisor))
    {
    }

    JobManager manager;
    Scheduler scheduler;
    SlotLimiter slotLimiter;
};

JobSystem::JobSystem(
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    std::size_t workers)
    : impl_(std::make_unique<Impl>(std::move(errorReporter), workers)) {
}

JobSystem::~JobSystem() = default;

JobSystem::JobSystem(JobSystem&&) noexcept = default;

JobSystem& JobSystem::operator=(JobSystem&&) noexcept = default;

JobId JobSystem::submit(JobKind kind)
{
    return impl_->manager.submit(kind);
}

void JobSystem::start(const JobId& id)
{
    impl_->manager.start(id);
}

void JobSystem::publish(const JobEvent& event)
{
    impl_->manager.publish(event);
}

void JobSystem::fail(const JobId& id, const std::string& error)
{
    impl_->manager.fail(id, error);
}

void JobSystem::finish(const JobId& id)
{
    impl_->manager.finish(id);
}

std::shared_ptr<std::atomic<bool>> JobSystem::cancelFlag(const JobId& id) const
{
    return impl_->manager.cancelFlag(id);
}

std::shared_ptr<std::atomic<bool>> JobSystem::pauseFlag(const JobId& id) const
{
    return impl_->manager.pauseFlag(id);
}

Scheduler& JobSystem::scheduler()
{
    return impl_->scheduler;
}

SlotLimiter& JobSystem::slotLimiter()
{
    return impl_->slotLimiter;
}

SubscriptionId JobSystem::subscribe(const JobId& id, JobEventCallback cb)
{
    return impl_->manager.subscribe(id, std::move(cb));
}

void JobSystem::unsubscribe(const JobId& id, SubscriptionId subId)
{
    impl_->manager.unsubscribe(id, subId);
}

void JobSystem::cancel(const JobId& id)
{
    impl_->manager.cancel(id);
}

void JobSystem::pause(const JobId& id)
{
    impl_->manager.pause(id);
}

void JobSystem::resume(const JobId& id)
{
    impl_->manager.resume(id);
}

std::optional<JobSnapshot> JobSystem::snapshot(const JobId& id) const
{
    return impl_->manager.snapshot(id);
}

void JobSystem::shutdown()
{
    impl_->scheduler.stop();
}

}
