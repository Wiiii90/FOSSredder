/**
 * @file ui/tests/unit/TestAnalysisState.cpp
 * @brief Tests for the UI AnalysisViewModel boundary.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

struct AnalysisStateHarness {
  std::unique_ptr<tests::support::InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceFacade> facade;
  std::unique_ptr<AnalysisWorkflow> workflow;
  std::unique_ptr<AnalysisViewModel> state;
};

AnalysisStateHarness makeHarnessWithAdjustedAnalysis() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  core::ports::workspace::AnalysisSnapshot analysis;
  analysis.id = "analysis-plot";
  analysis.name = "Plot With Adjustments";
  analysis.type = "plot";
  analysis.configJson =
      "{\"plotType\":\"pie\",\"plotMeasure\":\"totalAmount\"}";
  analysis.filterSpec = "date>=2026-01-01;date<=2026-12-31";
  analysis.exportFormat = "png";
  analysis.includeCalculationAdjustments = true;
  analysis.adjustments.emplace_back("tx-1", 1500.0);
  snapshot.analyses = {analysis};

  auto workspace =
      std::make_unique<tests::support::InMemoryWorkspace>(std::move(snapshot));
  auto *workspacePtr = workspace.get();
  auto facade = std::make_unique<WorkspaceFacade>(workspacePtr, workspacePtr);
  facade->selection()->setSelectedAnalysisId(QStringLiteral("analysis-plot"));

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  auto workflow = std::make_unique<AnalysisWorkflow>(
      [workspacePtr]() { return workspacePtr->workspaceSnapshot(); },
      analysisAdapter, nullptr);

  auto state = std::make_unique<AnalysisViewModel>();
  state->setWorkspace(facade.get());
  state->setAnalysisWorkflow(workflow.get());

  return {std::move(workspace), std::move(facade), std::move(workflow),
          std::move(state)};
}

AnalysisStateHarness makeCreateHarness() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.analyses.clear();
  auto workspace =
      std::make_unique<tests::support::InMemoryWorkspace>(std::move(snapshot));
  auto *workspacePtr = workspace.get();
  auto facade = std::make_unique<WorkspaceFacade>(workspacePtr, workspacePtr);
  facade->selection()->setSelectedAnalysisId(QString());

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  auto workflow = std::make_unique<AnalysisWorkflow>(
      [workspacePtr]() { return workspacePtr->workspaceSnapshot(); },
      analysisAdapter, nullptr);

  auto state = std::make_unique<AnalysisViewModel>();
  state->setWorkspace(facade.get());
  state->setAnalysisWorkflow(workflow.get());

  return {std::move(workspace), std::move(facade), std::move(workflow),
          std::move(state)};
}

bool variantListContains(const QVariantList &values, const QString &expected) {
  return std::any_of(
      values.cbegin(), values.cend(), [&expected](const QVariant &value) {
        const QVariantMap row = value.toMap();
        return row.value(QStringLiteral("value"), value).toString() == expected;
      });
}

bool propertyRowsContainId(const QVariantList &rows, const QString &expected) {
  return std::any_of(
      rows.cbegin(), rows.cend(), [&expected](const QVariant &value) {
        return value.toMap().value(QStringLiteral("id")).toString() == expected;
      });
}

} // namespace

TEST(AnalysisStateTest,
     ANL_ST_001_IncludeCalcToggleRefreshesVisiblePreviewState) {
  auto harness = makeHarnessWithAdjustedAnalysis();

  ASSERT_TRUE(harness.state->includeCalcAdjustments());
  ASSERT_EQ(harness.state->selectedAdjustmentTxIds().size(), 1);
  EXPECT_EQ(harness.state->selectedAdjustmentTxIds().front().toString(),
            QStringLiteral("tx-1"));
  const double adjustedTotal = harness.state->tableGrandTotal();
  const QString adjustedPreviewSource = harness.state->renderedPreviewSource();
  ASSERT_FALSE(adjustedPreviewSource.isEmpty());

  harness.state->setIncludeCalcAdjustments(false);
  const double plainTotal = harness.state->tableGrandTotal();
  const QString plainPreviewSource = harness.state->renderedPreviewSource();

  EXPECT_NE(adjustedTotal, plainTotal);
  EXPECT_NE(adjustedPreviewSource, plainPreviewSource);
  EXPECT_DOUBLE_EQ(plainTotal, 1214.5);
}

TEST(AnalysisStateTest,
     ANL_ST_002_CreatePersistsCalcAdjustmentsForInitialPreview) {
  auto harness = makeCreateHarness();

  harness.state->setYearValue(QStringLiteral("2026"));
  harness.state->refreshPreview();
  ASSERT_EQ(harness.state->previewTransactions().size(), 2);

  harness.state->setAdjustmentTransactionSelected(QStringLiteral("tx-2"), true);
  harness.state->setCalcPercentText(QStringLiteral("20,0 %"));
  harness.state->applySelectedCalc();

  const QJsonObject pendingAdjustments =
      QJsonDocument::fromJson(harness.state->pendingAdjustmentsJson().toUtf8())
          .object();
  ASSERT_TRUE(pendingAdjustments.contains(QStringLiteral("tx-2")));
  EXPECT_DOUBLE_EQ(pendingAdjustments.value(QStringLiteral("tx-2")).toDouble(),
                   -42.6);

  harness.state->setName(QStringLiteral("Created Plot"));
  harness.state->submitCreate();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 1U);
  ASSERT_EQ(snapshot.analyses.front().adjustments.size(), 1U);
  EXPECT_EQ(snapshot.analyses.front().adjustments.front().first,
            std::string("tx-2"));
  EXPECT_DOUBLE_EQ(snapshot.analyses.front().adjustments.front().second, -42.6);
  EXPECT_EQ(harness.state->pendingAdjustmentsJson(),
            QStringLiteral("{\"tx-2\":-42.6}"));

  const QJsonObject config =
      QJsonDocument::fromJson(
          QString::fromStdString(snapshot.analyses.front().configJson).toUtf8())
          .object();
  EXPECT_TRUE(config.value(QStringLiteral("properties")).toArray().isEmpty());
  EXPECT_TRUE(
      config.value(QStringLiteral("contractTypes")).toArray().isEmpty());

  const QVariantMap renderedWithStoredAdjustments =
      harness.workflow->computeAnalysisPreview(
          harness.facade->selection()->selectedAnalysisId(), QString(), true,
          QStringLiteral("{}"));
  const QVariantList renderedTransactions =
      renderedWithStoredAdjustments.value(QStringLiteral("transactions"))
          .toList();
  ASSERT_FALSE(renderedTransactions.isEmpty());
  bool renderedAdjustedTransaction = false;
  for (const QVariant &transactionValue : renderedTransactions) {
    const QVariantMap transaction = transactionValue.toMap();
    if (transaction.value(QStringLiteral("id")).toString() ==
            QStringLiteral("tx-2") &&
        transaction.value(QStringLiteral("amount")).toDouble() == -42.6) {
      renderedAdjustedTransaction = true;
    }
  }
  EXPECT_TRUE(renderedAdjustedTransaction);
  const QVariantList renderedTable =
      renderedWithStoredAdjustments.value(QStringLiteral("table")).toList();
  ASSERT_FALSE(renderedTable.isEmpty());
  bool renderedAdjustedAmount = false;
  for (const QVariant &rowValue : renderedTable) {
    const QVariantList row = rowValue.toList();
    if (row.size() > 1 && row.value(1).toString().toDouble() == 42.6) {
      renderedAdjustedAmount = true;
    }
  }
  EXPECT_TRUE(renderedAdjustedAmount);

  EXPECT_DOUBLE_EQ(harness.state->tableGrandTotal(), 1207.4);
  const QString adjustedPreviewSource = harness.state->renderedPreviewSource();
  ASSERT_FALSE(adjustedPreviewSource.isEmpty());

  harness.state->setIncludeCalcAdjustments(false);

  EXPECT_DOUBLE_EQ(harness.state->tableGrandTotal(), 1214.5);
  EXPECT_NE(harness.state->renderedPreviewSource(), adjustedPreviewSource);
}

TEST(AnalysisStateTest, ANL_ST_003_UpdatePreservesStoredCalcAdjustmentAmounts) {
  auto harness = makeHarnessWithAdjustedAnalysis();

  harness.state->setName(QStringLiteral("Renamed Plot"));
  harness.state->submitUpdate();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 1U);
  ASSERT_EQ(snapshot.analyses.front().adjustments.size(), 1U);
  EXPECT_EQ(snapshot.analyses.front().adjustments.front().first,
            std::string("tx-1"));
  EXPECT_DOUBLE_EQ(snapshot.analyses.front().adjustments.front().second,
                   1500.0);
  EXPECT_EQ(harness.state->pendingAdjustmentsJson(),
            QStringLiteral("{\"tx-1\":1500}"));
}

TEST(AnalysisStateTest,
     ANL_ST_004_FilterChoicesExposeUnassignedAndNoneSelection) {
  auto harness = makeCreateHarness();

  ASSERT_TRUE(propertyRowsContainId(harness.state->propertyFilterRows(),
                                    QStringLiteral("unassigned")));
  ASSERT_TRUE(variantListContains(harness.state->contractTypeRows(),
                                  QStringLiteral("unassigned")));
  EXPECT_TRUE(variantListContains(harness.state->selectedPropertyIds(),
                                  QStringLiteral("unassigned")));
  EXPECT_TRUE(variantListContains(harness.state->selectedContractTypes(),
                                  QStringLiteral("unassigned")));

  harness.state->selectUnassignedProperties();
  ASSERT_EQ(harness.state->selectedPropertyIds().size(), 1);
  EXPECT_EQ(harness.state->selectedPropertyIds().front().toString(),
            QStringLiteral("unassigned"));

  harness.state->selectUnassignedContractTypes();
  ASSERT_EQ(harness.state->selectedContractTypes().size(), 1);
  EXPECT_EQ(harness.state->selectedContractTypes().front().toString(),
            QStringLiteral("unassigned"));

  harness.state->setName(QStringLiteral("Analysis"));
  EXPECT_TRUE(harness.state->canSubmit());

  harness.state->selectNoProperties();
  EXPECT_TRUE(harness.state->selectedPropertyIds().isEmpty());
  EXPECT_FALSE(harness.state->canSubmit());

  harness.state->selectAllProperties();
  EXPECT_TRUE(harness.state->canSubmit());

  harness.state->selectNoContractTypes();
  EXPECT_TRUE(harness.state->selectedContractTypes().isEmpty());
  EXPECT_FALSE(harness.state->canSubmit());
}

} // namespace ui
