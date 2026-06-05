/**
 * @file ui/tests/unit/viewmodels/TestBookingViewModel.cpp
 * @brief Tests for the UI BookingViewModel boundary.
 */

#include <gtest/gtest.h>

#include "support/ViewModelTestHarness.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/BookingViewModel.h"

namespace ui {

TEST(BookingViewModelTest,
     VM_BOOKING_001_SubmitCreatesStatementAndTransaction) {
  tests::support::WorkspaceHarness harness;
  BookingViewModel viewModel(harness.store.get(), harness.commands.get(),
                             harness.selection.get(), harness.selectors.get());

  viewModel.setStatementName(QStringLiteral("February Statement"));
  viewModel.setTransactionName(QStringLiteral("Rent"));
  viewModel.setTransactionBookingDate(QStringLiteral("2026-02-01"));
  viewModel.setTransactionValuta(QStringLiteral("EUR"));
  viewModel.setTransactionAmountText(QStringLiteral("1250.25"));
  viewModel.setTransactionAllocatable(true);

  const QString statementId = viewModel.submit();

  ASSERT_FALSE(statementId.isEmpty());
  const auto snapshot = harness.workspace->snapshot();
  ASSERT_EQ(snapshot.statements.size(), 1);
  ASSERT_EQ(snapshot.transactions.size(), 1);
  EXPECT_EQ(snapshot.statements.front().name, "February Statement");
  EXPECT_EQ(snapshot.transactions.front().name, "Rent");
  EXPECT_DOUBLE_EQ(snapshot.transactions.front().amount, 1250.25);
  EXPECT_TRUE(snapshot.transactions.front().allocatable);
}

TEST(BookingViewModelTest,
     VM_BOOKING_002_UpdateCurrentRoutesStatementAndTransactionChanges) {
  tests::support::WorkspaceHarness harness(tests::support::makeWorkspaceSnapshot());
  BookingViewModel viewModel(harness.store.get(), harness.commands.get(),
                             harness.selection.get(), harness.selectors.get());

  viewModel.selectTransaction(QStringLiteral("statement-1"),
                              QStringLiteral("tx-1"));
  viewModel.setStatementName(QStringLiteral("Updated Statement"));
  viewModel.setTransactionName(QStringLiteral("Updated Transaction"));
  viewModel.setTransactionAmountText(QStringLiteral("42.50"));
  viewModel.updateCurrent();

  const auto snapshot = harness.workspace->snapshot();
  EXPECT_EQ(snapshot.statements.front().name, "Updated Statement");
  EXPECT_EQ(snapshot.transactions.front().name, "Updated Transaction");
  EXPECT_DOUBLE_EQ(snapshot.transactions.front().amount, 42.50);
}

} // namespace ui
