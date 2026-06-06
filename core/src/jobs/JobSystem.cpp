/**
 * @file core/src/jobs/JobSystem.cpp
 * @brief Implements the import job orchestration facade.
 */

#include "core/jobs/JobSystem.h"

#include "core/constants/import.h"
#include "core/constants/jobs.h"
#include "core/application/import/ImportRequest.h"
#include "core/application/import/IImportStatement.h"
#include "core/application/import/ImportResult.h"
#include "JobManager.h"
#include "core/jobs/Scheduler.h"

#include <filesystem>
#include <thread>

namespace {

constexpr unsigned int kSingleCpuFallbackThreshold = 2;
constexpr std::size_t kSingleWorkerFallback = 1;

std::size_t defaultWorkers()
{
    const auto hc = std::thread::hardware_concurrency();
    if (hc == 0) return core::constants::jobs::kFallbackWorkerCount;
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
    Impl(std::shared_ptr<core::application::importing::IImportStatement> importService, std::size_t workers)
        : importService(std::move(importService))
        , manager()
        , scheduler(resolveWorkerCount(workers), core::constants::jobs::kQueueCapacity)
        , ocrLimiter(std::max<std::size_t>(std::size_t{1}, resolveWorkerCount(workers) / core::constants::jobs::kOcrWorkerDivisor))
    {
    }

    std::shared_ptr<core::application::importing::IImportStatement> importService;
    JobManager manager;
    Scheduler scheduler;
    SlotLimiter ocrLimiter;
};

JobSystem::JobSystem(std::shared_ptr<core::application::importing::IImportStatement> importService, std::size_t workers)
    : impl_(std::make_unique<Impl>(std::move(importService), workers)) {
}

JobSystem::~JobSystem() = default;

JobSystem::JobSystem(JobSystem&&) noexcept = default;

JobSystem& JobSystem::operator=(JobSystem&&) noexcept = default;

JobId JobSystem::startImportStatement(const ImportStatementJobSpec& spec)
{
    const JobId id = impl_->manager.submitImportStatement(spec);

    impl_->scheduler.enqueue([this, id, spec]() {
        impl_->manager.start(id);

        try {
            if (!impl_->importService) {
                impl_->manager.fail(id, std::string(core::constants::jobs::messages::kImportServiceUnavailable));
                return;
            }

            if (spec.sourcePath.empty() || !std::filesystem::exists(spec.sourcePath)) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorSourceMissing));
                return;
            }

            if (spec.runRoot.empty()) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorRunRootMissing));
                return;
            }

            const auto cancel = impl_->manager.cancelFlag(id);
            const auto pause = impl_->manager.pauseFlag(id);
            auto progressCallback = [this, id](double progress, const std::string& message) {
                JobEvent event;
                event.jobId = id;
                event.kind = JobKind::ImportStatement;
                event.state = JobState::Running;
                event.stage = JobStage::None;
                event.progress = progress;
                event.message = message;
                impl_->manager.publish(event);
            };

            core::application::importing::ImportRequest importRequest{};
            importRequest.sourcePath = spec.sourcePath;
            importRequest.runRoot = spec.runRoot;
            importRequest.runIdPrefix = spec.runIdPrefix;
            importRequest.jobId = id;
            importRequest.progressCallback = std::move(progressCallback);
            importRequest.cancelFlag = cancel;
            importRequest.pauseFlag = pause;
            importRequest.scheduler = &impl_->scheduler;
            importRequest.ocrLimiter = &impl_->ocrLimiter;

            const auto result = impl_->importService->importStatement(importRequest);
            if (!result.data) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorExtractionFailed));
                return;
            }

            impl_->manager.setStatementResult(id, result.data);
            impl_->manager.setStatementTransactions(id, std::move(result.transactions));
            impl_->manager.setStatementArtifacts(id, std::move(result.artifacts));

            if (cancel && cancel->load()) {
                impl_->manager.cancel(id);
                return;
            }

            impl_->manager.finish(id);
        } catch (const std::exception& ex) {
            impl_->manager.fail(id, ex.what());
        } catch (...) {
            impl_->manager.fail(id, std::string(core::constants::jobs::messages::kUnknownError));
        }
    });

    return id;
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

std::shared_ptr<core::domain::Statement> JobSystem::statementResult(const JobId& id) const
{
    return impl_->manager.statementResult(id);
}

std::vector<core::application::importing::draft::TransactionDraft> JobSystem::statementTransactions(const JobId& id) const
{
    return impl_->manager.statementTransactions(id);
}

std::map<std::string, std::vector<uint8_t>> JobSystem::takeStatementArtifacts(const JobId& id)
{
    return impl_->manager.takeStatementArtifacts(id);
}

void JobSystem::shutdown() {
    impl_->scheduler.stop();
}

}
