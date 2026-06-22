/**
 * @file core/src/jobs/JobManager.cpp
 * @brief Implements private job state tracking and event delivery.
 */

#include "JobManager.h"

#include "core/errors/ErrorReporting.h"
#include "../utils/TransientId.h"

#include <utility>

namespace core::jobs {

namespace {

inline constexpr auto kQueued = "Queued";
inline constexpr auto kRunning = "Running";
inline constexpr auto kFinished = "Finished";
inline constexpr auto kFailed = "Failed";
inline constexpr auto kCanceled = "Canceled";

JobEvent snapshotToEvent(const JobSnapshot& snapshot, std::string message = {})
{
    JobEvent event;
    event.jobId = snapshot.jobId;
    event.kind = snapshot.kind;
    event.state = snapshot.state;
    event.stage = snapshot.stage;
    event.progress = snapshot.progress;
    event.message = message.empty() ? snapshot.message : std::move(message);
    event.pageIndex = snapshot.pageIndex;
    event.pageCount = snapshot.pageCount;
    return event;
}

void applyEventToSnapshot(JobSnapshot& snapshot, const JobEvent& event)
{
    snapshot.kind = event.kind;
    snapshot.state = event.state;
    snapshot.stage = event.stage;
    snapshot.progress = event.progress;
    if (!event.message.empty()) snapshot.message = event.message;
    snapshot.pageIndex = event.pageIndex;
    snapshot.pageCount = event.pageCount;
}

}

JobManager::JobManager(
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter)
    : errorReporter_(std::move(errorReporter)) {}

JobId JobManager::makeJobId() {
    return core::utils::makeTransientId();
}

JobId JobManager::submit(JobKind kind) {
    auto data = std::make_shared<JobData>();
    data->snap.jobId = makeJobId();
    data->snap.kind = kind;
    data->snap.state = JobState::Pending;
    data->snap.stage = JobStage::None;
    data->snap.progress = 0.0;
    data->snap.message = std::string(kQueued);
    data->cancel = std::make_shared<std::atomic<bool>>(false);
    data->pause = std::make_shared<std::atomic<bool>>(false);

    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        jobs_.emplace(data->snap.jobId, data);
        order_.push_back(data->snap.jobId);
    }
    prune(kMaxJobs);
    return data->snap.jobId;
}

SubscriptionId JobManager::subscribe(const JobId& id, JobEventCallback cb) {
    if (!cb) return 0;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return 0;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    SubscriptionId sid = job->nextSub++;
    job->subs.emplace(sid, std::move(cb));
    return sid;
}

void JobManager::unsubscribe(const JobId& id, SubscriptionId subId) {
    if (subId == 0) return;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    job->subs.erase(subId);
}

void JobManager::cancel(const JobId& id) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    if (job->cancel) {
        try { job->cancel->store(true); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::jobs::JobManager::cancel::store", std::current_exception()); }
    }

    JobEvent ev;
    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Canceled;
        job->snap.message = std::string(kCanceled);
        ev = snapshotToEvent(job->snap);
    }
    publish(ev);

    prune(kMaxJobs);
}

void JobManager::pause(const JobId& id) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    if (job->pause) {
        try { job->pause->store(true); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::jobs::JobManager::pause::store", std::current_exception()); }
    }

    JobEvent ev;
    {
        std::lock_guard<std::mutex> g(job->m);
        if (job->snap.state != JobState::Running) return;
        job->snap.state = JobState::Paused;
        job->snap.message = "Paused";
        ev = snapshotToEvent(job->snap);
    }
    publish(ev);
}

void JobManager::resume(const JobId& id) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    if (job->pause) {
        try { job->pause->store(false); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::jobs::JobManager::resume::store", std::current_exception()); }
    }

    JobEvent ev;
    {
        std::lock_guard<std::mutex> g(job->m);
        if (job->snap.state != JobState::Paused) return;
        job->snap.state = JobState::Running;
        job->snap.message = std::string(kRunning);
        ev = snapshotToEvent(job->snap);
    }
    publish(ev);
}

std::optional<JobSnapshot> JobManager::snapshot(const JobId& id) const {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return std::nullopt;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    return job->snap;
}

std::shared_ptr<std::atomic<bool>> JobManager::cancelFlag(const JobId& id) const {
    std::lock_guard<std::mutex> g(jobsMutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) return nullptr;
    return it->second->cancel;
}

std::shared_ptr<std::atomic<bool>> JobManager::pauseFlag(const JobId& id) const {
    std::lock_guard<std::mutex> g(jobsMutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) return nullptr;
    return it->second->pause;
}

void JobManager::publish(const JobEvent& ev) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(ev.jobId);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::vector<JobEventCallback> cbs;
    {
        std::lock_guard<std::mutex> g(job->m);
        applyEventToSnapshot(job->snap, ev);

        cbs.reserve(job->subs.size());
        for (auto& kv : job->subs) cbs.push_back(kv.second);
    }

    for (auto& cb : cbs) {
        try { cb(ev); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::jobs::JobManager::publish::callback", std::current_exception()); }
    }
}

void JobManager::start(const JobId& id) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Running;
        job->snap.message = std::string(kRunning);
        ev = snapshotToEvent(job->snap);
    }

    publish(ev);

    prune(kMaxJobs);
}

void JobManager::finish(const JobId& id) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Finished;
        job->snap.progress = 1.0;
        job->snap.message = std::string(kFinished);
        ev = snapshotToEvent(job->snap);
    }

    publish(ev);

    prune(kMaxJobs);
}

void JobManager::prune(std::size_t maxJobs) {
    if (maxJobs < 1) maxJobs = 1;

    std::lock_guard<std::mutex> g(jobsMutex_);
    if (jobs_.size() <= maxJobs) return;

    // Attempt to evict terminal jobs without subscribers in FIFO order.
    // If none are evictable, keep them (avoid deleting running/subscribed jobs).
    std::size_t scanned = 0;
    const std::size_t limit = order_.size();
    while (jobs_.size() > maxJobs && scanned < limit && !order_.empty()) {
        JobId id = std::move(order_.front());
        order_.pop_front();
        ++scanned;

        auto it = jobs_.find(id);
        if (it == jobs_.end()) continue;
        auto job = it->second;
        if (!job) {
            jobs_.erase(it);
            continue;
        }

        bool evictable = false;
        {
            std::lock_guard<std::mutex> gj(job->m);
            const auto st = job->snap.state;
            const bool terminal = (st == JobState::Finished || st == JobState::Failed || st == JobState::Canceled);
            evictable = terminal && job->subs.empty();
        }

        if (evictable) {
            jobs_.erase(it);
            continue;
        }

        // keep it in rotation
        order_.push_back(std::move(id));
    }
}

void JobManager::fail(const JobId& id, const std::string& error) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Failed;
        job->snap.error = error;
        job->snap.message = std::string(kFailed);
        ev = snapshotToEvent(job->snap, error);
    }

    publish(ev);
}

}
