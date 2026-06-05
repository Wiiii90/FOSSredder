/**
 * @file ui/tests/unit/workspace/TestWorkspaceSelection.cpp
 * @brief Tests for workspace selection state.
 */

#include <gtest/gtest.h>

#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

TEST(WorkspaceSelectionTest,
     WSP_SELECTION_001_TracksCurrentSelectionAcrossAllCollections) {
  WorkspaceStore store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());
  WorkspaceSelectors selectors(store);
  WorkspaceSelection selection(store, selectors);

  selection.setSelectedActorId(QStringLiteral("actor-1"));
  selection.setSelectedPropertyId(QStringLiteral("property-1"));
  selection.setSelectedContractId(QStringLiteral("contract-1"));
  selection.setSelectedStatementId(QStringLiteral("statement-1"));
  selection.setSelectedTransactionId(QStringLiteral("tx-1"));
  selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));
  selection.setSelectedAnnualId(QStringLiteral("annual-1"));

  EXPECT_EQ(selection.selectedActorId(), QStringLiteral("actor-1"));
  EXPECT_EQ(selection.selectedPropertyId(), QStringLiteral("property-1"));
  EXPECT_EQ(selection.selectedContractId(), QStringLiteral("contract-1"));
  EXPECT_EQ(selection.selectedStatementId(), QStringLiteral("statement-1"));
  EXPECT_EQ(selection.selectedTransactionId(), QStringLiteral("tx-1"));
  EXPECT_EQ(selection.selectedAnalysisId(), QStringLiteral("analysis-1"));
  EXPECT_EQ(selection.selectedAnnualId(), QStringLiteral("annual-1"));
}

TEST(WorkspaceSelectionTest,
     WSP_SELECTION_002_ClearsStaleSelectionsAfterReload) {
  WorkspaceStore store;
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  store.loadFromState(snapshot);
  WorkspaceSelectors selectors(store);
  WorkspaceSelection selection(store, selectors);

  selection.setSelectedActorId(QStringLiteral("actor-1"));
  selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));

  snapshot.actors.clear();
  snapshot.analyses.clear();
  store.loadFromState(snapshot);

  EXPECT_TRUE(selection.selectedActorId().isEmpty());
  EXPECT_TRUE(selection.selectedAnalysisId().isEmpty());
}

} // namespace ui
