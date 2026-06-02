/**
 * @file ui/tests/unit/TestAnalysisWorkflow.cpp
 * @brief Tests for analysis workflow transaction previews.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

TEST(AnalysisWorkflowTest,
     PreviewTransactionsReturnsAllMatchingWorkspaceTransactions) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

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

TEST(AnalysisWorkflowTest, PreviewTransactionsHonorsAnalysisFilters) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

  const QVariantMap preview =
      workflow.previewTransactions(QStringLiteral("amount>=1000"));
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
     AnalysisFilterSpecUsesYearDefaultAndOmitsAllSelectedGroups) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

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
     AnalysisFilterSpecEmitsExplicitClausesForPartialSelections) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

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
     ParseAnalysisFilterSpecMapsCoreFilterStateBackToUiState) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

  const QVariantMap parsed = workflow.parseAnalysisFilterSpec(
      QStringLiteral("dateField=valuta;date>=2025-01-01;date<=2025-12-31;"
                     "propertyId=property-1,unassigned;contract.type=lease,"
                     "unassigned;allocatable=non-allocatable"));

  EXPECT_EQ(parsed.value(QStringLiteral("dateField")).toString(),
            QStringLiteral("valuta"));
  EXPECT_EQ(parsed.value(QStringLiteral("dateMode")).toString(),
            QStringLiteral("year"));
  EXPECT_EQ(parsed.value(QStringLiteral("year")).toString(),
            QStringLiteral("2025"));
  EXPECT_EQ(parsed.value(QStringLiteral("allocatableMode")).toString(),
            QStringLiteral("non-allocatable"));

  const QVariantList propertyIds =
      parsed.value(QStringLiteral("propertyIds")).toList();
  ASSERT_EQ(propertyIds.size(), 1);
  EXPECT_EQ(propertyIds.front().toString(), QStringLiteral("property-1"));
  EXPECT_EQ(parsed.value(QStringLiteral("propertyIdsNone")).toBool(), true);

  const QVariantList contractTypes =
      parsed.value(QStringLiteral("contractTypes")).toList();
  ASSERT_EQ(contractTypes.size(), 1);
  EXPECT_EQ(contractTypes.front().toString(), QStringLiteral("lease"));
  EXPECT_EQ(parsed.value(QStringLiteral("contractTypesNone")).toBool(), true);
}

TEST(AnalysisWorkflowTest, AnalysisConfigJsonUsesCoreAnalysisKeys) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

  const QString json = workflow.analysisConfigJson(
      QStringLiteral("plot"), QStringLiteral("histogram"),
      QStringLiteral("totalAmount"), QStringList{QStringLiteral("property-1")},
      QStringList{QStringLiteral("lease")}, 0.0);

  const QJsonObject parsed = QJsonDocument::fromJson(json.toUtf8()).object();
  EXPECT_EQ(parsed.value(QStringLiteral("plotType")).toString(),
            QStringLiteral("histogram"));
  EXPECT_EQ(parsed.value(QStringLiteral("plotMeasure")).toString(),
            QStringLiteral("totalAmount"));
  ASSERT_TRUE(parsed.value(QStringLiteral("properties")).isArray());
  EXPECT_EQ(
      parsed.value(QStringLiteral("properties")).toArray().at(0).toString(),
      QStringLiteral("property-1"));
  ASSERT_TRUE(parsed.value(QStringLiteral("contractTypes")).isArray());
  EXPECT_EQ(
      parsed.value(QStringLiteral("contractTypes")).toArray().at(0).toString(),
      QStringLiteral("lease"));
}

TEST(AnalysisWorkflowTest,
     AnalysisAdjustmentsJsonStoresAdjustedAmountsByTransactionId) {
  auto state = tests::support::makeWorkspaceSnapshot();
  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

  QVariantMap selected;
  selected.insert(QStringLiteral("id"), QStringLiteral("tx-1"));
  selected.insert(QStringLiteral("amount"), 100.0);
  QVariantMap ignored;
  ignored.insert(QStringLiteral("id"), QStringLiteral("tx-2"));
  ignored.insert(QStringLiteral("amount"), 50.0);

  const QString json = workflow.analysisAdjustmentsJson(
      QVariantList{selected, ignored}, QStringList{QStringLiteral("tx-1")},
      19.0);

  const QJsonObject parsed = QJsonDocument::fromJson(json.toUtf8()).object();
  EXPECT_DOUBLE_EQ(parsed.value(QStringLiteral("tx-1")).toDouble(), 119.0);
  EXPECT_FALSE(parsed.contains(QStringLiteral("tx-2")));
}

TEST(AnalysisWorkflowTest,
     WorkspaceFacadePersistsSerializedAnalysisInputs) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceFacade facade(&workspace, &workspace);

  facade.saveAnalysis(
      QStringLiteral("analysis-1"), QStringLiteral("Monthly Analysis Updated"),
      QStringLiteral("tabular"), QStringLiteral("{\"groupBy\":\"month\"}"),
      QStringLiteral("amount>=1000"), QStringLiteral("xlsx"), false,
      QStringLiteral("{\"columns\":[\"name\"]}"), QStringLiteral("[\"tx-1\"]"),
      QStringLiteral("{\"tx-1\":1500.0}"));

  const auto snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 1U);
  const auto &analysis = snapshot.analyses.front();
  EXPECT_EQ(analysis.name, std::string("Monthly Analysis Updated"));
  EXPECT_EQ(analysis.filterSpec, std::string("amount>=1000"));
  EXPECT_EQ(analysis.exportFormat, std::string("xlsx"));
  EXPECT_FALSE(analysis.includeCalculationAdjustments);
  ASSERT_EQ(analysis.adjustments.size(), 1U);
  EXPECT_EQ(analysis.adjustments.front().first, std::string("tx-1"));
  EXPECT_DOUBLE_EQ(analysis.adjustments.front().second, 1500.0);
}

TEST(AnalysisWorkflowTest,
     ComputeAnalysisPreviewCarriesStoredAdjustmentsForPlots) {
  auto state = tests::support::makeWorkspaceSnapshot();
  core::ports::workspace::AnalysisSnapshot analysis;
  analysis.id = "analysis-plot";
  analysis.name = "Plot With Adjustments";
  analysis.type = "plot";
  analysis.configJson =
      "{\"plotType\":\"pie\",\"plotMeasure\":\"totalAmount\"}";
  analysis.filterSpec = "";
  analysis.exportFormat = "png";
  analysis.includeCalculationAdjustments = true;
  analysis.adjustments.emplace_back("tx-1", 1500.0);
  state.analyses = {analysis};

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow([state]() { return state; }, analysisAdapter);

  const QVariantMap withAdjustments = workflow.computeAnalysisPreview(
      QStringLiteral("analysis-plot"), QStringLiteral(""), true,
      QStringLiteral("{}"));
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

  const QVariantMap withoutAdjustments = workflow.computeAnalysisPreview(
      QStringLiteral("analysis-plot"), QStringLiteral(""), false,
      QStringLiteral("{}"));
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

} // namespace ui
