/**
 * @file ui/tests/unit/viewmodels/TestActorViewModel.cpp
 * @brief Tests for the UI ActorViewModel boundary.
 */

#include <gtest/gtest.h>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/ActorViewModel.h"

namespace ui {

TEST(ActorViewModelTest,
     VM_ACTOR_001_SubmitCreatesActorAndSelectsPersistedEntity) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  ActorViewModel viewModel(harness.store.get(), harness.commands.get(),
                           harness.selection.get(), harness.selectors.get());

  viewModel.enterCreateMode();
  viewModel.setName(QStringLiteral("New Actor"));
  viewModel.addAlias(QStringLiteral("Alias A"));
  viewModel.setContractSelected(QStringLiteral("contract-1"), true);

  const QString id = viewModel.submit();

  ASSERT_FALSE(id.isEmpty());
  EXPECT_EQ(harness.selection->selectedActorId(), id);
  const auto snapshot = harness.workspace->snapshot();
  ASSERT_EQ(snapshot.actors.size(), 2);
  EXPECT_EQ(snapshot.actors.back().name, "New Actor");
  ASSERT_EQ(snapshot.actors.back().aliases.size(), 1);
  EXPECT_EQ(snapshot.actors.back().aliases.front().value, "Alias A");
  ASSERT_EQ(snapshot.actors.back().contractIds.size(), 1);
  EXPECT_EQ(snapshot.actors.back().contractIds.front(), "contract-1");
}

TEST(ActorViewModelTest, VM_ACTOR_002_SelectEditUpdateAndDeleteCurrentActor) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  ActorViewModel viewModel(harness.store.get(), harness.commands.get(),
                           harness.selection.get(), harness.selectors.get());

  viewModel.selectActor(QStringLiteral("actor-1"));
  ASSERT_TRUE(viewModel.isEdit());
  EXPECT_EQ(viewModel.name(), QStringLiteral("Main Actor"));

  viewModel.setName(QStringLiteral("Updated Actor"));
  EXPECT_TRUE(viewModel.hasChanges());
  EXPECT_EQ(viewModel.submit(), QStringLiteral("actor-1"));
  EXPECT_EQ(harness.workspace->snapshot().actors.front().name, "Updated Actor");

  viewModel.deleteCurrent();
  EXPECT_TRUE(harness.workspace->snapshot().actors.empty());
  EXPECT_TRUE(harness.selection->selectedActorId().isEmpty());
}

} // namespace ui
