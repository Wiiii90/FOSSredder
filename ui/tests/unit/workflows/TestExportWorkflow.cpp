/**
 * @file ui/tests/unit/workflows/TestExportWorkflow.cpp
 * @brief Tests for export workflow log publication.
 */

#include <gtest/gtest.h>

#include <vector>

#include <QVariantMap>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workflows/ExportWorkflow.h"

namespace ui {

TEST(ExportWorkflowTest, WF_EXPORT_001_PublishesLogsThroughWorkspaceSink) {
  std::vector<core::ports::workspace::ExportLogSnapshot> logs;
  ExportWorkflow workflow([]() { return core::ports::workspace::WorkspaceSnapshot{}; },
                          nullptr, nullptr);
  workflow.setExportLogSink(
      [&logs](const core::ports::workspace::ExportLogSnapshot &log) {
        logs.push_back(log);
      });

  workflow.exportDataWithPayload(0, QStringLiteral("/tmp/export.xlsx"), true,
                                 QStringLiteral("de-DE"), QVariantMap{}, 1);

  ASSERT_EQ(logs.size(), 2U);
  EXPECT_EQ(logs.front().status, "Running");
  EXPECT_EQ(logs.back().status, "Failed");
  EXPECT_EQ(logs.back().targetPath, "/tmp/export.xlsx");
}

} // namespace ui
