/**
 * @file core/include/core/application/workspace/WorkspaceSessionState.h
 * @brief Internal mutable workspace session state for application services.
 */

#pragma once

#include <memory>
#include <vector>

#include "core/application/export/ExportLog.h"
#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/domain/catalog/WorkspaceCatalog.h"

namespace core::application::workspace {

struct WorkspaceWorkflowState {
    template <typename T>
    using EntityList = std::vector<std::shared_ptr<T>>;

    using StatementDraftList = EntityList<core::application::importing::draft::StatementDraft>;
    using TransactionDraftList = EntityList<core::application::importing::draft::TransactionDraft>;
    using ImportLogList = EntityList<core::application::importing::ImportLog>;
    using ExportLogList = EntityList<core::application::exporting::ExportLog>;

    StatementDraftList statementDrafts;
    TransactionDraftList transactionDrafts;
    ImportLogList importLogs;
    ExportLogList exportLogs;

    /** @brief Checks whether all workflow collections are empty. */
    [[nodiscard]] bool empty() const noexcept {
        return statementDrafts.empty() && transactionDrafts.empty() && importLogs.empty() && exportLogs.empty();
    }
};

struct WorkspaceSessionState {
    core::domain::catalog::WorkspaceCatalog catalog;
    WorkspaceWorkflowState workflow;

    /** @brief Checks whether catalog and workflow state are empty. */
    [[nodiscard]] bool empty() const noexcept {
        return catalog.empty() && workflow.empty();
    }
};

} // namespace core::application::workspace
