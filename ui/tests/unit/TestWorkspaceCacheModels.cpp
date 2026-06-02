/**
 * @file ui/tests/unit/TestWorkspaceCacheModels.cpp
 * @brief Tests for the UI WorkspaceCacheModels projection layer.
 */

#include <gtest/gtest.h>

#include <QVariant>

#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceCacheModels.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/workspace/StatementViewModel.h"
#include "ui/workspace/TransactionViewModel.h"
#include "ui/workspace/AnalysisListModel.h"
#include "ui/workspace/AnnualListModel.h"

namespace ui {

TEST(WorkspaceCacheModelsTest,
     ProjectsAllCollectionsAndRolesFromTheWorkspaceCatalog) {
  const auto snapshot = tests::support::makeWorkspaceSnapshot();

  WorkspaceCacheModels models;
  models.loadFromState(snapshot);

  EXPECT_EQ(models.actorModel().rowCount(), 1);
  EXPECT_EQ(models.propertyModel().rowCount(), 1);
  EXPECT_EQ(models.contractModel().rowCount(), 1);
  EXPECT_EQ(models.statements().rowCount(), 1);
  EXPECT_EQ(models.transactions().rowCount(), 2);
  EXPECT_EQ(models.analyses().rowCount(), 1);
  EXPECT_EQ(models.annuals().rowCount(), 1);

  const QModelIndex actorIndex = models.actorModel().index(0, 0);
  EXPECT_EQ(models.actorModel()
                .data(actorIndex,
                      models.actorModel().roleNames().key("id"))
                .toString(),
            QStringLiteral("actor-1"));
  EXPECT_EQ(models.actorModel()
                .data(actorIndex,
                      models.actorModel().roleNames().key("name"))
                .toString(),
            QStringLiteral("Main Actor"));

  const QModelIndex propertyIndex = models.propertyModel().index(0, 0);
  EXPECT_EQ(models.propertyModel()
                .data(propertyIndex,
                      models.propertyModel().roleNames().key("id"))
                .toString(),
            QStringLiteral("property-1"));
  EXPECT_EQ(models.propertyModel()
                .data(propertyIndex,
                      models.propertyModel().roleNames().key("name"))
                .toString(),
            QStringLiteral("Primary Property"));

  const QModelIndex contractIndex = models.contractModel().index(0, 0);
  EXPECT_EQ(models.contractModel()
                .data(contractIndex,
                      models.contractModel().roleNames().key("id"))
                .toString(),
            QStringLiteral("contract-1"));
  EXPECT_EQ(models.contractModel()
                .data(contractIndex,
                      models.contractModel().roleNames().key("name"))
                .toString(),
            QStringLiteral("Lease Contract"));
  EXPECT_EQ(models.contractModel()
                .data(contractIndex,
                      models.contractModel().roleNames().key("type"))
                .toString(),
            QStringLiteral("lease"));
  EXPECT_EQ(models.contractModel()
                .data(contractIndex,
                      models.contractModel().roleNames().key("actorIds"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("actor-1"));
  EXPECT_EQ(models.contractModel()
                .data(contractIndex,
                      models.contractModel().roleNames().key("propertyIds"))
                .toList()
                .first()
                .toString(),
            QStringLiteral("property-1"));

  const QModelIndex statementIndex = models.statements().index(0, 0);
  EXPECT_EQ(models.statements()
                .data(statementIndex, StatementViewModel::IdRole)
                .toString(),
            QStringLiteral("statement-1"));
  EXPECT_EQ(models.statements()
                .data(statementIndex, StatementViewModel::NameRole)
                .toString(),
            QStringLiteral("January Statement"));

  const QModelIndex txIndex = models.transactions().index(0, 0);
  EXPECT_EQ(
      models.transactions().data(txIndex, TransactionViewModel::IdRole).toString(),
      QStringLiteral("tx-1"));
  EXPECT_EQ(models.transactions()
                .data(txIndex, TransactionViewModel::StatementIdRole)
                .toString(),
            QStringLiteral("statement-1"));
  EXPECT_EQ(
      models.transactions().data(txIndex, TransactionViewModel::NameRole).toString(),
      QStringLiteral("Rent"));
  EXPECT_EQ(models.transactions()
                .data(txIndex, TransactionViewModel::BookingDateRole)
                .toString(),
            QStringLiteral("2026-01-05"));
  EXPECT_EQ(models.transactions()
                .data(txIndex, TransactionViewModel::ValutaRole)
                .toString(),
            QStringLiteral("EUR"));
  EXPECT_DOUBLE_EQ(models.transactions()
                       .data(txIndex, TransactionViewModel::AmountRole)
                       .toDouble(),
                   1250.0);
  EXPECT_EQ(
      models.transactions().data(txIndex, TransactionViewModel::StatusRole).toInt(),
      2);
  EXPECT_EQ(models.transactions()
                .data(txIndex, TransactionViewModel::ActorIdRole)
                .toString(),
            QStringLiteral("actor-1"));
  EXPECT_EQ(
      models.transactions().data(txIndex, TransactionViewModel::TypeRole).toString(),
      QStringLiteral("lease"));
  EXPECT_TRUE(models.transactions()
                  .data(txIndex, TransactionViewModel::AllocatableRole)
                  .toBool());
  EXPECT_EQ(models.transactions()
                .data(txIndex, TransactionViewModel::PropertyIdsRole)
                .toList()
                .first()
                .toString(),
            QStringLiteral("property-1"));

  const QModelIndex analysisIndex = models.analyses().index(0, 0);
  EXPECT_EQ(
      models.analyses().data(analysisIndex, AnalysisList::IdRole).toString(),
      QStringLiteral("analysis-1"));
  EXPECT_EQ(
      models.analyses().data(analysisIndex, AnalysisList::NameRole).toString(),
      QStringLiteral("Monthly Analysis"));
  EXPECT_EQ(
      models.analyses().data(analysisIndex, AnalysisList::TypeRole).toString(),
      QStringLiteral("tabular"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::ConfigRole)
                .toString(),
            QStringLiteral("{\"groupBy\":\"month\"}"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::FilterRole)
                .toString(),
            QStringLiteral("{}"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::AdjustmentsRole)
                .toString(),
            QStringLiteral("{\"actor-1\":19.25}"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::ExportFormatRole)
                .toString(),
            QStringLiteral("csv"));
  EXPECT_TRUE(models.analyses()
                  .data(analysisIndex, AnalysisList::IncludeCalcAdjustmentsRole)
                  .toBool());
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::ExportStateRole)
                .toString(),
            QStringLiteral("{}"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::SnapshotTransactionsRole)
                .toString(),
            QStringLiteral("[{\"transactionId\":\"tx-1\"}]"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::CreatedAtRole)
                .toString(),
            QStringLiteral("2026-01-01T08:00:00Z"));
  EXPECT_EQ(models.analyses()
                .data(analysisIndex, AnalysisList::UpdatedAtRole)
                .toString(),
            QStringLiteral("2026-01-02T08:00:00Z"));

  const QModelIndex annualIndex = models.annuals().index(0, 0);
  EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::IdRole).toString(),
            QStringLiteral("annual-1"));
  EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::NameRole).toString(),
            QStringLiteral("2026"));
  EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::YearRole).toInt(),
            2026);
}

TEST(WorkspaceCacheModelsTest, RefreshesTransactionTypesAfterContractChanges) {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.contracts.front().type = "rental";

  WorkspaceCacheModels models;
  models.loadFromState(snapshot);

  const QModelIndex txIndex = models.transactions().index(0, 0);
  EXPECT_EQ(
      models.transactions().data(txIndex, TransactionViewModel::TypeRole).toString(),
      QStringLiteral("rental"));
}

} // namespace ui
