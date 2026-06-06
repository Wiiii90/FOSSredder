/**
 * @file persistence/tests/state/TestWorkspaceStateStore.cpp
 * @brief Tests for the SQLite-backed workspace state store.
 */

#include <gtest/gtest.h>

#include <memory>

#include "persistence/SqliteDb.h"
#include "persistence/WorkspaceStateStore.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(WorkspaceStateStoreTest, SavesAndLoadsTheFullWorkspaceStateRoundTrip)
{
    TempDatabase dbFile("workspace-state-store");
    auto db = std::make_shared<SqliteDb>(dbFile.string());
    WorkspaceStateStore store(db);

    const auto state = makeWorkspaceSessionState();
    const auto impact = store.save(state);
    EXPECT_TRUE(impact.empty());

    const auto loaded = store.load();
    EXPECT_EQ(loaded.catalog.actors().size(), 1);
    EXPECT_EQ(loaded.catalog.properties().size(), 1);
    EXPECT_EQ(loaded.catalog.contracts().size(), 1);
    EXPECT_EQ(loaded.catalog.statements().size(), 1);
    EXPECT_EQ(loaded.catalog.transactions().size(), 2);
    EXPECT_EQ(loaded.catalog.analyses().size(), 1);
    EXPECT_EQ(loaded.catalog.annuals().size(), 1);
    EXPECT_EQ(loaded.workflow.statementDrafts.size(), 1);
    EXPECT_EQ(loaded.workflow.transactionDrafts.size(), 2);
    EXPECT_EQ(loaded.workflow.importLogs.size(), 1);
    EXPECT_EQ(loaded.workflow.exportLogs.size(), 1);

    ASSERT_EQ(loaded.catalog.statements().front()->transactionIds(),
              std::vector<std::string>({"tx-1", "tx-2"}));
    ASSERT_EQ(loaded.catalog.actors().front()->aliases().size(), 2);
    EXPECT_EQ(loaded.catalog.actors().front()->aliases().at(0).value(), "Primary Actor");
    EXPECT_EQ(loaded.catalog.actors().front()->aliases().at(1).value(), "Main Actor");
    ASSERT_EQ(loaded.catalog.properties().front()->aliases().size(), 1);
    EXPECT_EQ(loaded.catalog.properties().front()->aliases().at(0).value(), "Property Alias");
    ASSERT_EQ(loaded.catalog.contracts().front()->aliases().size(), 1);
    EXPECT_EQ(loaded.catalog.contracts().front()->aliases().at(0).value(), "Lease");
    ASSERT_EQ(loaded.catalog.transactions().front()->name(), "Rent");
    ASSERT_EQ(loaded.catalog.transactions().front()->statementId(), "statement-1");
    ASSERT_EQ(loaded.catalog.analyses().front()->adjustments().at("actor-1"), 19.25);
    ASSERT_EQ(loaded.workflow.statementDrafts.front()->name, "Imported Statement");
    ASSERT_EQ(loaded.workflow.importLogs.front()->status, "done");
    ASSERT_EQ(loaded.workflow.exportLogs.front()->status, "done");
}

TEST(WorkspaceStateStoreTest, ReportsDeletionImpactDeterministically)
{
    TempDatabase dbFile("workspace-state-store-impact");
    auto db = std::make_shared<SqliteDb>(dbFile.string());
    WorkspaceStateStore store(db);

    const auto initial = makeWorkspaceSessionState();
    const auto initialImpact = store.save(initial);
    EXPECT_TRUE(initialImpact.empty());

    auto next = initial;
    next.catalog.setActors({});
    next.catalog.setStatements({});
    next.catalog.setTransactions({});
    next.catalog.setAnalyses({});
    next.workflow.statementDrafts.clear();
    next.workflow.transactionDrafts.clear();
    next.workflow.importLogs.clear();
    next.workflow.exportLogs.clear();

    const auto impact = store.save(next);
    EXPECT_EQ(impact.deletedActorIds, std::vector<std::string>({"actor-1"}));
    EXPECT_EQ(impact.deletedStatementIds, std::vector<std::string>({"statement-1"}));
    EXPECT_TRUE(impact.deletedTransactionIds.empty());
    EXPECT_EQ(impact.deletedAnalysisIds, std::vector<std::string>({"analysis-1"}));
    EXPECT_EQ(impact.deletedStatementDraftIds, std::vector<std::string>({"statement-draft-1"}));
    EXPECT_EQ(impact.deletedTransactionDraftIds,
              std::vector<std::string>({"draft-tx-1", "draft-tx-2"}));
    EXPECT_EQ(impact.deletedImportLogIds, std::vector<std::string>({"import-log-1"}));
    EXPECT_EQ(impact.deletedExportLogIds, std::vector<std::string>({"export-log-1"}));
}

} // namespace persistence::tests
