/**
 * @file ui/tests/unit/workspace/TestWorkspaceStore.cpp
 * @brief Tests for the UI workspace store role.
 */

#include <gtest/gtest.h>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

TEST(WorkspaceStoreTest, WSP_STORE_001_LoadsSnapshotsAndTracksDataRevisions) {
  WorkspaceStore store;
  const int initialRevision = store.dataRevision();

  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  EXPECT_EQ(store.dataRevision(), initialRevision + 1);
  EXPECT_EQ(store.snapshot().actors.size(), 1U);
  EXPECT_EQ(store.snapshot().transactions.size(), 2U);
}

TEST(WorkspaceStoreTest,
     WSP_STORE_002_AppliesDeletionImpactAcrossRelatedCollections) {
  WorkspaceStore store;
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  ASSERT_EQ(snapshot.statements.front().transactionIds.size(), 2U);
  store.loadFromState(snapshot);

  core::ports::workspace::DeletionImpact impact;
  impact.deletedTransactionIds = {"tx-1"};
  impact.deletedAnalysisIds = {"analysis-1"};
  impact.deletedAnnualIds = {"annual-1"};

  store.applyDeletionImpact(impact);

  EXPECT_FALSE(store.snapshot().transactions.empty());
  EXPECT_EQ(store.snapshot().transactions.front().id, std::string("tx-2"));
  ASSERT_EQ(store.snapshot().statements.size(), 1U);
  EXPECT_EQ(store.snapshot().statements.front().transactionIds.size(), 1U);
  EXPECT_EQ(store.snapshot().statements.front().transactionIds.front(),
            std::string("tx-2"));
  EXPECT_TRUE(store.snapshot().analyses.empty());
  EXPECT_TRUE(store.snapshot().annuals.empty());
}

TEST(WorkspaceStoreTest, WSP_STORE_003_CurrentPathUsesBoundReader) {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.hasCurrentPath = true;
  snapshot.currentPath = "P:/workspace/demo.foss";
  tests::support::InMemoryWorkspace workspace(snapshot);

  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);

  EXPECT_EQ(store.currentPath(), QStringLiteral("P:/workspace/demo.foss"));
}

} // namespace ui
