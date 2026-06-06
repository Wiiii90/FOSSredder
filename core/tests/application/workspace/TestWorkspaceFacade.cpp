/**
 * @file core/tests/application/workspace/TestWorkspaceFacade.cpp
 * @brief Tests for the workspace facade boundary.
 */

#include <gtest/gtest.h>

#include "core/application/workspace/WorkspaceFacade.h"
#include "core/domain/entities/Actor.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceFacadeTest, ProjectsSnapshotsAndRoutesCatalogCommands) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceFacade facade(std::move(storage));

    std::size_t snapshotCount = 0;
    core::ports::workspace::WorkspaceSnapshot lastSnapshot;
    facade.setSnapshotChangedCallback([&](const core::ports::workspace::WorkspaceSnapshot& snapshot) {
        ++snapshotCount;
        lastSnapshot = snapshot;
    });

    facade.newFile("P:/workspace.db");
    EXPECT_EQ(snapshotCount, 1u);
    EXPECT_TRUE(lastSnapshot.hasCurrentPath);
    EXPECT_EQ(lastSnapshot.currentPath, "P:/workspace.db");
    EXPECT_TRUE(lastSnapshot.actors.empty());
    EXPECT_TRUE(lastSnapshot.properties.empty());
    EXPECT_TRUE(lastSnapshot.contracts.empty());
    EXPECT_TRUE(lastSnapshot.statements.empty());
    EXPECT_TRUE(lastSnapshot.transactions.empty());

    core::ports::workspace::ActorCommand addCommand;
    addCommand.name = "Alpha";
    const auto actorId = facade.addActor(addCommand);

    EXPECT_FALSE(actorId.empty());
    EXPECT_EQ(snapshotCount, 2u);
    EXPECT_EQ(facade.workspaceSnapshot().actors.size(), 1u);
    EXPECT_TRUE(storagePtr->savedState_.catalog.actors().empty());

    core::ports::workspace::ActorCommand updateCommand;
    updateCommand.id = actorId;
    updateCommand.name = "Alpha Updated";
    facade.updateActor(updateCommand);

    EXPECT_EQ(snapshotCount, 3u);
    ASSERT_EQ(facade.workspaceSnapshot().actors.size(), 1u);
    EXPECT_EQ(facade.workspaceSnapshot().actors.front().name, "Alpha Updated");

    facade.deleteActor(actorId);

    EXPECT_EQ(snapshotCount, 4u);
    EXPECT_TRUE(facade.workspaceSnapshot().actors.empty());

    facade.saveFile();
    EXPECT_TRUE(storagePtr->savedState_.catalog.actors().empty());
}

} // namespace core::application
