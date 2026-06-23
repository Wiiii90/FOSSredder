/**
 * @file ui/tests/unit/workspace/TestWorkspaceCommands.cpp
 * @brief Tests for UI workspace command routing.
 */

#include <gtest/gtest.h>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

TEST(WorkspaceCommandsTest,
     WSP_COMMANDS_001_SaveActorRoutesThroughWriterAndRefreshesStore) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  const QString actorId = commands.saveActor({}, QStringLiteral("New Actor"),
                                             {QStringLiteral("Alias A")});

  ASSERT_FALSE(actorId.isEmpty());
  ASSERT_EQ(store.snapshot().actors.size(), 2U);
  EXPECT_EQ(store.snapshot().actors.back().name, std::string("New Actor"));
  EXPECT_EQ(store.snapshot().actors.back().aliases.front().value,
            std::string("Alias A"));
}

TEST(WorkspaceCommandsTest,
     WSP_COMMANDS_002_ImportAndExportLogsArePersistedThroughWorkspace) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  commands.upsertImportLog(
      QStringLiteral("import-log-1"), QStringLiteral("Draft"),
      QStringLiteral("Review required"), true, QStringLiteral("draft-1"), {},
      QStringLiteral("statement.pdf"));

  core::ports::workspace::ExportLogSnapshot exportLog;
  exportLog.id = "export-log-1";
  exportLog.status = "Success";
  exportLog.targetPath = "P:/exports/out.xlsx";
  commands.saveExportLog(exportLog);

  ASSERT_EQ(store.snapshot().importLogs.size(), 1U);
  EXPECT_EQ(store.snapshot().importLogs.front().draftId,
            std::string("draft-1"));
  ASSERT_EQ(store.snapshot().exportLogs.size(), 1U);
  EXPECT_EQ(store.snapshot().exportLogs.front().targetPath,
            std::string("P:/exports/out.xlsx"));

  commands.deleteImportLog(QStringLiteral("import-log-1"));
  commands.deleteExportLog(QStringLiteral("export-log-1"));

  EXPECT_TRUE(store.snapshot().importLogs.empty());
  EXPECT_TRUE(store.snapshot().exportLogs.empty());
}

TEST(WorkspaceCommandsTest,
     WSP_COMMANDS_003_BeforeStorageSaveCallbackRunsBeforeSave) {
  tests::support::InMemoryWorkspace workspace(
      tests::support::makeWorkspaceSnapshot());
  WorkspaceStore store;
  store.setWorkspacePorts(&workspace, &workspace);
  WorkspaceCommands commands(store);

  bool callbackCalled = false;
  commands.setBeforeStorageSaveCallback([&callbackCalled]() {
    callbackCalled = true;
  });

  commands.saveFile();

  EXPECT_TRUE(callbackCalled);
  EXPECT_EQ(workspace.savedSnapshot().actors.size(), 1U);
}

} // namespace ui
