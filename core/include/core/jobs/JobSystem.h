/**
 * @file core/include/core/jobs/JobSystem.h
 * @brief Declares the generic job system facade.
 */

#pragma once

#include "core/jobs/JobTypes.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace core::jobs {

class Scheduler;
class SlotLimiter;

class JobSystem {
public:
    /**
     * @brief Create a generic job system.
     * @param errorReporter Reporter used for background job diagnostics.
     * @param workers Worker count for the scheduler.
     */
    explicit JobSystem(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
              std::size_t workers = 0);

    /**
     * @brief Destroy the job system.
     */
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) noexcept;
    JobSystem& operator=(JobSystem&&) noexcept;

    [[nodiscard]] JobId submit(JobKind kind = JobKind::Generic);
    void start(const JobId& id);
    void publish(const JobEvent& event);
    void fail(const JobId& id, const std::string& error);
    void finish(const JobId& id);
    [[nodiscard]] std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;
    [[nodiscard]] std::shared_ptr<std::atomic<bool>> pauseFlag(const JobId& id) const;
    [[nodiscard]] Scheduler& scheduler();
    [[nodiscard]] SlotLimiter& slotLimiter();

    /**
     * @brief Subscribe to job events.
     * @param id Job identifier.
     * @param cb Event callback.
     * @return Subscription identifier.
     */
    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);

    /**
     * @brief Unsubscribe from job events.
     * @param id Job identifier.
     * @param subId Subscription identifier.
     */
    void unsubscribe(const JobId& id, SubscriptionId subId);

    /**
     * @brief Cancel a job.
     * @param id Job identifier.
     */
    void cancel(const JobId& id);
    void pause(const JobId& id);
    void resume(const JobId& id);

    /**
     * @brief Retrieve a job snapshot.
     * @param id Job identifier.
     * @return Job snapshot, if the job exists.
     */
    std::optional<JobSnapshot> snapshot(const JobId& id) const;

    /**
     * @brief Shut down the job system.
     */
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
