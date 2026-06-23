/**
 * @file core/tests/application/workspace/TestWorkspaceSnapshotProjector.cpp
 * @brief Tests for snapshot projection from workspace session state.
 */

#include <gtest/gtest.h>

#include "core/application/workspace/WorkspaceSnapshotProjector.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Alias.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

namespace {

core::domain::Alias MakeAlias(const std::string& value)
{
    return core::domain::Alias{value, "kind", value, "created", "updated", 3, "last"};
}

template <typename T>
std::shared_ptr<T> MakeEntity()
{
    return std::make_shared<T>();
}

} // namespace

TEST(WorkspaceSnapshotProjectorTest, ProjectsCatalogAndWorkflowState) {
    core::application::workspace::WorkspaceSessionState state;

    auto actor = MakeEntity<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alpha");
    actor->setAliases({MakeAlias("alpha")});
    state.catalog.setActors({actor});

    auto property = MakeEntity<core::domain::Property>();
    property->setId("property-1");
    property->rename("Property");
    state.catalog.setProperties({property});

    auto contract = MakeEntity<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Contract");
    contract->setType("rent");
    contract->setActorIds({"actor-1"});
    contract->setPropertyIds({"property-1"});
    state.catalog.setContracts({contract});

    auto statement = MakeEntity<core::domain::Statement>();
    statement->setId("statement-1");
    statement->rename("Statement");
    statement->setTransactionIds({"tx-1"});
    state.catalog.setStatements({statement});

    auto transaction = MakeEntity<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setName("Rent");
    transaction->setBookingDate("2024-01-31");
    transaction->setAmount(10.0);
    transaction->setStatementId("statement-1");
    transaction->setContractId("contract-1");
    transaction->setActorId("actor-1");
    transaction->setPropertyIds({"property-1"});
    state.catalog.setTransactions({transaction});

    auto analysis = MakeEntity<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Analysis");
    analysis->setType("tab");
    analysis->setExportFormat("xlsx");
    state.catalog.setAnalyses({analysis});

    auto annual = MakeEntity<core::domain::Annual>();
    annual->setId("annual-1");
    annual->rename("Annual");
    annual->setYear(2024);
    annual->setAnalysisIds({"analysis-1"});
    state.catalog.setAnnuals({annual});

    auto statementDraft = std::make_shared<core::application::importing::draft::StatementDraft>();
    statementDraft->id = "draft-1";
    statementDraft->name = "Draft";
    statementDraft->transactionIds = {"draft-tx-1"};
    statementDraft->transactions.push_back(core::application::importing::draft::TransactionDraft{});
    statementDraft->transactions.back().id = "draft-tx-1";
    statementDraft->transactions.back().statementDraftId = "draft-1";
    statementDraft->transactions.back().name = "Draft Row";
    statementDraft->transactions.back().bookingDate = "2024-01-31";
    statementDraft->transactions.back().amount = 10.0;
    state.workflow.statementDrafts.push_back(statementDraft);

    auto importLog = std::make_shared<core::application::importing::ImportLog>();
    importLog->id = "import-1";
    importLog->status = "done";
    state.workflow.importLogs.push_back(importLog);

    auto exportLog = std::make_shared<core::application::exporting::ExportLog>();
    exportLog->id = "export-1";
    exportLog->status = "done";
    state.workflow.exportLogs.push_back(exportLog);

    WorkspaceSnapshotProjector projector;
    const auto snapshot = projector.project(state, "P:/workspace.db");

    EXPECT_TRUE(snapshot.hasCurrentPath);
    EXPECT_EQ(snapshot.currentPath, "P:/workspace.db");
    EXPECT_EQ(snapshot.actors.size(), 1u);
    EXPECT_EQ(snapshot.properties.size(), 1u);
    EXPECT_EQ(snapshot.contracts.size(), 1u);
    EXPECT_EQ(snapshot.statements.size(), 1u);
    EXPECT_EQ(snapshot.transactions.size(), 1u);
    EXPECT_EQ(snapshot.analyses.size(), 1u);
    EXPECT_EQ(snapshot.annuals.size(), 1u);
    EXPECT_EQ(snapshot.statementDrafts.size(), 1u);
    EXPECT_EQ(snapshot.importLogs.size(), 1u);
    EXPECT_EQ(snapshot.exportLogs.size(), 1u);
}

TEST(WorkspaceSnapshotProjectorTest, ResolvesDraftSnapshotByIdAndFallback) {
    core::application::workspace::WorkspaceSessionState state;
    auto draft = std::make_shared<core::application::importing::draft::StatementDraft>();
    draft->id = "draft-1";
    draft->name = "Draft";
    state.workflow.statementDrafts.push_back(draft);

    WorkspaceSnapshotProjector projector;

    const auto firstDraft = projector.projectStatementDraft(state);
    ASSERT_TRUE(firstDraft.has_value());
    EXPECT_EQ(firstDraft->id, "draft-1");

    const auto missingDraft = projector.projectStatementDraft(state, "missing");
    EXPECT_FALSE(missingDraft.has_value());
}

} // namespace core::application
