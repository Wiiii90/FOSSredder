/**
 * @file ui/tests/interaction/TestImportState.cpp
 * @brief Smoke tests for the import state workflow.
 */

#include <gtest/gtest.h>

#include <QRegularExpression>

#include "core/application/import/ImportLog.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/errors/IErrorReporter.h"
#include "ui/state/import/ImportState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workflows/import/ImportWorkflowState.h"

namespace ui {

namespace {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

} // namespace

TEST(ImportStateTest, HeaderIsUsable) {
    SUCCEED();
}

TEST(ImportStateTest, OverviewStateAppliesDefaultPathAndFiltersManualFiles)
{
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        std::make_shared<NoopErrorReporter>());
    SettingsViewModel settings;
    ImportState state;

    settings.setImportDefaultPath(QStringLiteral("P:/imports/default.pdf"));
    state.setSettingsViewModel(&settings);
    state.setImportWorkflow(&workflow);

    EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/default.pdf"));
    EXPECT_TRUE(state.canStart());

    settings.setImportDefaultPath(QStringLiteral("P:/imports/updated.pdf"));
    EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/updated.pdf"));

    settings.setImportDefaultPath({});
    workflow.setSelectedFile({});
    state.setManualPathText(QStringLiteral("P:/imports/readme.txt"));
    state.commitManualImportFiles();

    EXPECT_TRUE(workflow.selectedFile().isEmpty());
    EXPECT_TRUE(workflow.queuedFiles().isEmpty());

    state.setManualPathText(QStringLiteral("P:/imports/statement.PDF"));
    state.commitManualImportFiles();

    EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/statement.PDF"));
    EXPECT_EQ(state.importFileSummary(), QStringLiteral("Selected: statement.PDF"));
    EXPECT_TRUE(state.manualPathText().isEmpty());

    settings.setImportDefaultPath(QStringLiteral("P:/imports/later-default.pdf"));
    EXPECT_EQ(workflow.selectedFile(), QStringLiteral("P:/imports/statement.PDF"));
}

TEST(ImportStateTest, TogglePauseGatesProgressUpdates)
{
    importing::ImportWorkflowState state;
    state.beginImport(QStringLiteral("statement.pdf"));

    EXPECT_TRUE(state.togglePause());
    EXPECT_TRUE(state.isPaused());
    EXPECT_EQ(state.phase(), QStringLiteral("Paused"));

    state.updateProgress(0.5, QStringLiteral("halfway"), QRegularExpression{});
    EXPECT_DOUBLE_EQ(state.progress(), 0.01);
    EXPECT_EQ(state.phase(), QStringLiteral("Paused"));

    EXPECT_TRUE(state.togglePause());
    EXPECT_FALSE(state.isPaused());
    EXPECT_EQ(state.phase(), QStringLiteral("Running import..."));

    state.updateProgress(0.5, QStringLiteral("halfway"), QRegularExpression{});
    EXPECT_DOUBLE_EQ(state.progress(), 0.5);
    EXPECT_EQ(state.phase(), QStringLiteral("halfway"));
}

TEST(ImportStateTest, InterleavedRunsKeepFinalizeMappedToContextDraft)
{
    core::application::workspace::WorkspaceSessionState snapshot;

    auto first = std::make_shared<core::application::importing::ImportLog>();
    first->id = "draft-1";
    first->time = "2026-05-15 10:00:00";
    first->type = "statement";
    first->file = "/tmp/Januar 2025.pdf";
    first->status = "Draft";
    first->message = "Draft ready";
    first->draftAttached = true;
    first->draftId = "draft-1";
    snapshot.workflow.importLogs.push_back(first);

    auto second = std::make_shared<core::application::importing::ImportLog>();
    second->id = "draft-2";
    second->time = "2026-05-15 11:00:00";
    second->type = "statement";
    second->file = "/tmp/Maerz 2025.pdf";
    second->status = "Draft";
    second->message = "Draft ready";
    second->draftAttached = true;
    second->draftId = "draft-2";
    snapshot.workflow.importLogs.push_back(second);

    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        std::make_shared<NoopErrorReporter>());

    workflow.setStateSnapshotProvider([snapshot]() { return snapshot; });
    ASSERT_NE(workflow.runs(), nullptr);
    ASSERT_EQ(workflow.runs()->rowCount(), 2);

    // Stress-like interleaving: switch active runs and write mixed notes.
    workflow.activateRunAt(0);
    workflow.addRunNote(QStringLiteral("Paused"), QStringLiteral("Import paused."),
                        false);
    workflow.addRunNote(QStringLiteral("Canceled"), QStringLiteral("Import canceled."),
                        false);

    workflow.activateRunAt(1);
    workflow.addRunNote(QStringLiteral("Draft"), QStringLiteral("Draft paused."),
                        true, {}, QStringLiteral("draft-2"));

    workflow.activateRunAt(0);
    workflow.addRunNote(QStringLiteral("Finalized"),
                        QStringLiteral("Draft was finalized into a statement."),
                        false, QStringLiteral("statement-2"),
                        QStringLiteral("draft-2"));

    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("draft-1"));
    EXPECT_NE(workflow.runs()->at(0).status, QStringLiteral("Finalized"));
    EXPECT_TRUE(workflow.runs()->at(0).statementId.isEmpty());

    EXPECT_EQ(workflow.runs()->at(1).logId, QStringLiteral("draft-2"));
    EXPECT_EQ(workflow.runs()->at(1).status, QStringLiteral("Finalized"));
    EXPECT_EQ(workflow.runs()->at(1).statementId, QStringLiteral("statement-2"));
}

} // namespace ui
