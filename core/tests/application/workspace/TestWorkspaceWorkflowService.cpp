/**
 * @file core/tests/application/workspace/TestWorkspaceWorkflowService.cpp
 * @brief Tests for workspace workflow state handling.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "core/application/workspace/WorkspaceWorkflowService.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceWorkflowServiceTest, SavesAndClearsStatementDraftState) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    WorkspaceSession session(std::move(storage));
    WorkspaceWorkflowService service(session);

    core::ports::workspace::StatementDraftCommand saveCommand;
    saveCommand.draft.id = "draft-1";
    saveCommand.draft.name = "Imported Statement";
    saveCommand.draft.transactionIds = {"tx-1"};
    saveCommand.draft.transactions.push_back({
        "tx-1",
        "draft-1",
        "Rent",
        "2024-01-31",
        {},
        10.0,
        {},
        {},
        {},
        static_cast<int>(core::domain::Transaction::Status::Neutral),
        true,
        0,
        {}
    });

    service.saveStatementDraft(saveCommand);

    EXPECT_EQ(session.state().workflow.statementDrafts.size(), 1u);
    EXPECT_EQ(session.state().workflow.transactionDrafts.size(), 1u);
    ASSERT_FALSE(session.state().workflow.statementDrafts.empty());
    EXPECT_EQ(session.state().workflow.statementDrafts.front()->id, "draft-1");

    service.clearStatementDraft("draft-1");

    EXPECT_TRUE(session.state().workflow.statementDrafts.empty());
    EXPECT_TRUE(session.state().workflow.transactionDrafts.empty());
}

TEST(WorkspaceWorkflowServiceTest, SavingStatementDraftUpdatesSessionWithoutDiskCommit) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");
    WorkspaceWorkflowService service(session);

    core::ports::workspace::StatementDraftCommand saveCommand;
    saveCommand.draft.id = "draft-1";
    saveCommand.draft.name = "Imported Statement";
    saveCommand.draft.transactions.push_back({
        "tx-1",
        "draft-1",
        "Rent",
        "2024-01-31",
        {},
        10.0,
        {},
        {},
        {},
        static_cast<int>(core::domain::Transaction::Status::Neutral),
        true,
        0,
        {}
    });

    service.saveStatementDraft(saveCommand);

    EXPECT_EQ(session.state().workflow.statementDrafts.size(), 1u);
    EXPECT_TRUE(storagePtr->savedState_.workflow.statementDrafts.empty());
}

TEST(WorkspaceWorkflowServiceTest, ReplacesImportAndExportLogsAtomically) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Main Actor");
    session.mutableCatalogState().setActors({actor});

    auto property = std::make_shared<core::domain::Property>();
    property->setId("property-1");
    property->rename("Main Property");
    session.mutableCatalogState().setProperties({property});

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Main Contract");
    contract->setType("lease");
    contract->setActorIds({"actor-1"});
    contract->setPropertyIds({"property-1"});
    session.mutableCatalogState().setContracts({contract});

    WorkspaceWorkflowService service(session);

    core::ports::workspace::ImportLogsCommand importLogs;
    importLogs.logs.push_back({"import-1", "2024-01-01T00:00:00Z", "csv", "file.csv", "done", "ok", false, {}, {}, {}});
    service.setImportLogs(importLogs);

    core::ports::workspace::ExportLogsCommand exportLogs;
    exportLogs.logs.push_back({"export-1", "2024-01-02T00:00:00Z", "target", "done", "ok", "payload", {}, {}});
    service.setExportLogs(exportLogs);

    EXPECT_EQ(session.state().workflow.importLogs.size(), 1u);
    EXPECT_EQ(session.state().workflow.exportLogs.size(), 1u);
    ASSERT_FALSE(session.state().workflow.importLogs.empty());
    ASSERT_FALSE(session.state().workflow.exportLogs.empty());
    EXPECT_EQ(session.state().workflow.importLogs.front()->id, "import-1");
    EXPECT_EQ(session.state().workflow.exportLogs.front()->id, "export-1");
    EXPECT_TRUE(storagePtr->savedState_.workflow.importLogs.empty());
    EXPECT_TRUE(storagePtr->savedState_.workflow.exportLogs.empty());

    session.saveFile();

    EXPECT_EQ(storagePtr->savedState_.workflow.importLogs.size(), 1u);
    EXPECT_EQ(storagePtr->savedState_.workflow.exportLogs.size(), 1u);
}

TEST(WorkspaceWorkflowServiceTest, FinalizesImportedDraftIntoCatalogInMemoryUntilSaveFile) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");
    WorkspaceWorkflowService service(session);

    core::ports::workspace::StatementDraftCommand command;
    command.draft.id = "statement-draft-1";
    command.draft.name = "Imported Statement";
    command.draft.transactions.push_back({
        "draft-tx-1",
        "statement-draft-1",
        "Rent",
        "2026-01-05",
        "EUR",
        1250.0,
        "actor-1",
        "contract-1",
        {"property-1"},
        static_cast<int>(core::domain::Transaction::Status::Verified),
        true,
        0,
        {}
    });

    core::ports::workspace::FinalizeStatementDraftCommand finalizeCommand;
    finalizeCommand.draft = command.draft;
    const auto statementId = service.finalizeStatementDraft(finalizeCommand);

    EXPECT_FALSE(statementId.empty());
    ASSERT_FALSE(session.state().catalog.statements().empty());
    ASSERT_FALSE(session.state().catalog.transactions().empty());
    EXPECT_EQ(session.state().catalog.statements().front()->transactionIds().front(),
              session.state().catalog.transactions().front()->id());
    EXPECT_EQ(session.state().catalog.transactions().front()->statementId(),
              session.state().catalog.statements().front()->id());
    EXPECT_TRUE(storagePtr->savedState_.catalog.statements().empty());
    EXPECT_TRUE(storagePtr->savedState_.catalog.transactions().empty());

    session.saveFile();

    EXPECT_EQ(storagePtr->savedState_.catalog.statements().size(), 1u);
    EXPECT_EQ(storagePtr->savedState_.catalog.transactions().size(), 1u);
}

} // namespace core::application
