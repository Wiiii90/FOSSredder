/**
 * @file ui/tests/unit/viewmodels/TestContractViewModel.cpp
 * @brief Tests for the UI ContractViewModel boundary.
 */

#include <gtest/gtest.h>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/ContractViewModel.h"

namespace ui {

TEST(ContractViewModelTest,
     VM_CONTRACT_001_SubmitCreatesContractWithActorPropertyAndMode) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  ContractViewModel viewModel(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  viewModel.enterCreateMode();
  viewModel.setName(QStringLiteral("New Contract"));
  viewModel.setType(QStringLiteral("service"));
  viewModel.setAllocatableMode(QStringLiteral("full"));
  viewModel.selectPrimaryActor(QStringLiteral("actor-1"));
  viewModel.setPropertySelected(QStringLiteral("property-1"), true);
  viewModel.addAlias(QStringLiteral("Service Alias"));

  const QString id = viewModel.submit();

  ASSERT_FALSE(id.isEmpty());
  EXPECT_EQ(harness.selection->selectedContractId(), id);
  const auto snapshot = harness.workspace->snapshot();
  ASSERT_EQ(snapshot.contracts.size(), 2);
  EXPECT_EQ(snapshot.contracts.back().name, "New Contract");
  EXPECT_EQ(snapshot.contracts.back().type, "service");
  EXPECT_EQ(snapshot.contracts.back().allocatableMode, "full");
  ASSERT_EQ(snapshot.contracts.back().actorIds.size(), 1);
  ASSERT_EQ(snapshot.contracts.back().propertyIds.size(), 1);
}

TEST(ContractViewModelTest,
     VM_CONTRACT_002_SelectEditUpdateAndDeleteCurrentContract) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  ContractViewModel viewModel(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  viewModel.selectContract(QStringLiteral("contract-1"));
  ASSERT_TRUE(viewModel.isEdit());
  viewModel.setType(QStringLiteral("updated-type"));
  viewModel.setPropertySelected(QStringLiteral("property-1"), false);
  viewModel.setPropertySelected(QStringLiteral("property-1"), true);

  EXPECT_EQ(viewModel.submit(), QStringLiteral("contract-1"));
  EXPECT_EQ(harness.workspace->snapshot().contracts.front().type,
            "updated-type");

  viewModel.deleteCurrent();
  EXPECT_TRUE(harness.workspace->snapshot().contracts.empty());
  EXPECT_TRUE(harness.selection->selectedContractId().isEmpty());
}

} // namespace ui
