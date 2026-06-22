/**
 * @file core/src/jobs/JobManager.h
 * @brief Declares the private generic job state manager used behind `JobSystem`.
 */

#pragma once

#include "core/constants/jobs.h"
#include "core/jobs/JobTypes.h"

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace core::jobs {

class JobManager {
public:
    /**
     * @brief Create a manager for generic job state and event delivery.
     * @param errorReporter Reporter used when callbacks or flag updates throw.
     */
    explicit JobManager(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);

    /**
     * @brief Create a tracked job in the pending state.
     * @param kind Kind of job to create.
     * @return Identifier of the created job.
     */
    JobId submit(JobKind kind);

    /**
     * @brief Subscribe to events for a job.
     * @param id Job identifier.
     * @param cb Callback to invoke when events are published.
     * @return Subscription identifier, or zero when the job or callback is invalid.
     */
    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);

    /**
     * @brief Remove a job event subscription.
     * @param id Job identifier.
     * @param subId Subscription identifier.
     */
    void unsubscribe(const JobId& id, SubscriptionId subId);

    /**
     * @brief Mark a job as canceled and set its cancellation flag.
     * @param id Job identifier.
     */
    void cancel(const JobId& id);

    /**
     * @brief Mark a running job as paused and set its pause flag.
     * @param id Job identifier.
     */
    void pause(const JobId& id);

    /**
     * @brief Resume a paused job and clear its pause flag.
     * @param id Job identifier.
     */
    void resume(const JobId& id);

    /**
     * @brief Retrieve the latest state snapshot for a job.
     * @param id Job identifier.
     * @return Snapshot when the job exists.
     */
    std::optional<JobSnapshot> snapshot(const JobId& id) const;

    /**
     * @brief Retrieve a job cancellation flag.
     * @param id Job identifier.
     * @return Shared cancellation flag, or null when the job is unknown.
     */
    std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;

    /**
     * @brief Retrieve a job pause flag.
     * @param id Job identifier.
     * @return Shared pause flag, or null when the job is unknown.
     */
    std::shared_ptr<std::atomic<bool>> pauseFlag(const JobId& id) const;

    /**
     * @brief Publish an event to subscribers and update the stored snapshot.
     * @param ev Event to publish.
     */
    void publish(const JobEvent& ev);

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
     * @brief Mark a job as running.
     * @param id Job identifier.
     */
    void start(const JobId& id);

private:
    /**
     * @brief Evict old terminal jobs without subscribers.
     * @param maxJobs Maximum number of jobs to retain.
     */
    void prune(std::size_t maxJobs);

    struct JobData {
        JobSnapshot snap;
        std::shared_ptr<std::atomic<bool>> cancel;
        std::shared_ptr<std::atomic<bool>> pause;
        std::unordered_map<SubscriptionId, JobEventCallback> subs;
        SubscriptionId nextSub = 1;
        mutable std::mutex m;
    };

    /**
     * @brief Generate a new unique job identifier.
     * @return Generated job identifier.
     */
    static JobId makeJobId();

    static constexpr std::size_t kMaxJobs = core::constants::jobs::kJobHistoryLimit;

    std::unordered_map<JobId, std::shared_ptr<JobData>> jobs_;
    std::deque<JobId> order_;
    mutable std::mutex jobsMutex_;
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

}
