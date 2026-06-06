/**
 * @file ui/tests/unit/viewmodels/TestPropertyViewModel.cpp
 * @brief Tests for the UI PropertyViewModel boundary.
 */

#include <gtest/gtest.h>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/PropertyViewModel.h"

namespace ui {

TEST(PropertyViewModelTest,
     VM_PROPERTY_001_SubmitCreatesPropertyAndPersistsContracts) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  PropertyViewModel viewModel(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  viewModel.enterCreateMode();
  viewModel.setName(QStringLiteral("New Property"));
  viewModel.addAlias(QStringLiteral("Property Alias"));
  viewModel.setContractSelected(QStringLiteral("contract-1"), true);

  const QString id = viewModel.submit();

  ASSERT_FALSE(id.isEmpty());
  EXPECT_EQ(harness.selection->selectedPropertyId(), id);
  const auto snapshot = harness.workspace->snapshot();
  ASSERT_EQ(snapshot.properties.size(), 2);
  EXPECT_EQ(snapshot.properties.back().name, "New Property");
  ASSERT_EQ(snapshot.properties.back().contractIds.size(), 1);
  EXPECT_EQ(snapshot.properties.back().contractIds.front(), "contract-1");
}

TEST(PropertyViewModelTest,
     VM_PROPERTY_002_SelectEditUpdateAndDeleteCurrentProperty) {
  tests::support::WorkspaceHarness harness(
      tests::support::makeWorkspaceSnapshot());
  PropertyViewModel viewModel(harness.store.get(), harness.commands.get(),
                              harness.selection.get(), harness.selectors.get());

  viewModel.selectProperty(QStringLiteral("property-1"));
  ASSERT_TRUE(viewModel.isEdit());
  EXPECT_EQ(viewModel.name(), QStringLiteral("Primary Property"));

  viewModel.setName(QStringLiteral("Updated Property"));
  EXPECT_TRUE(viewModel.hasChanges());
  EXPECT_EQ(viewModel.submit(), QStringLiteral("property-1"));
  EXPECT_EQ(harness.workspace->snapshot().properties.front().name,
            "Updated Property");

  viewModel.deleteCurrent();
  EXPECT_TRUE(harness.workspace->snapshot().properties.empty());
  EXPECT_TRUE(harness.selection->selectedPropertyId().isEmpty());
}

} // namespace ui
