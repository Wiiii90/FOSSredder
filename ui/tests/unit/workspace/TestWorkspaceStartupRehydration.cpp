/**
 * @file ui/tests/unit/workspace/TestWorkspaceStartupRehydration.cpp
 * @brief Tests for startup rehydration of persisted workspace state into
 * UI-facing models.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QVariantList>

#include "core/errors/IErrorReporter.h"
#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceRowProjector.h"
#include "ui/adapters/ExportAdapter.h"
#include "ui/workflows/ExportWorkflow.h"
#include "ui/workflows/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace {

core::ports::workspace::WorkspaceSnapshot makeLoadedWorkspaceSnapshot() {
  auto snapshot = ui::tests::support::makeWorkspaceSnapshot();

  core::ports::workspace::ImportLogSnapshot importLog;
  importLog.id = "import-log-1";
  importLog.time = "2026-05-15T08:00:00Z";
  importLog.type = "statement";
  importLog.file = "/tmp/import.csv";
  importLog.status = "done";
  importLog.message = "Import completed";
  importLog.draftAttached = true;
  importLog.draftId = "statement-draft-1";
  importLog.statementDraftIds = {"statement-draft-1"};
  importLog.statementId = "statement-1";
  snapshot.importLogs.push_back(std::move(importLog));

  core::ports::workspace::ExportLogSnapshot exportLog;
  exportLog.id = "export-log-1";
  exportLog.time = "2026-05-15T09:00:00Z";
  exportLog.targetPath = "/tmp/export.csv";
  exportLog.status = "done";
  exportLog.message = "Export completed";
  exportLog.payload = "{}";
  exportLog.annualIds = {"annual-1"};
  exportLog.analysisIds = {"analysis-1"};
  snapshot.exportLogs.push_back(std::move(exportLog));

  return snapshot;
}

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
  void report(const core::errors::ErrorEvent &) override {}
};

} // namespace

namespace ui {

TEST(WorkspaceStartupRehydrationTest,
     WSP_STARTUP_001_RehydratesWorkspaceAndWorkflowModelsFromLoadedState) {
  tests::support::InMemoryWorkspace workspace(makeLoadedWorkspaceSnapshot());
  workspace.openFile("P:/workspace.db");

  WorkspaceFacade facade(&workspace, &workspace);
  auto &models = facade.cache()->models();

  EXPECT_EQ(facade.actorRows().size(), 1);
  EXPECT_EQ(facade.propertyRows().size(), 1);
  EXPECT_EQ(facade.contractRows().size(), 1);
  EXPECT_EQ(models.statements().rowCount(), 1);
  EXPECT_EQ(models.transactions().rowCount(), 2);
  EXPECT_EQ(models.analyses().rowCount(), 1);
  EXPECT_EQ(models.annuals().rowCount(), 1);
  const QVariantList transactionRows = buildStatementTransactionRows(
      *facade.cache(), QStringLiteral("statement-1"));
  ASSERT_EQ(transactionRows.size(), 2);
  EXPECT_EQ(
      transactionRows.at(0).toMap().value(QStringLiteral("id")).toString(),
      QStringLiteral("tx-1"));
  EXPECT_EQ(
      transactionRows.at(1).toMap().value(QStringLiteral("id")).toString(),
      QStringLiteral("tx-2"));

  ExportWorkflow exportWorkflow(
      [&workspace]() { return workspace.workspaceSnapshot(); },
      std::make_shared<ui::adapters::ExportAdapter>(nullptr));

  exportWorkflow.refreshFromStateSnapshot();

  const QVariantList importLogs = facade.importLogRows();
  EXPECT_EQ(importLogs.size(), 1);
  EXPECT_EQ(exportWorkflow.exportLogs().size(), 1U);
  EXPECT_EQ(importLogs.at(0).toMap().value(QStringLiteral("logId")).toString(),
            QStringLiteral("import-log-1"));
  EXPECT_EQ(exportWorkflow.exportLogs().at(0).logId,
            QStringLiteral("export-log-1"));
}

} // namespace ui
