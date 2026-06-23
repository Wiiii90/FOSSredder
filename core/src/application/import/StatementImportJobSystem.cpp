/**
 * @file core/src/application/import/StatementImportJobSystem.cpp
 * @brief Implements asynchronous statement import job orchestration.
 */

#include "core/application/import/StatementImportJobSystem.h"

#include "core/application/import/IImportStatement.h"
#include "core/application/import/ImportRequest.h"
#include "core/application/import/ImportResult.h"
#include "ImportConstants.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/Scheduler.h"

#include <exception>
#include <filesystem>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace core::application::importing {

namespace {

inline constexpr std::string_view kImportServiceUnavailable =
    "core::domain::Statement import service not available";
inline constexpr std::string_view kUnknownError = "Unknown error";

} // namespace

class StatementImportJobSystem::Impl {
public:
    Impl(std::shared_ptr<IImportStatement> importService,
         std::shared_ptr<core::jobs::JobSystem> jobSystem)
        : importService(std::move(importService))
        , jobSystem(std::move(jobSystem))
    {
    }

    struct ImportJobResult {
        std::shared_ptr<core::domain::Statement> statement;
        std::vector<draft::TransactionDraft> transactions;
        std::map<std::string, std::vector<std::uint8_t>> artifacts;
    };

    void setResult(const core::jobs::JobId& id, ImportResult result)
    {
        std::lock_guard<std::mutex> lock(resultsMutex);
        auto& entry = results[id];
        entry.statement = std::move(result.data);
        entry.transactions = std::move(result.transactions);
        entry.artifacts = std::move(result.artifacts);
    }

    std::shared_ptr<IImportStatement> importService;
    std::shared_ptr<core::jobs::JobSystem> jobSystem;
    mutable std::mutex resultsMutex;
    std::unordered_map<core::jobs::JobId, ImportJobResult> results;
};

StatementImportJobSystem::StatementImportJobSystem(
    std::shared_ptr<IImportStatement> importService,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    std::size_t workers)
    : StatementImportJobSystem(
          std::move(importService),
          std::make_shared<core::jobs::JobSystem>(std::move(errorReporter), workers))
{
}

StatementImportJobSystem::StatementImportJobSystem(
    std::shared_ptr<IImportStatement> importService,
    std::shared_ptr<core::jobs::JobSystem> jobSystem)
    : impl_(std::make_unique<Impl>(std::move(importService), std::move(jobSystem)))
{
}

StatementImportJobSystem::~StatementImportJobSystem() = default;

StatementImportJobSystem::StatementImportJobSystem(StatementImportJobSystem&&) noexcept = default;

StatementImportJobSystem& StatementImportJobSystem::operator=(StatementImportJobSystem&&) noexcept = default;

core::jobs::JobId StatementImportJobSystem::startImportStatement(const ImportStatementJobSpec& spec)
{
    if (!impl_->jobSystem) {
        return {};
    }

    const auto id = impl_->jobSystem->submit();

    impl_->jobSystem->scheduler().enqueue([this, id, spec]() {
        impl_->jobSystem->start(id);

        try {
            if (!impl_->importService) {
                impl_->jobSystem->fail(id, std::string(kImportServiceUnavailable));
                return;
            }

            if (spec.sourcePath.empty() || !std::filesystem::exists(spec.sourcePath)) {
                impl_->jobSystem->fail(id, std::string(constants::kErrorSourceMissing));
                return;
            }

            if (spec.runRoot.empty()) {
                impl_->jobSystem->fail(id, std::string(constants::kErrorRunRootMissing));
                return;
            }

            const auto cancel = impl_->jobSystem->cancelFlag(id);
            const auto pause = impl_->jobSystem->pauseFlag(id);
            auto progressCallback = [this, id](double progress, const std::string& message) {
                core::jobs::JobEvent event;
                event.jobId = id;
                event.kind = core::jobs::JobKind::Generic;
                event.state = core::jobs::JobState::Running;
                event.stage = core::jobs::JobStage::None;
                event.progress = progress;
                event.message = message;
                impl_->jobSystem->publish(event);
            };

            ImportRequest importRequest{};
            importRequest.sourcePath = spec.sourcePath;
            importRequest.runRoot = spec.runRoot;
            importRequest.runIdPrefix = spec.runIdPrefix;
            importRequest.jobId = id;
            importRequest.progressCallback = std::move(progressCallback);
            importRequest.cancelFlag = cancel;
            importRequest.pauseFlag = pause;
            importRequest.scheduler = &impl_->jobSystem->scheduler();
            importRequest.ocrLimiter = &impl_->jobSystem->slotLimiter();

            auto result = impl_->importService->importStatement(importRequest);
            if (!result.data) {
                impl_->jobSystem->fail(id, std::string(constants::kErrorExtractionFailed));
                return;
            }

            impl_->setResult(id, std::move(result));

            if (cancel && cancel->load()) {
                impl_->jobSystem->cancel(id);
                return;
            }

            impl_->jobSystem->finish(id);
        } catch (const std::exception& ex) {
            impl_->jobSystem->fail(id, ex.what());
        } catch (...) {
            impl_->jobSystem->fail(id, std::string(kUnknownError));
        }
    });

    return id;
}

core::jobs::SubscriptionId StatementImportJobSystem::subscribe(
    const core::jobs::JobId& id,
    core::jobs::JobEventCallback cb)
{
    return impl_->jobSystem ? impl_->jobSystem->subscribe(id, std::move(cb)) : 0;
}

void StatementImportJobSystem::unsubscribe(const core::jobs::JobId& id, core::jobs::SubscriptionId subId)
{
    if (impl_->jobSystem) impl_->jobSystem->unsubscribe(id, subId);
}

void StatementImportJobSystem::cancel(const core::jobs::JobId& id)
{
    if (impl_->jobSystem) impl_->jobSystem->cancel(id);
}

void StatementImportJobSystem::pause(const core::jobs::JobId& id)
{
    if (impl_->jobSystem) impl_->jobSystem->pause(id);
}

void StatementImportJobSystem::resume(const core::jobs::JobId& id)
{
    if (impl_->jobSystem) impl_->jobSystem->resume(id);
}

std::optional<core::jobs::JobSnapshot> StatementImportJobSystem::snapshot(const core::jobs::JobId& id) const
{
    return impl_->jobSystem ? impl_->jobSystem->snapshot(id) : std::nullopt;
}

std::shared_ptr<core::domain::Statement> StatementImportJobSystem::statementResult(const core::jobs::JobId& id) const
{
    std::lock_guard<std::mutex> lock(impl_->resultsMutex);
    const auto it = impl_->results.find(id);
    return it == impl_->results.end() ? nullptr : it->second.statement;
}

std::vector<draft::TransactionDraft> StatementImportJobSystem::statementTransactions(const core::jobs::JobId& id) const
{
    std::lock_guard<std::mutex> lock(impl_->resultsMutex);
    const auto it = impl_->results.find(id);
    return it == impl_->results.end() ? std::vector<draft::TransactionDraft>{} : it->second.transactions;
}

std::map<std::string, std::vector<std::uint8_t>>
StatementImportJobSystem::takeStatementArtifacts(const core::jobs::JobId& id)
{
    std::lock_guard<std::mutex> lock(impl_->resultsMutex);
    const auto it = impl_->results.find(id);
    if (it == impl_->results.end()) return {};
    auto out = std::move(it->second.artifacts);
    it->second.artifacts.clear();
    return out;
}

void StatementImportJobSystem::shutdown()
{
    if (impl_->jobSystem) impl_->jobSystem->shutdown();
}

} // namespace core::application::importing
