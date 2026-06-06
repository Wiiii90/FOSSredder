/**
 * @file ui/tests/unit/workflows/TestAnalysisWorkflow.cpp
 * @brief Tests for analysis workflow transaction previews.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QDate>
#include <QVariantList>
#include <QVariantMap>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/workflows/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

TEST(
    AnalysisWorkflowTest,
    WF_ANALYSIS_001_PreviewTransactionsReturnsAllMatchingWorkspaceTransactions) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const QVariantMap preview = workflow.previewTransactions(QStringLiteral(""));
  const QVariantList transactions =
      preview.value(QStringLiteral("transactions")).toList();
  const QVariantMap metrics = preview.value(QStringLiteral("metrics")).toMap();

  EXPECT_EQ(transactions.size(), 2);
  EXPECT_EQ(metrics.value(QStringLiteral("transactionCount")).toInt(), 2);
  EXPECT_EQ(metrics.value(QStringLiteral("statementCount")).toInt(), 1);

  ASSERT_FALSE(transactions.isEmpty());
  const QVariantMap first = transactions.front().toMap();
  EXPECT_EQ(first.value(QStringLiteral("id")).toString(),
            QStringLiteral("tx-1"));
  EXPECT_EQ(first.value(QStringLiteral("statementId")).toString(),
            QStringLiteral("statement-1"));
  EXPECT_EQ(first.value(QStringLiteral("contractType")).toString(),
            QStringLiteral("lease"));
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_002_PreviewTransactionsHonorsAnalysisFilters) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const QVariantMap preview =
      workflow.previewTransactions(QStringLiteral("allocatable=allocatable"));
  const QVariantList transactions =
      preview.value(QStringLiteral("transactions")).toList();
  const QVariantMap metrics = preview.value(QStringLiteral("metrics")).toMap();

  EXPECT_EQ(transactions.size(), 1);
  EXPECT_EQ(metrics.value(QStringLiteral("transactionCount")).toInt(), 1);
  ASSERT_FALSE(transactions.isEmpty());
  EXPECT_EQ(transactions.front().toMap().value(QStringLiteral("id")).toString(),
            QStringLiteral("tx-1"));
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_003_FilterSpecUsesYearDefaultAndOmitsAllSelectedGroups) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const int defaultYear = QDate::currentDate().year() - 1;
  const QString filterSpec = workflow.analysisFilterSpec(
      QStringLiteral("bookingDate"), QStringLiteral("year"), QString(),
      QString(), QString(), QStringList{}, QStringList{},
      QStringLiteral("all"));

  EXPECT_EQ(filterSpec, QStringLiteral("date>=%1-01-01;date<=%2-12-31")
                            .arg(defaultYear)
                            .arg(defaultYear));
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_004_FilterSpecEmitsExplicitClausesForPartialSelections) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const QString filterSpec = workflow.analysisFilterSpec(
      QStringLiteral("bookingDate"), QStringLiteral("range"),
      QStringLiteral("2025"), QStringLiteral("2025-01-01"),
      QStringLiteral("2025-12-31"), QStringList{QStringLiteral("property-1")},
      QStringList{QStringLiteral("lease")}, QStringLiteral("allocatable"));

  EXPECT_EQ(
      filterSpec,
      QStringLiteral("date>=2025-01-01;date<=2025-12-31;propertyId=property-1;"
                     "contract.type=lease;allocatable=allocatable"));
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_005_AdjustmentsStoreAdjustedAmountsByTransactionId) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  QVariantMap selected;
  selected.insert(QStringLiteral("id"), QStringLiteral("tx-1"));
  selected.insert(QStringLiteral("amount"), 100.0);
  QVariantMap ignored;
  ignored.insert(QStringLiteral("id"), QStringLiteral("tx-2"));
  ignored.insert(QStringLiteral("amount"), 50.0);

  const QVariantMap adjustments =
      workflow.analysisAdjustmentAmountsFromPercentText(
          QVariantList{selected, ignored}, QStringList{QStringLiteral("tx-1")},
          QStringLiteral("19"));

  EXPECT_DOUBLE_EQ(adjustments.value(QStringLiteral("tx-1")).toDouble(), 119.0);
  EXPECT_FALSE(adjustments.contains(QStringLiteral("tx-2")));
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_006_WorkspaceCommandsPersistAnalysisSnapshotAndAdjustments) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  commands.updateAnalysis(
      QStringLiteral("analysis-1"), QStringLiteral("Monthly Analysis Updated"),
      QStringLiteral("tabular"),
      QVariantMap{{QStringLiteral("type"), QStringLiteral("tabular")}},
      QVariantMap{{QStringLiteral("dateMode"), QStringLiteral("year")},
                  {QStringLiteral("year"), QStringLiteral("2026")}},
      QStringLiteral("xlsx"), false,
      QVariantList{QVariantMap{{QStringLiteral("id"), QStringLiteral("tx-1")}}},
      QVariantMap{{QStringLiteral("tx-1"), 1500.0}});

  const auto snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 1U);
  const auto& analysis = snapshot.analyses.front();
  EXPECT_EQ(analysis.name, std::string("Monthly Analysis Updated"));
  EXPECT_EQ(analysis.filter.year, std::string("2026"));
  EXPECT_EQ(analysis.exportFormat, std::string("xlsx"));
  EXPECT_FALSE(analysis.includeCalculationAdjustments);
  ASSERT_EQ(analysis.adjustments.size(), 1U);
  EXPECT_EQ(analysis.adjustments.front().first, std::string("tx-1"));
  EXPECT_DOUBLE_EQ(analysis.adjustments.front().second, 1500.0);
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_007_ComputePreviewCarriesStoredAdjustmentsForPlots) {
  auto state = tests::support::makeWorkspaceSnapshot();
  core::ports::workspace::AnalysisSnapshot analysis;
  analysis.id = "analysis-plot";
  analysis.name = "Plot With Adjustments";
  analysis.type = "plot";
  analysis.config.type = "plot";
  analysis.config.plotType = "pie";
  analysis.config.plotMeasure = "totalAmount";
  analysis.exportFormat = "png";
  analysis.includeCalculationAdjustments = true;
  analysis.adjustments.emplace_back("tx-1", 1500.0);
  state.analyses = {analysis};

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const QVariantMap withAdjustments = workflow.computeAnalysisPreview(
      QStringLiteral("analysis-plot"), QStringLiteral(""), true, QVariantMap{});
  const QVariantList adjustedTransactions =
      withAdjustments.value(QStringLiteral("transactions")).toList();
  ASSERT_FALSE(adjustedTransactions.isEmpty());
  EXPECT_DOUBLE_EQ(adjustedTransactions.front()
                       .toMap()
                       .value(QStringLiteral("amount"))
                       .toDouble(),
                   1500.0);
  const QVariantList adjustedTable =
      withAdjustments.value(QStringLiteral("table")).toList();
  ASSERT_FALSE(adjustedTable.isEmpty());
  const double adjustedVisibleAmount =
      adjustedTable.front().toList().value(1).toString().toDouble();

  const QVariantMap withoutAdjustments =
      workflow.computeAnalysisPreview(QStringLiteral("analysis-plot"),
                                      QStringLiteral(""), false, QVariantMap{});
  const QVariantList plainTransactions =
      withoutAdjustments.value(QStringLiteral("transactions")).toList();
  ASSERT_FALSE(plainTransactions.isEmpty());
  EXPECT_DOUBLE_EQ(plainTransactions.front()
                       .toMap()
                       .value(QStringLiteral("amount"))
                       .toDouble(),
                   1250.0);
  const QVariantList plainTable =
      withoutAdjustments.value(QStringLiteral("table")).toList();
  ASSERT_FALSE(plainTable.isEmpty());
  const double plainVisibleAmount =
      plainTable.front().toList().value(1).toString().toDouble();

  EXPECT_NE(adjustedVisibleAmount, plainVisibleAmount);
}

TEST(AnalysisWorkflowTest,
     WF_ANALYSIS_008_ComputePreviewIncludesProjectedTableState) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [state]() {
        return state;
      },
      analysisAdapter);

  const QVariantMap preview = workflow.computeAnalysisPreview(
      QStringLiteral("analysis-1"), QStringLiteral(""), false, QVariantMap{});
  ASSERT_TRUE(preview.contains(QStringLiteral("tableState")));
  const QVariantMap table = preview.value(QStringLiteral("tableState")).toMap();

  EXPECT_TRUE(table.contains(QStringLiteral("contractTypes")));
  EXPECT_TRUE(table.contains(QStringLiteral("propertyRows")));
  EXPECT_TRUE(table.contains(QStringLiteral("grandTotal")));
}

} // namespace ui
