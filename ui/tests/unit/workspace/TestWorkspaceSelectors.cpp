/**
 * @file ui/tests/unit/workspace/TestWorkspaceSelectors.cpp
 * @brief Tests for QML-facing workspace selector projections.
 */

#include <gtest/gtest.h>

#include <QVariantMap>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

TEST(WorkspaceSelectorsTest,
     WSP_SELECTORS_001_ProjectsCatalogRowsAndLookupHelpers) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  store.loadFromState(workspace.workspaceSnapshot());
  WorkspaceSelectors selectors(store);

  ASSERT_EQ(selectors.actorRows().size(), 1);
  EXPECT_EQ(selectors.actorRows()
                .front()
                .toMap()
                .value(payload::keys::common::kName)
                .toString(),
            QStringLiteral("Main Actor"));
  EXPECT_EQ(selectors.actorIdByName(QStringLiteral("Main Actor")),
            QStringLiteral("actor-1"));
  EXPECT_TRUE(selectors.hasPropertyId(QStringLiteral("property-1")));
  EXPECT_FALSE(selectors.hasContractId(QStringLiteral("missing-contract")));
}

TEST(WorkspaceSelectorsTest,
     WSP_SELECTORS_002_ProjectsStatementTransactionsAndTransactionRows) {
  WorkspaceStore store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());
  WorkspaceSelectors selectors(store);

  const QVariantList rows =
      selectors.statementTransactionRows(QStringLiteral("statement-1"));
  ASSERT_EQ(rows.size(), 2);
  EXPECT_EQ(rows.front().toMap().value(payload::keys::common::kId).toString(),
            QStringLiteral("tx-1"));

  const QVariantMap transaction =
      selectors.transactionRowById(QStringLiteral("tx-1"));
  EXPECT_EQ(
      transaction.value(payload::keys::transaction::kContractType).toString(),
      QStringLiteral("lease"));
}

TEST(WorkspaceSelectorsTest,
     WSP_SELECTORS_003_DelegatesCatalogSelectionRulesToWorkspaceReader) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceSelectors selectors(store);

  QVariantMap transaction;
  transaction.insert(payload::keys::transaction::kContractId,
                     QStringLiteral("contract-1"));
  transaction.insert(payload::keys::transaction::kActorId,
                     QStringLiteral("actor-1"));
  transaction.insert(payload::keys::transaction::kPropertyIds,
                     QVariantList{QStringLiteral("property-1")});

  QVariantMap changes;
  changes.insert(payload::keys::transaction::kPropertyIds,
                 QVariantList{QStringLiteral("missing-property")});

  const QVariantMap next =
      selectors.transactionFormWithCatalogSelection(transaction, changes);

  EXPECT_TRUE(
      next.value(payload::keys::transaction::kContractId).toString().isEmpty());
  EXPECT_EQ(
      next.value(payload::keys::transaction::kPropertyIds).toList().size(), 1);
}

} // namespace ui
