/**
 * @file ui/tests/unit/TestWorkspaceSelection.cpp
 * @brief Tests for the UI WorkspaceSelection synchronization layer.
 */

#include <gtest/gtest.h>

#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceSelection.h"

namespace ui {

TEST(WorkspaceSelectionTest,
     TracksCurrentSelectionAcrossAllCollections) {
  WorkspaceCacheModels models;
  models.loadFromState(tests::support::makeWorkspaceSnapshot());

  WorkspaceSelection selection(models);

  selection.setSelectedActorId(QStringLiteral("actor-1"));
  selection.setSelectedPropertyId(QStringLiteral("property-1"));
  selection.setSelectedContractId(QStringLiteral("contract-1"));
  selection.setSelectedStatementId(QStringLiteral("statement-1"));
  selection.setSelectedTransactionId(QStringLiteral("tx-1"));
  selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));
  selection.setSelectedAnnualId(QStringLiteral("annual-1"));
  selection.setLastAnalysisResult(QVariantMap{{QStringLiteral("ok"), true}});

  EXPECT_EQ(selection.selectedActorId(), QStringLiteral("actor-1"));
  EXPECT_EQ(selection.selectedPropertyId(), QStringLiteral("property-1"));
  EXPECT_EQ(selection.selectedContractId(), QStringLiteral("contract-1"));
  EXPECT_EQ(selection.selectedStatementId(), QStringLiteral("statement-1"));
  EXPECT_EQ(selection.selectedTransactionId(), QStringLiteral("tx-1"));
  EXPECT_EQ(selection.selectedAnalysisId(), QStringLiteral("analysis-1"));
  EXPECT_EQ(selection.selectedAnnualId(), QStringLiteral("annual-1"));
  ASSERT_TRUE(selection.lastAnalysisResult().isValid());

}

TEST(WorkspaceSelectionTest,
     ClearsStaleSelectionsAndAnalysisResultAfterReload) {
  WorkspaceCacheModels models;
  models.loadFromState(tests::support::makeWorkspaceSnapshot());

  WorkspaceSelection selection(models);
  selection.setSelectedActorId(QStringLiteral("actor-1"));
  selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));
  selection.setLastAnalysisResult(QVariantMap{{QStringLiteral("ok"), true}});

  models.removeActorAt(0);
  models.analyses().removeAt(0);

  EXPECT_TRUE(selection.selectedActorId().isEmpty());
  EXPECT_TRUE(selection.selectedAnalysisId().isEmpty());
  EXPECT_FALSE(selection.lastAnalysisResult().isValid());

  selection.loadFromState();
  EXPECT_TRUE(selection.selectedActorId().isEmpty());
  EXPECT_TRUE(selection.selectedAnalysisId().isEmpty());
}

} // namespace ui
