/**
 * @file ui/tests/unit/workspace/TestWorkspaceRowProjector.cpp
 * @brief Tests for workspace row projection helpers.
 */

#include <gtest/gtest.h>

#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceRowProjector.h"
#include "ui/workspace/WorkspaceCache.h"

namespace ui {

TEST(WorkspaceRowProjectorTest, WSP_ROW_001_ProjectsStableRowsForAllWorkspaceFamilies) {
  WorkspaceCache store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  const QVariantList actorRows = buildActorRows(store);
  const QVariantList propertyRows = buildPropertyRows(store);
  const QVariantList contractRows = buildContractRows(store);
  const QVariantList analysisRows = buildAnalysisRows(store);
  const QVariantList annualRows = buildAnnualRows(store);
  const QVariantList statementRows = buildStatementRows(store);
  const QVariantList transactionRows =
      buildStatementTransactionRows(store, QStringLiteral("statement-1"));

  ASSERT_EQ(actorRows.size(), 1);
  ASSERT_EQ(propertyRows.size(), 1);
  ASSERT_EQ(contractRows.size(), 1);
  ASSERT_EQ(analysisRows.size(), 1);
  ASSERT_EQ(annualRows.size(), 1);
  ASSERT_EQ(statementRows.size(), 1);
  ASSERT_EQ(transactionRows.size(), 2);

  EXPECT_EQ(actorRows.first().toMap().value(QStringLiteral("id")).toString(),
            QStringLiteral("actor-1"));
  EXPECT_EQ(actorRows.first()
                .toMap()
                .value(QStringLiteral("aliases"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("Primary Actor"));
  EXPECT_EQ(actorRows.first()
                .toMap()
                .value(QStringLiteral("selectedIds"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("contract-1"));
  EXPECT_EQ(
      propertyRows.first().toMap().value(QStringLiteral("name")).toString(),
      QStringLiteral("Primary Property"));
  EXPECT_EQ(propertyRows.first()
                .toMap()
                .value(QStringLiteral("aliases"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("Property Alias"));
  EXPECT_EQ(propertyRows.first()
                .toMap()
                .value(QStringLiteral("selectedIds"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("contract-1"));
  EXPECT_EQ(
      contractRows.first().toMap().value(QStringLiteral("type")).toString(),
      QStringLiteral("lease"));
  EXPECT_EQ(analysisRows.first()
                .toMap()
                .value(QStringLiteral("adjustments"))
                .toString(),
            QStringLiteral("{\"actor-1\":19.25}"));
  EXPECT_EQ(annualRows.first().toMap().value(QStringLiteral("year")).toInt(),
            2026);
  EXPECT_EQ(
      statementRows.first().toMap().value(QStringLiteral("name")).toString(),
      QStringLiteral("January Statement"));
  EXPECT_EQ(
      transactionRows.first().toMap().value(QStringLiteral("id")).toString(),
      QStringLiteral("tx-1"));
}

} // namespace ui
