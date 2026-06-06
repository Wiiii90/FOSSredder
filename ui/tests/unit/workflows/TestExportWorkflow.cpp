/**
 * @file ui/tests/unit/workflows/TestExportWorkflow.cpp
 * @brief Tests for export workflow log publication.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QVariantMap>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "support/WorkspacePortFakes.h"
#include "ui/adapters/ExportAdapter.h"
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
  char arg0_[32] = "export_workflow_tests";
  char* argv_[1] = {};
  std::unique_ptr<QCoreApplication> app_;
};

bool waitUntilSettled(const ExportWorkflow& workflow) {
  QElapsedTimer timer;
  timer.start();
  while (workflow.isRunning() && timer.elapsed() < 5000) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QThread::msleep(5);
  }
  QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  return !workflow.isRunning();
}

class CancelAwareExportRunner final
    : public core::ports::exporting::IExportRunner {
public:
  [[nodiscard]] core::ports::exporting::ExportResult
  runExport(const core::ports::workspace::WorkspaceSnapshot&,
            core::ports::exporting::ExportRequest request) const override {
    for (int i = 0; i < 20; ++i) {
      if (request.shouldCancel && request.shouldCancel()) {
        return {false,
                core::ports::exporting::ExportStatus::Canceled,
                request.format,
                request.outputPath,
                "EXPORT_CANCELED",
                "Canceled"};
      }
      if (request.waitIfPaused) {
        request.waitIfPaused();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return {true,
            core::ports::exporting::ExportStatus::Ok,
            request.format,
            request.outputPath,
            {},
            {}};
  }
};

} // namespace

TEST(ExportWorkflowTest, WF_EXPORT_001_PublishesLogsThroughWorkspaceSink) {
  std::vector<core::ports::workspace::ExportLogSnapshot> logs;
  ExportWorkflow workflow(
      []() {
        return core::ports::workspace::WorkspaceSnapshot{};
      },
      nullptr, nullptr);
  workflow.setExportLogSink(
      [&logs](const core::ports::workspace::ExportLogSnapshot& log) {
        logs.push_back(log);
      });

  workflow.exportDataWithPayload(0, QStringLiteral("/tmp/export.xlsx"), true,
                                 QStringLiteral("de-DE"), QVariantMap{}, 1);

  ASSERT_EQ(logs.size(), 2U);
  EXPECT_EQ(logs.front().status, "Running");
  EXPECT_EQ(logs.back().status, "Failed");
  EXPECT_EQ(logs.back().targetPath, "/tmp/export.xlsx");
}

TEST(ExportWorkflowTest,
     WF_EXPORT_002_SuccessfulExportPublishesSuccessLogAndResetsState) {
  CoreApplicationScope app;
  std::vector<core::ports::workspace::ExportLogSnapshot> logs;
  const auto adapter = std::make_shared<adapters::ExportAdapter>(
      std::make_shared<tests::support::ExportRunnerStub>());
  ExportWorkflow workflow(
      []() {
        return core::ports::workspace::WorkspaceSnapshot{};
      },
      adapter, nullptr);
  workflow.setExportLogSink(
      [&logs](const core::ports::workspace::ExportLogSnapshot& log) {
        logs.push_back(log);
      });

  workflow.exportDataWithPayload(0, QStringLiteral("P:/exports/out.xlsx"), true,
                                 QStringLiteral("de-DE"), QVariantMap{}, 1);

  ASSERT_TRUE(waitUntilSettled(workflow));
  ASSERT_GE(logs.size(), 2U);
  EXPECT_FALSE(workflow.isRunning());
  EXPECT_FALSE(workflow.isPaused());
  EXPECT_EQ(workflow.progress(), 1.0);
  EXPECT_EQ(logs.back().status, "Success");
  EXPECT_EQ(logs.back().targetPath, "P:/exports/out.xlsx");
}

TEST(ExportWorkflowTest,
     WF_EXPORT_003_CancelRequestsArePropagatedAndPublishCanceledLog) {
  CoreApplicationScope app;
  std::vector<core::ports::workspace::ExportLogSnapshot> logs;
  const auto adapter = std::make_shared<adapters::ExportAdapter>(
      std::make_shared<CancelAwareExportRunner>());
  ExportWorkflow workflow(
      []() {
        return core::ports::workspace::WorkspaceSnapshot{};
      },
      adapter, nullptr);
  workflow.setExportLogSink(
      [&logs](const core::ports::workspace::ExportLogSnapshot& log) {
        logs.push_back(log);
      });

  workflow.exportDataWithPayload(0, QStringLiteral("P:/exports/cancel.xlsx"),
                                 true, QStringLiteral("de-DE"), QVariantMap{},
                                 1);
  ASSERT_TRUE(workflow.isRunning());

  workflow.cancelExport();

  ASSERT_TRUE(waitUntilSettled(workflow));
  ASSERT_GE(logs.size(), 2U);
  EXPECT_FALSE(workflow.isRunning());
  EXPECT_FALSE(workflow.isPaused());
  EXPECT_EQ(workflow.progress(), 0.0);
  EXPECT_EQ(logs.back().status, "Canceled");
}

TEST(ExportWorkflowTest, WF_EXPORT_004_PauseAndResumeUpdateWorkflowState) {
  CoreApplicationScope app;
  const auto adapter = std::make_shared<adapters::ExportAdapter>(
      std::make_shared<CancelAwareExportRunner>());
  ExportWorkflow workflow(
      []() {
        return core::ports::workspace::WorkspaceSnapshot{};
      },
      adapter, nullptr);

  workflow.exportDataWithPayload(0, QStringLiteral("P:/exports/pause.xlsx"),
                                 true, QStringLiteral("de-DE"), QVariantMap{},
                                 1);
  ASSERT_TRUE(workflow.isRunning());

  workflow.pauseExport();
  EXPECT_TRUE(workflow.isPaused());
  EXPECT_EQ(workflow.currentMode(), ExportWorkflow::ProgressMode);

  workflow.resumeExport();
  EXPECT_FALSE(workflow.isPaused());

  workflow.cancelExport();
  EXPECT_TRUE(waitUntilSettled(workflow));
}

} // namespace ui
