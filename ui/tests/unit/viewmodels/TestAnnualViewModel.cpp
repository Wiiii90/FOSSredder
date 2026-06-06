/**
 * @file ui/tests/unit/viewmodels/TestAnnualViewModel.cpp
 * @brief Tests for the UI AnnualViewModel boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/viewmodels/AnnualViewModel.h"
#include "ui/workflows/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

struct AnnualStateHarness {
  std::unique_ptr<tests::support::InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceStore> store;
  std::unique_ptr<WorkspaceCommands> commands;
  std::unique_ptr<WorkspaceSelectors> selectors;
  std::unique_ptr<WorkspaceSelection> selection;
  std::unique_ptr<AnnualWorkflow> workflow;
  std::unique_ptr<AnnualViewModel> state;
};

auto makeAnnualSnapshot() {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  auto tabular = tests::support::makeAnalysis();
  tabular.id = "analysis-table";
  tabular.name = "Table";
  tabular.type = "tabular";
  tabular.exportFormat = "csv";
  tabular.snapshotTransactions = {tests::support::makeTransaction(
      "tx-1", "Rent", "2026-01-05", 1250.0, "statement-1", true, "contract-1",
      {}, {"property-1"})};

  auto plot = tests::support::makeAnalysis();
  plot.id = "analysis-plot";
  plot.name = "Plot";
  plot.type = "plot";
  plot.exportFormat = "png";
  plot.snapshotTransactions = {tests::support::makeTransaction(
      "tx-2", "Fees", "2026-01-06", -35.5, "statement-1", false, {}, {},
      {"property-1"})};

  auto annual = tests::support::makeAnnual();
  annual.name = "Annual 2026";
  annual.analysisIds = {"analysis-table"};

  snapshot.analyses = {tabular, plot};
  snapshot.annuals = {annual};
  return snapshot;
}

AnnualStateHarness makeHarness(bool selectAnnual) {
  auto snapshot = makeAnnualSnapshot();
  if (!selectAnnual) {
    snapshot.annuals.clear();
  }
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
  selection->setSelectedAnnualId(selectAnnual ? QStringLiteral("annual-1")
                                              : QString());

  auto annualAdapter = std::make_shared<ui::adapters::AnnualAdapter>(
      std::make_shared<tests::support::FakeAnnualRunner>());
  auto workflow = std::make_unique<AnnualWorkflow>(
      [workspacePtr]() {
        return workspacePtr->workspaceSnapshot();
      },
      annualAdapter);

  auto state = std::make_unique<AnnualViewModel>();
  state->setWorkspaceRoles(store.get(), commands.get(), selection.get(),
                           selectors.get());
  state->setAnnualWorkflow(workflow.get());

  return {std::move(workspace), std::move(store),     std::move(commands),
          std::move(selectors), std::move(selection), std::move(workflow),
          std::move(state)};
}

bool sectionVisible(const QVariantList& sections, const QString& key) {
  for (const QVariant& value : sections) {
    const QVariantMap section = value.toMap();
    if (section.value(QStringLiteral("key")).toString() == key) {
      return section.value(QStringLiteral("visible")).toBool();
    }
  }
  return false;
}

} // namespace

TEST(AnnualViewModelTest, VM_ANNUAL_001_LoadsSelectedAnnualAndDerivedPanels) {
  auto harness = makeHarness(true);

  EXPECT_TRUE(harness.state->isEdit());
  EXPECT_EQ(harness.state->name(), QStringLiteral("Annual 2026"));
  EXPECT_EQ(harness.state->year(), 2026);
  EXPECT_EQ(harness.state->assignedAnalysisRows().size(), 1);
  EXPECT_EQ(harness.state->availableAnalysisRows().size(), 1);
  EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                             QStringLiteral("deduplicated")));
  EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                             QStringLiteral("workspaceOnly")));
}

TEST(AnnualViewModelTest,
     VM_ANNUAL_002_AssignmentChangesUpdatePreviewAndPersistAnnual) {
  auto harness = makeHarness(true);

  harness.state->addAvailableAnalysisAtIndex(0);

  EXPECT_TRUE(harness.state->hasChanges());
  EXPECT_EQ(harness.state->analysisIds().size(), 2);
  EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                             QStringLiteral("divergent")));
  EXPECT_FALSE(sectionVisible(harness.state->transactionSections(),
                              QStringLiteral("workspaceOnly")));

  harness.state->submitUpdate();
  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.annuals.size(), 1U);
  ASSERT_EQ(snapshot.annuals.front().analysisIds.size(), 2U);
  EXPECT_EQ(snapshot.annuals.front().analysisIds.at(1),
            std::string("analysis-plot"));
}

TEST(AnnualViewModelTest,
     VM_ANNUAL_003_CreateStatePersistsNewAnnualThroughWorkspace) {
  auto harness = makeHarness(false);

  harness.state->setName(QStringLiteral("Created Annual"));
  harness.state->setYear(2026);
  harness.state->addAvailableAnalysisAtIndex(0);
  harness.state->submitCreate();

  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.annuals.size(), 1U);
  EXPECT_EQ(snapshot.annuals.front().name, std::string("Created Annual"));
  EXPECT_EQ(snapshot.annuals.front().year, 2026);
  ASSERT_EQ(snapshot.annuals.front().analysisIds.size(), 1U);
  EXPECT_EQ(snapshot.annuals.front().analysisIds.front(),
            std::string("analysis-table"));
  EXPECT_FALSE(harness.selection->selectedAnnualId().isEmpty());
}

TEST(AnnualViewModelTest,
     VM_ANNUAL_004_ExportFormatChangesRouteThroughWorkspace) {
  auto harness = makeHarness(true);

  harness.state->setAnalysisExportFormat(QStringLiteral("analysis-plot"),
                                         QStringLiteral("JPG"));

  const auto snapshot = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 2U);
  EXPECT_EQ(snapshot.analyses.at(1).exportFormat, std::string("jpg"));
  EXPECT_TRUE(harness.state->hasChanges());
}

} // namespace ui
