/**
 * @file ui/tests/unit/viewmodels/TestExportViewModel.cpp
 * @brief Tests for the UI ExportViewModel boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/shell/Settings.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

struct ExportStateHarness {
  std::unique_ptr<tests::support::InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceStore> store;
  std::unique_ptr<WorkspaceCommands> commands;
  std::unique_ptr<WorkspaceSelectors> selectors;
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
  auto* workspacePtr = workspace.get();
  auto store = std::make_unique<WorkspaceStore>();
  store->setWorkspacePorts(workspacePtr, workspacePtr);
  store->loadFromState(workspacePtr->workspaceSnapshot());
  auto commands = std::make_unique<WorkspaceCommands>(*store);
  auto selectors = std::make_unique<WorkspaceSelectors>(*store);

  auto state = std::make_unique<ExportViewModel>();
  state->setWorkspaceRoles(store.get(), commands.get(), selectors.get());
  state->setTargetDirectory(QStringLiteral("test:///export"));

  return {std::move(workspace), std::move(store), std::move(commands),
          std::move(selectors), std::move(state)};
}

QVariantMap firstMap(const QVariantList& values) {
  return values.empty() ? QVariantMap() : values.front().toMap();
}

} // namespace

TEST(ExportViewModelTest, VM_EXPORT_001_LoadsWorkspaceRowsAndSelectsAnnual) {
  auto harness = makeHarness();

  ASSERT_EQ(harness.state->annualRows().size(), 1);
  ASSERT_EQ(harness.state->analysisRows().size(), 2);
  EXPECT_EQ(harness.state->addMode(), QStringLiteral("annual"));
  EXPECT_EQ(harness.state->selectedAddIndex(), 0);
  EXPECT_TRUE(harness.state->canAddEntry());
}

TEST(ExportViewModelTest,
     VM_EXPORT_002_AnnualEntryProjectsAssignedAnalysesForExport) {
  auto harness = makeHarness();

  harness.state->addSelectedEntry();
  const QVariantList entries = harness.state->exportEntries();

  ASSERT_EQ(entries.size(), 1);
  const QVariantMap annual = entries.at(0).toMap();
  EXPECT_TRUE(annual.value(QStringLiteral("isAnnual")).toBool());
  EXPECT_EQ(annual.value(QStringLiteral("objectId")).toString(),
            QStringLiteral("annual-export"));
  const QVariantList analyses =
      annual.value(QStringLiteral("analyses")).toList();
  ASSERT_EQ(analyses.size(), 2);
  EXPECT_EQ(
      analyses.at(1).toMap().value(QStringLiteral("exportType")).toString(),
      QStringLiteral("PNG"));
}

TEST(ExportViewModelTest,
     VM_EXPORT_003_StandalonePlotDefaultsToImageExportOptions) {
  auto harness = makeHarness();

  harness.state->setAddMode(QStringLiteral("analysis"));
  harness.state->selectAddRow(1);
  harness.state->addSelectedEntry();

  const QVariantMap entry = firstMap(harness.state->exportEntries());
  EXPECT_FALSE(entry.value(QStringLiteral("isAnnual")).toBool());
  EXPECT_EQ(entry.value(QStringLiteral("objectId")).toString(),
            QStringLiteral("analysis-plot"));
  EXPECT_EQ(entry.value(QStringLiteral("exportType")).toString(),
            QStringLiteral("PNG"));
  EXPECT_EQ(entry.value(QStringLiteral("exportTypeOptions")).toList().size(),
            2);
}

TEST(ExportViewModelTest,
     VM_EXPORT_004_SettingsDefaultsRefreshPristineExportForm) {
  ExportViewModel state;
  Settings settings;

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
