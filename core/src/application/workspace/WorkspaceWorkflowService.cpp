/**
 * @file core/src/application/workspace/WorkspaceWorkflowService.cpp
 * @brief Implements workspace workflow command orchestration using typed boundary commands.
 */

#include "core/application/workspace/WorkspaceWorkflowService.h"

#include "../../utils/StableId.h"
#include "core/application/import/draft/DraftFinalizer.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/workspace/WorkspaceSession.h"

#include <algorithm>

namespace {

core::application::importing::draft::TransactionDraft toDraft(const core::ports::workspace::TransactionDraftSnapshot& snapshot) {
    core::application::importing::draft::TransactionDraft draft;
    draft.id = snapshot.id;
    draft.statementDraftId = snapshot.statementDraftId;
    draft.name = snapshot.name;
    draft.bookingDate = snapshot.bookingDate;
    draft.valuta = snapshot.valuta;
    draft.amount = snapshot.amount;
    draft.actorId = snapshot.actorId;
    draft.contractId = snapshot.contractId;
    draft.propertyIds = snapshot.propertyIds;
    draft.status = static_cast<core::domain::Transaction::Status>(snapshot.status);
    draft.allocatable = snapshot.allocatable;
    draft.position = snapshot.position;
    draft.metadata = snapshot.metadata;
    draft.proofImageData = snapshot.proofImageData;
    return draft;
}

core::application::importing::draft::StatementDraft toDraft(const core::ports::workspace::StatementDraftSnapshot& snapshot) {
    core::application::importing::draft::StatementDraft draft;
    draft.id = snapshot.id;
    draft.name = snapshot.name;
    draft.transactionIds = snapshot.transactionIds;
    draft.createdAt = snapshot.createdAt;
    draft.updatedAt = snapshot.updatedAt;
    draft.transactions.reserve(snapshot.transactions.size());
    for (const auto& tx : snapshot.transactions) {
        draft.transactions.push_back(toDraft(tx));
    }
    return draft;
}

core::application::importing::ImportLog toImportLog(const core::ports::workspace::ImportLogSnapshot& item) {
    core::application::importing::ImportLog log;
    log.id = item.id;
    log.time = item.time;
    log.type = item.type;
    log.file = item.file;
    log.status = item.status;
    log.message = item.message;
    log.draftAttached = item.draftAttached;
    log.draftId = item.draftId;
    log.statementDraftIds = item.statementDraftIds;
    log.statementId = item.statementId;
    return log;
}

core::application::exporting::ExportLog toExportLog(const core::ports::workspace::ExportLogSnapshot& item) {
    core::application::exporting::ExportLog log;
    log.id = item.id;
    log.time = item.time;
    log.targetPath = item.targetPath;
    log.status = item.status;
    log.message = item.message;
    log.annualIds = item.annualIds;
    log.analysisIds = item.analysisIds;
    return log;
}

void saveStatementDraft(core::application::workspace::WorkspaceSessionState& document,
                        const core::application::importing::draft::StatementDraft& draft) {
    auto statementDraft = std::make_shared<core::application::importing::draft::StatementDraft>(draft);
    if (!statementDraft) {
        return;
    }

    if (statementDraft->id.empty()) {
        statementDraft->id = core::utils::makeStableId();
    }

    auto statementIt = std::find_if(document.workflow.statementDrafts.begin(), document.workflow.statementDrafts.end(),
                                    [statementDraft](const auto& item) {
                                        return item && item->id == statementDraft->id;
                                    });
    if (statementIt == document.workflow.statementDrafts.end()) {
        document.workflow.statementDrafts.push_back(statementDraft);
    } else {
        *statementIt = statementDraft;
    }

    document.workflow.transactionDrafts.erase(
        std::remove_if(document.workflow.transactionDrafts.begin(), document.workflow.transactionDrafts.end(),
                       [statementDraft](const auto& tx) {
                           return tx && tx->statementDraftId == statementDraft->id;
                       }),
        document.workflow.transactionDrafts.end());

    for (std::size_t i = 0; i < statementDraft->transactions.size(); ++i) {
        auto tx = std::make_shared<core::application::importing::draft::TransactionDraft>(statementDraft->transactions[i]);
        if (!tx) {
            continue;
        }
        if (tx->id.empty()) {
            tx->id = core::utils::makeStableId();
        }
        tx->statementDraftId = statementDraft->id;
        tx->position = static_cast<int>(i);
        document.workflow.transactionDrafts.push_back(std::move(tx));
    }
}

void clearStatementDraft(core::application::workspace::WorkspaceSessionState& document, const std::string& draftId) {
    if (draftId.empty()) {
        document.workflow.statementDrafts.clear();
        document.workflow.transactionDrafts.clear();
        return;
    }

    document.workflow.statementDrafts.erase(
        std::remove_if(document.workflow.statementDrafts.begin(), document.workflow.statementDrafts.end(),
                       [&draftId](const auto& draft) {
                           return draft && draft->id == draftId;
                       }),
        document.workflow.statementDrafts.end());

    document.workflow.transactionDrafts.erase(
        std::remove_if(document.workflow.transactionDrafts.begin(), document.workflow.transactionDrafts.end(),
                       [&draftId](const auto& tx) {
                           return tx && tx->statementDraftId == draftId;
                       }),
        document.workflow.transactionDrafts.end());
}

void saveImportLog(core::application::workspace::WorkspaceSessionState& document,
                   core::application::importing::ImportLog log) {
    if (log.id.empty()) {
        log.id = core::utils::makeStableId();
    }
    auto next = std::make_shared<core::application::importing::ImportLog>(std::move(log));
    auto it = std::find_if(document.workflow.importLogs.begin(), document.workflow.importLogs.end(),
                           [&next](const auto& item) {
                               return item && next && item->id == next->id;
                           });
    if (it == document.workflow.importLogs.end()) {
        document.workflow.importLogs.push_back(std::move(next));
    } else {
        *it = std::move(next);
    }
}

void deleteImportLog(core::application::workspace::WorkspaceSessionState& document,
                     const std::string& id) {
    document.workflow.importLogs.erase(
        std::remove_if(document.workflow.importLogs.begin(), document.workflow.importLogs.end(),
                       [&id](const auto& log) {
                           return log && log->id == id;
                       }),
        document.workflow.importLogs.end());
}

void saveExportLog(core::application::workspace::WorkspaceSessionState& document,
                   core::application::exporting::ExportLog log) {
    if (log.id.empty()) {
        log.id = core::utils::makeStableId();
    }
    auto next = std::make_shared<core::application::exporting::ExportLog>(std::move(log));
    auto it = std::find_if(document.workflow.exportLogs.begin(), document.workflow.exportLogs.end(),
                           [&next](const auto& item) {
                               return item && next && item->id == next->id;
                           });
    if (it == document.workflow.exportLogs.end()) {
        document.workflow.exportLogs.push_back(std::move(next));
    } else {
        *it = std::move(next);
    }
}

void deleteExportLog(core::application::workspace::WorkspaceSessionState& document,
                     const std::string& id) {
    document.workflow.exportLogs.erase(
        std::remove_if(document.workflow.exportLogs.begin(), document.workflow.exportLogs.end(),
                       [&id](const auto& log) {
                           return log && log->id == id;
                       }),
        document.workflow.exportLogs.end());
}

} // namespace

