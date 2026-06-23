/**
 * @file ui/tests/unit/viewmodels/TestAnalysisViewModel.cpp
 * @brief Tests for the UI AnalysisViewModel boundary.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/workflows/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

void waitForUiEvents(int milliseconds) {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < milliseconds) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    QThread::msleep(1);
  }
  QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

struct AnalysisStateHarness {
  std::unique_ptr<tests::support::InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceStore> store;
  std::unique_ptr<WorkspaceCommands> commands;
  std::unique_ptr<WorkspaceSelectors> selectors;
  std::unique_ptr<WorkspaceSelection> selection;
  std::unique_ptr<AnalysisWorkflow> workflow;
  std::unique_ptr<AnalysisViewModel> state;
};

AnalysisStateHarness makeHarnessWithAdjustedAnalysis() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  core::ports::workspace::AnalysisSnapshot analysis;
  analysis.id = "analysis-plot";
  analysis.name = "Plot With Adjustments";
  analysis.type = "plot";
  analysis.config.type = "plot";
  analysis.config.plotType = "pie";
  analysis.config.plotMeasure = "totalAmount";
  analysis.filter.dateMode = "year";
  analysis.filter.year = "2026";
  analysis.exportFormat = "png";
  analysis.includeCalculationAdjustments = true;
  analysis.adjustments.emplace_back("tx-1", 1500.0);
  snapshot.analyses = {analysis};

  auto workspace =
      std::make_unique<tests::support::InMemoryWorkspace>(std::move(snapshot));
  auto* workspacePtr = workspace.get();
  auto store = std::make_unique<WorkspaceStore>();
  store->setWorkspacePorts(workspacePtr, workspacePtr);
  store->loadFromState(workspacePtr->workspaceSnapshot());
  auto commands = std::make_unique<WorkspaceCommands>(*store);
  auto selectors = std::make_unique<WorkspaceSelectors>(*store);
  auto selection = std::make_unique<WorkspaceSelection>(*store, *selectors);
  workspace->setSnapshotChangedCallback(
      [storePtr = store.get()](
          const core::ports::workspace::WorkspaceSnapshot& nextSnapshot) {
        storePtr->loadFromState(nextSnapshot);
      });
  selection->setSelectedAnalysisId(QStringLiteral("analysis-plot"));

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  auto workflow = std::make_unique<AnalysisWorkflow>(
      [workspacePtr]() {
        return workspacePtr->workspaceSnapshot();
      },
      analysisAdapter, tests::support::noopErrorReporter(), nullptr);

  auto state = std::make_unique<AnalysisViewModel>();
  state->setWorkspaceRoles(store.get(), commands.get(), selection.get(),
                           selectors.get());
  state->setAnalysisWorkflow(workflow.get());

  return {std::move(workspace), std::move(store),     std::move(commands),
          std::move(selectors), std::move(selection), std::move(workflow),
          std::move(state)};
}

AnalysisStateHarness makeCreateHarness() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.analyses.clear();
  auto workspace =
      std::make_unique<tests::support::InMemoryWorkspace>(std::move(snapshot));
  auto* workspacePtr = workspace.get();
  auto store = std::make_unique<WorkspaceStore>();
  store->setWorkspacePorts(workspacePtr, workspacePtr);
  store->loadFromState(workspacePtr->workspaceSnapshot());
  auto commands = std::make_unique<WorkspaceCommands>(*store);
  auto selectors = std::make_unique<WorkspaceSelectors>(*store);
  auto selection = std::make_unique<WorkspaceSelection>(*store, *selectors);
  workspace->setSnapshotChangedCallback(
      [storePtr = store.get()](
          const core::ports::workspace::WorkspaceSnapshot& nextSnapshot) {
        storePtr->loadFromState(nextSnapshot);
      });
  selection->setSelectedAnalysisId(QString());

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  auto workflow = std::make_unique<AnalysisWorkflow>(
      [workspacePtr]() {
        return workspacePtr->workspaceSnapshot();
      },
      analysisAdapter, tests::support::noopErrorReporter(), nullptr);

  auto state = std::make_unique<AnalysisViewModel>();
  state->setWorkspaceRoles(store.get(), commands.get(), selection.get(),
                           selectors.get());
  state->setAnalysisWorkflow(workflow.get());

  return {std::move(workspace), std::move(store),     std::move(commands),
          std::move(selectors), std::move(selection), std::move(workflow),
          std::move(state)};
}

bool variantListContains(const QVariantList& values, const QString& expected) {
  return std::any_of(
      values.cbegin(), values.cend(), [&expected](const QVariant& value) {
        const QVariantMap row = value.toMap();
        return row.value(QStringLiteral("value"), value).toString() == expected;
      });
}

bool propertyRowsContainId(const QVariantList& rows, const QString& expected) {
  return std::any_of(
      rows.cbegin(), rows.cend(), [&expected](const QVariant& value) {
        return value.toMap().value(QStringLiteral("id")).toString() == expected;
      });
}

} // namespace

TEST(AnalysisViewModelTest,
     VM_ANALYSIS_001_IncludeAdjustmentsToggleRefreshesVisiblePreviewState) {
  auto harness = makeHarnessWithAdjustedAnalysis();

  ASSERT_TRUE(harness.state->includeAdjustments());
  ASSERT_EQ(harness.state->selectedAdjustmentTxIds().size(), 1);
  EXPECT_EQ(harness.state->selectedAdjustmentTxIds().front().toString(),
            QStringLiteral("tx-1"));
  const double adjustedTotal = harness.state->tableGrandTotal();
  const QString adjustedPreviewSource = harness.state->renderedPreviewSource();
  ASSERT_FALSE(adjustedPreviewSource.isEmpty());

  harness.state->setIncludeAdjustments(false);
  const double plainTotal = harness.state->tableGrandTotal();
  const QString plainPreviewSource = harness.state->renderedPreviewSource();

  EXPECT_NE(adjustedTotal, plainTotal);
  EXPECT_NE(adjustedPreviewSource, plainPreviewSource);
  EXPECT_DOUBLE_EQ(plainTotal, 1214.5);
}

TEST(AnalysisViewModelTest,
     VM_ANALYSIS_002_CreatePersistsAdjustmentsForInitialPreview) {
  auto harness = makeCreateHarness();

  harness.state->setYearValue(QStringLiteral("2026"));
  waitForUiEvents(60);
  ASSERT_EQ(harness.state->previewTransactionRows().size(), 2);

  harness.state->setAdjustmentTransactionSelected(QStringLiteral("tx-2"), true);
  harness.state->setAdjustmentPercentText(QStringLiteral("20,0 %"));
  harness.state->applySelectedAdjustment();

  harness.state->setName(QStringLiteral("Created Plot"));
  harness.state->submitCreate();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 1U);
  ASSERT_EQ(snapshot.analyses.front().adjustments.size(), 1U);
  EXPECT_EQ(snapshot.analyses.front().adjustments.front().first,
            std::string("tx-2"));
  EXPECT_DOUBLE_EQ(snapshot.analyses.front().adjustments.front().second, -42.6);
  EXPECT_TRUE(snapshot.analyses.front().config.propertyIds.empty());
  EXPECT_TRUE(snapshot.analyses.front().config.contractTypes.empty());

  const QVariantMap renderedWithStoredAdjustments =
      harness.workflow->computeAnalysisPreview(
          harness.selection->selectedAnalysisId(), QString(), true,
          QVariantMap{});
  const QVariantList renderedTransactions =
      renderedWithStoredAdjustments.value(QStringLiteral("transactions"))
          .toList();
  ASSERT_FALSE(renderedTransactions.isEmpty());
  bool renderedAdjustedTransaction = false;
  for (const QVariant& transactionValue : renderedTransactions) {
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
  for (const QVariant& rowValue : renderedTable) {
    const QVariantList row = rowValue.toList();
    if (row.size() > 1 && row.value(1).toString().toDouble() == 42.6) {
      renderedAdjustedAmount = true;
    }
  }
  EXPECT_TRUE(renderedAdjustedAmount);

  EXPECT_DOUBLE_EQ(harness.state->tableGrandTotal(), 1207.4);
  const QString adjustedPreviewSource = harness.state->renderedPreviewSource();
  ASSERT_FALSE(adjustedPreviewSource.isEmpty());

  harness.state->setIncludeAdjustments(false);

  EXPECT_DOUBLE_EQ(harness.state->tableGrandTotal(), 1214.5);
  EXPECT_NE(harness.state->renderedPreviewSource(), adjustedPreviewSource);
}

TEST(AnalysisViewModelTest,
     VM_ANALYSIS_003_UpdatePreservesStoredAdjustmentAmounts) {
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
}

TEST(AnalysisViewModelTest,
     VM_ANALYSIS_004_FilterOptionsExposeUnassignedAndNoneSelection) {
  auto harness = makeCreateHarness();

  ASSERT_TRUE(propertyRowsContainId(harness.state->propertyFilterRows(),
                                    QStringLiteral("unassigned")));
  ASSERT_TRUE(variantListContains(harness.state->contractTypeRows(),
                                  QStringLiteral("unassigned")));
  EXPECT_TRUE(variantListContains(harness.state->selectedPropertyIds(),
                                  QStringLiteral("unassigned")));
  EXPECT_TRUE(variantListContains(harness.state->selectedContractTypes(),
                                  QStringLiteral("unassigned")));

  harness.state->setSelectedPropertyIds({QStringLiteral("unassigned")});
  ASSERT_EQ(harness.state->selectedPropertyIds().size(), 1);
  EXPECT_EQ(harness.state->selectedPropertyIds().front().toString(),
            QStringLiteral("unassigned"));

  harness.state->setSelectedContractTypes({QStringLiteral("unassigned")});
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
