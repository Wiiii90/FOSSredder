/**
 * @file ui/tests/interaction/TestFeatureWiring.cpp
 * @brief Interaction tests for analysis, annual, and export UI source wiring.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/adapters/ExportAdapter.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/viewmodels/AnnualViewModel.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/workflows/AnalysisWorkflow.h"
#include "ui/workflows/AnnualWorkflow.h"
#include "ui/workflows/ExportWorkflow.h"

namespace ui {
namespace {

class CoreApplicationScope {
public:
  CoreApplicationScope() {
    if (QCoreApplication::instance()) {
      return;
    }
    argv_[0] = arg0_;
    app_ = std::make_unique<QCoreApplication>(argc_, argv_);
  }

private:
  int argc_ = 1;
  char arg0_[32] = "ui_interaction_tests";
  char* argv_[1] = {};
  std::unique_ptr<QCoreApplication> app_;
};

bool waitUntilExportSettles(const ExportWorkflow& workflow) {
  QElapsedTimer timer;
  timer.start();
  while (workflow.isRunning() && timer.elapsed() < 5000) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QThread::msleep(10);
  }
  QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  return !workflow.isRunning();
}

} // namespace

TEST(
    FeatureWiringInteractionTest,
    INTERACTION_ANALYSIS_001_ViewModelCreatesAnalysisThroughWorkflowAndWorkspace) {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  snapshot.analyses.clear();
  tests::support::WorkspaceHarness harness(std::move(snapshot));

  auto adapter = std::make_shared<adapters::AnalysisAdapter>(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  AnalysisWorkflow workflow(
      [&harness]() {
        return harness.workspace->workspaceSnapshot();
      },
      adapter, nullptr);
  AnalysisViewModel viewModel;
  viewModel.setWorkspace(harness.facade.get());
  viewModel.setAnalysisWorkflow(&workflow);

  viewModel.setName(QStringLiteral("Interaction Analysis"));
  viewModel.setMainTypeIndex(0);
  viewModel.submitCreate();

  const QString selectedId = harness.facade->selection()->selectedAnalysisId();
  ASSERT_FALSE(selectedId.isEmpty());
  const auto workspace = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(workspace.analyses.size(), 1U);
  EXPECT_EQ(workspace.analyses.front().name,
            std::string("Interaction Analysis"));
  EXPECT_FALSE(viewModel.previewTransactionRows().isEmpty());
}

TEST(FeatureWiringInteractionTest,
     INTERACTION_ANNUAL_001_ViewModelCreatesAnnualThroughWorkflowAndWorkspace) {
  auto snapshot = tests::support::makeWorkspaceSnapshot();
  auto plot = tests::support::makeAnalysis();
  plot.id = "analysis-plot";
  plot.name = "Plot";
  plot.type = "plot";
  snapshot.analyses.push_back(plot);
  snapshot.annuals.clear();
  tests::support::WorkspaceHarness harness(std::move(snapshot));

  auto adapter = std::make_shared<adapters::AnnualAdapter>(
      std::make_shared<tests::support::FakeAnnualRunner>());
  AnnualWorkflow workflow(
      [&harness]() {
        return harness.workspace->workspaceSnapshot();
      },
      adapter);
  AnnualViewModel viewModel;
  viewModel.setWorkspace(harness.facade.get());
  viewModel.setAnnualWorkflow(&workflow);

  viewModel.setName(QStringLiteral("Interaction Annual"));
  viewModel.setYear(2026);
  viewModel.addAvailableAnalysisAtIndex(0);
  viewModel.submitCreate();

  const QString selectedId = harness.facade->selection()->selectedAnnualId();
  ASSERT_FALSE(selectedId.isEmpty());
  const auto workspace = harness.workspace->workspaceSnapshot();
  ASSERT_EQ(workspace.annuals.size(), 1U);
  EXPECT_EQ(workspace.annuals.front().name, std::string("Interaction Annual"));
  ASSERT_EQ(workspace.annuals.front().analysisIds.size(), 1U);
  EXPECT_FALSE(viewModel.statusSummaryText().isEmpty());
}

TEST(
    FeatureWiringInteractionTest,
    INTERACTION_EXPORT_001_ViewModelRunsExportThroughWorkflowAdapterAndLogSink) {
  CoreApplicationScope app;
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());

  auto adapter = std::make_shared<adapters::ExportAdapter>(
      std::make_shared<tests::support::ExportRunnerStub>());
  ExportWorkflow workflow(
      [&harness]() {
        return harness.workspace->workspaceSnapshot();
      },
      adapter);
  ExportViewModel viewModel;
  viewModel.setWorkspace(harness.facade.get());
  viewModel.setExportWorkflow(&workflow);
  viewModel.setTargetDirectory(QStringLiteral("P:/exports/out.xlsx"));

  ASSERT_TRUE(viewModel.canAddEntry());
  viewModel.addSelectedEntry();
  ASSERT_TRUE(viewModel.canStart());
  viewModel.startExport();

  ASSERT_TRUE(waitUntilExportSettles(workflow));
  EXPECT_FALSE(workflow.isRunning());
  EXPECT_EQ(workflow.progress(), 1.0);
  ASSERT_FALSE(viewModel.exportLogs().isEmpty());
  EXPECT_EQ(viewModel.exportLogs()
                .front()
                .toMap()
                .value(QStringLiteral("status"))
                .toString(),
            QStringLiteral("Success"));
}

} // namespace ui
