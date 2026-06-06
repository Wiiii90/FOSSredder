/**
 * @file ui/tests/unit/workflows/TestAnnualWorkflow.cpp
 * @brief Tests for the UI annual workflow boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/workflows/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

core::ports::workspace::WorkspaceSnapshot makeAnnualSnapshot() {
  auto tabular = tests::support::makeAnalysis();
  tabular.id = "analysis-table";
  tabular.name = "Table";
  tabular.type = "tabular";
  tabular.snapshotTransactions = {tests::support::makeTransaction(
      "tx-1", "Rent", "2026-01-05", 1250.0, "statement-1", true, "contract-1",
      {}, {"property-1"})};

  auto plot = tests::support::makeAnalysis();
  plot.id = "analysis-plot";
  plot.name = "Plot";
  plot.type = "plot";
  plot.snapshotTransactions = {tests::support::makeTransaction(
      "tx-2", "Fees", "2026-01-06", -35.5, "statement-1", false, {}, {},
      {"property-1"})};

  auto annual = tests::support::makeAnnual();
  annual.analysisIds = {"analysis-table"};

  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.analyses = {tabular, plot};
  snapshot.annuals = {annual};
  return snapshot;
}

} // namespace

TEST(AnnualWorkflowTest,
     WF_ANNUAL_001_ComputeAnnualUsesStoredAnnualAssignments) {
  const auto snapshot = makeAnnualSnapshot();
  auto annualAdapter = std::make_shared<ui::adapters::AnnualAdapter>(
      std::make_shared<tests::support::FakeAnnualRunner>());
  AnnualWorkflow workflow(
      [snapshot]() {
        return snapshot;
      },
      annualAdapter);

  const QVariantMap result = workflow.computeAnnualPreview(
      QStringLiteral("annual-1"), {QStringLiteral("analysis-table")}, 2026);
  const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();

  EXPECT_EQ(result.value(QStringLiteral("annualId")).toString(),
            QStringLiteral("annual-1"));
  EXPECT_EQ(stats.value(QStringLiteral("assignedAnalysisCount")).toInt(), 1);
  EXPECT_EQ(result.value(QStringLiteral("deduplicated")).toList().size(), 1);
  EXPECT_EQ(result.value(QStringLiteral("workspaceOnly")).toList().size(), 1);
}

TEST(AnnualWorkflowTest,
     WF_ANNUAL_002_ComputeAnnualPreviewUsesInjectedAnalysisIdsAndYear) {
  const auto snapshot = makeAnnualSnapshot();
  auto annualAdapter = std::make_shared<ui::adapters::AnnualAdapter>(
      std::make_shared<tests::support::FakeAnnualRunner>());
  AnnualWorkflow workflow(
      [snapshot]() {
        return snapshot;
      },
      annualAdapter);

  const QVariantMap result = workflow.computeAnnualPreview(
      QStringLiteral("annual-1"),
      {QStringLiteral("analysis-table"), QStringLiteral("analysis-plot")},
      2026);
  const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();

  EXPECT_EQ(stats.value(QStringLiteral("assignedAnalysisCount")).toInt(), 2);
  EXPECT_EQ(result.value(QStringLiteral("divergent")).toList().size(), 2);
  EXPECT_EQ(result.value(QStringLiteral("workspaceOnly")).toList().size(), 0);
}

TEST(AnnualWorkflowTest, WF_ANNUAL_003_WorkspaceCommandsSaveAndDeleteAnnual) {
  tests::support::InMemoryWorkspace workspace(makeAnnualSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  const QString createdId = commands.addAnnual(
      QStringLiteral("Created Annual"), 2026,
      {QStringLiteral("analysis-table"), QStringLiteral("analysis-plot")});
  ASSERT_FALSE(createdId.isEmpty());
  auto snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.annuals.size(), 2U);
  EXPECT_EQ(snapshot.annuals.back().name, std::string("Created Annual"));
  ASSERT_EQ(snapshot.annuals.back().analysisIds.size(), 2U);

  commands.updateAnnual(createdId, QStringLiteral("Updated Annual"), 2027,
                        {QStringLiteral("analysis-table")});
  snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.annuals.size(), 2U);
  EXPECT_EQ(snapshot.annuals.back().name, std::string("Updated Annual"));
  EXPECT_EQ(snapshot.annuals.back().year, 2027);
  ASSERT_EQ(snapshot.annuals.back().analysisIds.size(), 1U);

  commands.deleteAnnual(createdId);
  snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.annuals.size(), 1U);
  EXPECT_EQ(snapshot.annuals.front().id, std::string("annual-1"));
}

TEST(
    AnnualWorkflowTest,
    WF_ANNUAL_004_WorkspaceCommandsPreserveAnalysisPayloadWhenExportFormatChanges) {
  tests::support::InMemoryWorkspace workspace(makeAnnualSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  commands.updateAnalysisExportFormat(QStringLiteral("analysis-plot"),
                                      QStringLiteral("jpg"));

  const auto snapshot = workspace.workspaceSnapshot();
  ASSERT_EQ(snapshot.analyses.size(), 2U);
  EXPECT_EQ(snapshot.analyses.at(1).exportFormat, std::string("jpg"));
  EXPECT_EQ(snapshot.analyses.at(1).name, std::string("Plot"));
  EXPECT_EQ(snapshot.analyses.at(1).type, std::string("plot"));
}

} // namespace ui
