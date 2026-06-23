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

    /**
     * @brief Copying a job system is disabled.
     */
    JobSystem(const JobSystem&) = delete;

    /**
     * @brief Copy-assigning a job system is disabled.
     * @return Reference to this job system.
     */
    JobSystem& operator=(const JobSystem&) = delete;

    /**
     * @brief Move a job system facade.
     * @param other Job system to move from.
     */
    JobSystem(JobSystem&& other) noexcept;

    /**
     * @brief Move-assign a job system facade.
     * @param other Job system to move from.
     * @return Reference to this job system.
     */
    JobSystem& operator=(JobSystem&& other) noexcept;

    /**
     * @brief Create a new tracked job.
     * @param kind Kind of job to submit.
     * @return Identifier of the created job.
     */
    [[nodiscard]] JobId submit(JobKind kind = JobKind::Generic);

    /**
     * @brief Mark a job as running.
     * @param id Job identifier.
     */
    void start(const JobId& id);

    /**
     * @brief Publish a job event to subscribers and update its snapshot.
     * @param event Event to publish.
     */
    void publish(const JobEvent& event);

    /**
     * @brief Mark a job as failed.
     * @param id Job identifier.
     * @param error Error message to store and publish.
     */
    void fail(const JobId& id, const std::string& error);

    /**
     * @brief Mark a job as finished.
     * @param id Job identifier.
     */
    void finish(const JobId& id);

    /**
     * @brief Retrieve a job cancellation flag.
     * @param id Job identifier.
     * @return Shared cancellation flag, or null when the job is unknown.
     */
    [[nodiscard]] std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;

    /**
     * @brief Retrieve a job pause flag.
     * @param id Job identifier.
     * @return Shared pause flag, or null when the job is unknown.
     */
    [[nodiscard]] std::shared_ptr<std::atomic<bool>> pauseFlag(const JobId& id) const;

    /**
     * @brief Access the shared worker scheduler.
     * @return Scheduler used for background job work.
     */
    [[nodiscard]] Scheduler& scheduler();

    /**
     * @brief Access the shared slot limiter.
     * @return Slot limiter for scarce external resources.
     */
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

    /**
     * @brief Pause a running job.
     * @param id Job identifier.
     */
    void pause(const JobId& id);

    /**
     * @brief Resume a paused job.
     * @param id Job identifier.
     */
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
