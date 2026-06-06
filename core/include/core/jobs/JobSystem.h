/**
 * @file core/include/core/jobs/JobSystem.h
 * @brief Declares the import job orchestration facade used by the UI and app layer.
 */

#pragma once

#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"
#include "core/application/import/draft/TransactionDraft.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::application::importing {
class IImportStatement;
}

namespace core::jobs {

class JobSystem {
public:
    /**
     * @brief Create a job system over an import service.
     * @param importService Import service used for import jobs.
     * @param workers Worker count for the scheduler.
     */
    JobSystem(std::shared_ptr<core::application::importing::IImportStatement> importService, std::size_t workers = 0);

    /**
     * @brief Destroy the job system.
     */
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) noexcept;
    JobSystem& operator=(JobSystem&&) noexcept;

    /**
     * @brief Start an import-statement job.
     * @param spec Import job specification.
     * @return Job identifier.
     */
    JobId startImportStatement(const ImportStatementJobSpec& spec);

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
     * @brief Retrieve the statement result of a job.
     * @param id Job identifier.
     * @return Imported statement result.
     */
    std::shared_ptr<core::domain::Statement> statementResult(const JobId& id) const;

    /**
     * @brief Retrieve the transaction drafts of a job.
     * @param id Job identifier.
     * @return Imported transaction drafts.
     */
    std::vector<core::application::importing::draft::TransactionDraft> statementTransactions(const JobId& id) const;

    /**
     * @brief Retrieve and clear job artifacts.
     * @param id Job identifier.
     * @return Job artifacts.
     */
    std::map<std::string, std::vector<uint8_t>> takeStatementArtifacts(const JobId& id);

    /**
     * @brief Shut down the job system.
     */
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
