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
    explicit JobManager(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);

    JobId submit(JobKind kind);

    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);
    void unsubscribe(const JobId& id, SubscriptionId subId);

    void cancel(const JobId& id);
    void pause(const JobId& id);
    void resume(const JobId& id);

    std::optional<JobSnapshot> snapshot(const JobId& id) const;

    std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;
    std::shared_ptr<std::atomic<bool>> pauseFlag(const JobId& id) const;

    void publish(const JobEvent& ev);
    void fail(const JobId& id, const std::string& error);
    void finish(const JobId& id);
    void start(const JobId& id);

private:
    void prune(std::size_t maxJobs);

    struct JobData {
        JobSnapshot snap;
        std::shared_ptr<std::atomic<bool>> cancel;
        std::shared_ptr<std::atomic<bool>> pause;
        std::unordered_map<SubscriptionId, JobEventCallback> subs;
        SubscriptionId nextSub = 1;
        mutable std::mutex m;
    };

    static JobId makeJobId();

    static constexpr std::size_t kMaxJobs = core::constants::jobs::kJobHistoryLimit;

    std::unordered_map<JobId, std::shared_ptr<JobData>> jobs_;
    std::deque<JobId> order_;
    mutable std::mutex jobsMutex_;
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

}
