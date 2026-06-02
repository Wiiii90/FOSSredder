/**
 * @file ui/tests/unit/TestExportState.cpp
 * @brief Tests for the UI ExportViewModel boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/state/settings/SettingsStore.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

struct ExportStateHarness {
  std::unique_ptr<tests::support::InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceFacade> facade;
  std::unique_ptr<ExportViewModel> state;
};

auto makeExportSnapshot() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  auto tabular = tests::support::makeAnalysis();
  tabular.id = "analysis-table";
  tabular.name = "Table Analysis";
  tabular.type = "tabular";
  tabular.exportFormat = "csv";

  auto plot = tests::support::makeAnalysis();
  plot.id = "analysis-plot";
  plot.name = "Plot Analysis";
  plot.type = "plot";
  plot.exportFormat = "png";

  auto annual = tests::support::makeAnnual();
  annual.id = "annual-export";
  annual.name = "Annual Export";
  annual.analysisIds = {"analysis-table", "analysis-plot"};

  snapshot.analyses = {tabular, plot};
  snapshot.annuals = {annual};
  return snapshot;
}

ExportStateHarness makeHarness() {
  auto workspace =
      std::make_unique<tests::support::InMemoryWorkspace>(makeExportSnapshot());
  auto *workspacePtr = workspace.get();
  auto facade = std::make_unique<WorkspaceFacade>(workspacePtr, workspacePtr);

  auto state = std::make_unique<ExportViewModel>();
  state->setWorkspace(facade.get());
  state->setTargetDirectory(QStringLiteral("test:///export"));

  return {std::move(workspace), std::move(facade), std::move(state)};
}

QVariantMap firstMap(const QVariantList &values) {
  return values.empty() ? QVariantMap() : values.front().toMap();
}

} // namespace

TEST(ExportStateTest, EXP_ST_001_LoadsWorkspaceRowsAndPendingAnnual) {
  auto harness = makeHarness();

  ASSERT_EQ(harness.state->annualRows().size(), 1);
  ASSERT_EQ(harness.state->analysisRows().size(), 2);
  EXPECT_EQ(harness.state->addMode(), QStringLiteral("annual"));
  EXPECT_EQ(harness.state->pendingIndex(), 0);
  EXPECT_TRUE(harness.state->canAddEntry());
}

TEST(ExportStateTest, EXP_ST_002_AnnualEntryProjectsAssignedAnalysesForExport) {
  auto harness = makeHarness();

  harness.state->addPendingEntry();
  const QVariantList items = harness.state->exportItems();

  ASSERT_EQ(items.size(), 3);
  EXPECT_EQ(items.at(0).toMap().value(QStringLiteral("objectType")).toString(),
            QStringLiteral("Annual"));
  EXPECT_EQ(items.at(0).toMap().value(QStringLiteral("objectId")).toString(),
            QStringLiteral("annual-export"));
  EXPECT_EQ(items.at(1).toMap().value(QStringLiteral("annualId")).toString(),
            QStringLiteral("annual-export"));
  EXPECT_EQ(items.at(2).toMap().value(QStringLiteral("exportType")).toString(),
            QStringLiteral("PNG"));
}

TEST(ExportStateTest, EXP_ST_003_StandalonePlotDefaultsToImageExportOptions) {
  auto harness = makeHarness();

  harness.state->setAddMode(QStringLiteral("analysis"));
  harness.state->selectPendingRow(1);
  harness.state->addPendingEntry();

  const QVariantMap entry = firstMap(harness.state->exportEntries());
  EXPECT_FALSE(entry.value(QStringLiteral("isAnnual")).toBool());
  EXPECT_EQ(entry.value(QStringLiteral("objectId")).toString(),
            QStringLiteral("analysis-plot"));
  EXPECT_EQ(entry.value(QStringLiteral("exportType")).toString(),
            QStringLiteral("PNG"));
  EXPECT_EQ(entry.value(QStringLiteral("exportTypeOptions")).toList().size(),
            2);
}

TEST(ExportStateTest,
     EXP_ST_004_LoadItemsRoundTripsAnnualWithoutDuplicateAnalyses) {
  auto harness = makeHarness();

  harness.state->loadItems({
      QVariantMap{
          {QStringLiteral("objectType"), QStringLiteral("Annual")},
          {QStringLiteral("objectId"), QStringLiteral("annual-export")},
          {QStringLiteral("objectName"), QStringLiteral("Annual Export")}},
      QVariantMap{
          {QStringLiteral("objectType"), QStringLiteral("Analysis")},
          {QStringLiteral("annualId"), QStringLiteral("annual-export")},
          {QStringLiteral("objectId"), QStringLiteral("analysis-plot")},
          {QStringLiteral("objectName"), QStringLiteral("Plot Analysis")},
          {QStringLiteral("exportType"), QStringLiteral("JPG")}},
  });

  const QVariantList entries = harness.state->exportEntries();
  ASSERT_EQ(entries.size(), 1);
  const QVariantList analyses =
      entries.front().toMap().value(QStringLiteral("analyses")).toList();
  ASSERT_EQ(analyses.size(), 2);
  EXPECT_EQ(analyses.at(1).toMap().value(QStringLiteral("objectId")).toString(),
            QStringLiteral("analysis-plot"));
  EXPECT_EQ(
      analyses.at(1).toMap().value(QStringLiteral("exportType")).toString(),
      QStringLiteral("JPG"));
}

TEST(ExportStateTest, EXP_ST_005_SettingsDefaultsRefreshPristineExportForm) {
  ExportViewModel state;
  SettingsStore settings;

  settings.setExportDefaultDirectory(QStringLiteral("test:///exports/one"));
  settings.setExportArchiveFormat(1);
  state.setSettings(&settings);

  EXPECT_EQ(state.targetDirectory(), QStringLiteral("test:///exports/one"));
  EXPECT_EQ(state.packageFormatIndex(), 1);

  settings.setExportDefaultDirectory(QStringLiteral("test:///exports/two"));
  EXPECT_EQ(state.targetDirectory(), QStringLiteral("test:///exports/two"));

  state.setTargetDirectory(QStringLiteral("test:///exports/manual"));
  settings.setExportDefaultDirectory(QStringLiteral("test:///exports/three"));
  EXPECT_EQ(state.targetDirectory(), QStringLiteral("test:///exports/manual"));

  state.clearForm();
  EXPECT_EQ(state.targetDirectory(), QStringLiteral("test:///exports/three"));
}

} // namespace ui
