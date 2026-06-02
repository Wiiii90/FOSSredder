/**
 * @file ui/tests/unit/TestExportWorkflow.cpp
 * @brief Tests for export workflow log restoration and refresh behavior.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/adapters/ExportAdapter.h"
#include "ui/workflows/export/ExportWorkflow.h"

namespace {

core::ports::workspace::WorkspaceSnapshot
makeStateWithExportLog(const QString &id) {
  core::ports::workspace::WorkspaceSnapshot state;
  core::ports::workspace::ExportLogSnapshot log;
  log.id = id.toStdString();
  log.time = "2026-05-15 10:00:00";
  log.targetPath = "/tmp/export.xlsx";
  log.status = "Success";
  log.message = "done";
  log.payload = "{}";
  state.exportLogs.push_back(std::move(log));
  return state;
}

} // namespace

namespace ui {

TEST(ExportWorkflowTest, RestoresPersistedExportLogsFromSnapshotProvider) {
  auto runner = std::make_shared<ui::adapters::ExportAdapter>(nullptr);
  ExportWorkflow workflow([state = makeStateWithExportLog(
                               QStringLiteral("log-1"))]() { return state; },
                          runner, nullptr);

  ASSERT_EQ(workflow.exportLogs().size(), 1U);
  EXPECT_EQ(workflow.exportLogs().at(0).logId, QStringLiteral("log-1"));
  EXPECT_EQ(workflow.exportLogs().at(0).file,
            QStringLiteral("/tmp/export.xlsx"));
  EXPECT_EQ(workflow.exportLogs().at(0).status, QStringLiteral("Success"));
}

TEST(ExportWorkflowTest, RefreshFromStateSnapshotReplacesExportLogs) {
  auto runner = std::make_shared<ui::adapters::ExportAdapter>(nullptr);
  ExportWorkflow workflow([state = makeStateWithExportLog(
                               QStringLiteral("log-1"))]() { return state; },
                          runner, nullptr);

  EXPECT_EQ(workflow.exportLogs().size(), 1U);

  workflow.refreshFromStateSnapshot();
  EXPECT_EQ(workflow.exportLogs().size(), 1U);
  EXPECT_EQ(workflow.exportLogs().at(0).logId, QStringLiteral("log-1"));
}

} // namespace ui