namespace core::application {

WorkspaceWorkflowService::WorkspaceWorkflowService(WorkspaceSession& session)
    : session_(&session) {
}

WorkspaceWorkflowService::~WorkspaceWorkflowService() = default;

WorkspaceWorkflowService::WorkspaceWorkflowService(WorkspaceWorkflowService&&) noexcept = default;

WorkspaceWorkflowService& WorkspaceWorkflowService::operator=(WorkspaceWorkflowService&&) noexcept = default;

core::application::workspace::WorkspaceSessionState& WorkspaceWorkflowService::mutableDocument() noexcept {
    return session_->mutableState();
}

void WorkspaceWorkflowService::commit() {
    session_->commit();
}

std::string WorkspaceWorkflowService::finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand& command) {
    const auto id = core::application::importing::draft::DraftFinalizer::finalize(mutableDocument().catalog, toDraft(command.draft));
    if (!id.empty()) {
        session_->notifyState();
    }
    return id;
}

void WorkspaceWorkflowService::saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command) {
    ::saveStatementDraft(mutableDocument(), toDraft(command.draft));
    session_->notifyState();
}

void WorkspaceWorkflowService::clearStatementDraft(const std::string& draftId) {
    ::clearStatementDraft(mutableDocument(), draftId);
    session_->notifyState();
}

void WorkspaceWorkflowService::saveImportLog(const core::ports::workspace::ImportLogCommand& command) {
    ::saveImportLog(mutableDocument(), toImportLog(command.log));
    session_->notifyState();
}

void WorkspaceWorkflowService::deleteImportLog(const std::string& id) {
    ::deleteImportLog(mutableDocument(), id);
    session_->notifyState();
}

void WorkspaceWorkflowService::saveExportLog(const core::ports::workspace::ExportLogCommand& command) {
    ::saveExportLog(mutableDocument(), toExportLog(command.log));
    session_->notifyState();
}

void WorkspaceWorkflowService::deleteExportLog(const std::string& id) {
    ::deleteExportLog(mutableDocument(), id);
    session_->notifyState();
}

} // namespace core::application
