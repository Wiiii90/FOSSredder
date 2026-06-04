/**
 * @file ui/tests/unit/workspace/TestWorkspaceCache.cpp
 * @brief Tests for the UI WorkspaceCache boundary and derived filters.
 */

#include <gtest/gtest.h>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceCache.h"
#include "ui/workspace/TransactionFilterModel.h"
#include "ui/workspace/TransactionListModel.h"

namespace ui {

TEST(WorkspaceCacheTest,
     WSP_CACHE_001_ExposesFilteredTransactionViewsForWorkspaceCollections) {
  WorkspaceCache store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  TransactionFilter *statementFilter =
      store.statementTransactions(QStringLiteral("statement-1"));
  ASSERT_NE(statementFilter, nullptr);
  EXPECT_EQ(statementFilter->rowCount(), 2);

  TransactionFilter *propertyFilter =
      store.propertyTransactions(QStringLiteral("property-1"));
  ASSERT_NE(propertyFilter, nullptr);
  EXPECT_EQ(propertyFilter->rowCount(), 2);

  propertyFilter->setTxType(QStringLiteral("lease"));
  EXPECT_EQ(propertyFilter->rowCount(), 1);
}

TEST(WorkspaceCacheTest, WSP_CACHE_002_AppliesDeletionImpactAcrossModelsAndFilters) {
  WorkspaceCache store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  TransactionFilter *statementFilter =
      store.statementTransactions(QStringLiteral("statement-1"));
  TransactionFilter *propertyFilter =
      store.propertyTransactions(QStringLiteral("property-1"));
  ASSERT_NE(statementFilter, nullptr);
  ASSERT_NE(propertyFilter, nullptr);
  EXPECT_EQ(statementFilter->rowCount(), 2);
  EXPECT_EQ(propertyFilter->rowCount(), 2);

  core::ports::workspace::DeletionImpact impact;
  impact.deletedStatementIds = {std::string("statement-1")};
  impact.deletedTransactionIds = {std::string("tx-1"), std::string("tx-2")};
  impact.deletedPropertyIds = {std::string("property-1")};

  store.applyDeletionImpact(impact);

  EXPECT_EQ(store.models().statements().rowCount(), 0);
  EXPECT_EQ(store.models().transactions().rowCount(), 0);
  EXPECT_EQ(store.models().propertyModel().rowCount(), 0);

  TransactionFilter *refreshedStatementFilter =
      store.statementTransactions(QStringLiteral("statement-1"));
  TransactionFilter *refreshedPropertyFilter =
      store.propertyTransactions(QStringLiteral("property-1"));
  ASSERT_NE(refreshedStatementFilter, nullptr);
  ASSERT_NE(refreshedPropertyFilter, nullptr);
  EXPECT_EQ(refreshedStatementFilter->rowCount(), 0);
  EXPECT_EQ(refreshedPropertyFilter->rowCount(), 0);
}

TEST(WorkspaceCacheTest, WSP_CACHE_003_UpdatesTransactionPropertyAssignmentsInPlace) {
  WorkspaceCache store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  TransactionFilter *propertyFilter =
      store.propertyTransactions(QStringLiteral("property-1"));
  ASSERT_NE(propertyFilter, nullptr);
  EXPECT_EQ(propertyFilter->rowCount(), 2);

  store.setTransactionPropertyIdsImmediate(
      QStringLiteral("tx-2"), QStringList{QStringLiteral("property-2")});

  const QModelIndex rowIndex = store.models().transactions().index(1, 0);
  ASSERT_TRUE(rowIndex.isValid());
  EXPECT_EQ(store.models()
                .transactions()
                .data(rowIndex, TransactionListModel::PropertyIdsRole)
                .toList(),
            QVariantList({QStringLiteral("property-2")}));

  EXPECT_EQ(propertyFilter->rowCount(), 1);
}

TEST(WorkspaceCacheTest,
     WSP_CACHE_004_ResolvesAmountForCommitWithParserAndPersistedFallback) {
  WorkspaceCache store;
  store.loadFromState(tests::support::makeWorkspaceSnapshot());

  EXPECT_DOUBLE_EQ(store.amountForTransactionCommit(QStringLiteral("12,75"),
                                                    QStringLiteral(""), 0.0),
                   12.75);
  EXPECT_DOUBLE_EQ(
      store.amountForTransactionCommit(QStringLiteral("not-a-number"),
                                       QStringLiteral("tx-1"), 0.0),
      1250.0);
  EXPECT_DOUBLE_EQ(store.amountForTransactionCommit(
                       QVariant(), QStringLiteral("missing"), 7.5),
                   7.5);
}

} // namespace ui
