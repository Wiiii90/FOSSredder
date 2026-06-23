/**
 * @file core/include/core/application/import/StatementImportJobSystem.h
 * @brief Declares asynchronous statement import job orchestration.
 */

#pragma once

#include "core/application/import/draft/TransactionDraft.h"
#include "core/jobs/JobTypes.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace core::jobs {
class JobSystem;
}

namespace core::application::importing {

class IImportStatement;

struct ImportStatementJobSpec {
    std::string sourcePath;
    std::string runRoot;
    std::string runIdPrefix;
};

class StatementImportJobSystem {
public:
    StatementImportJobSystem(
        std::shared_ptr<IImportStatement> importService,
        std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
        std::size_t workers = 0);
    StatementImportJobSystem(
        std::shared_ptr<IImportStatement> importService,
        std::shared_ptr<core::jobs::JobSystem> jobSystem);
    ~StatementImportJobSystem();

    StatementImportJobSystem(const StatementImportJobSystem&) = delete;
    StatementImportJobSystem& operator=(const StatementImportJobSystem&) = delete;
    StatementImportJobSystem(StatementImportJobSystem&&) noexcept;
    StatementImportJobSystem& operator=(StatementImportJobSystem&&) noexcept;

    [[nodiscard]] core::jobs::JobId startImportStatement(const ImportStatementJobSpec& spec);

    [[nodiscard]] core::jobs::SubscriptionId subscribe(
        const core::jobs::JobId& id,
        core::jobs::JobEventCallback cb);
    void unsubscribe(const core::jobs::JobId& id, core::jobs::SubscriptionId subId);

    void cancel(const core::jobs::JobId& id);
    void pause(const core::jobs::JobId& id);
    void resume(const core::jobs::JobId& id);

    [[nodiscard]] std::optional<core::jobs::JobSnapshot> snapshot(const core::jobs::JobId& id) const;
    [[nodiscard]] std::shared_ptr<core::domain::Statement> statementResult(const core::jobs::JobId& id) const;
    [[nodiscard]] std::vector<draft::TransactionDraft> statementTransactions(const core::jobs::JobId& id) const;
    [[nodiscard]] std::map<std::string, std::vector<std::uint8_t>> takeStatementArtifacts(const core::jobs::JobId& id);

    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core::application::importing
