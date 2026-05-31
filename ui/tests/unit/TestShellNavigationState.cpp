/**
 * @file ui/tests/unit/TestShellNavigationState.cpp
 * @brief Tests for the shell-level navigation state.
 */

#include <gtest/gtest.h>

#include "ui/state/navigation/NavigationState.h"
#include "ui/state/shell/ShellNavigationState.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

TEST(ShellNavigationStateTest,
     SHL_NAV_001_ToolbarCreateNavigationClearsWorkspaceSelection)
{
  NavigationState navigation;
  WorkspaceFacade workspace;
  workspace.loadFromState(tests::support::makeWorkspaceCatalog());
  ShellNavigationState state;
  state.setNavigation(&navigation);
  state.setWorkspace(&workspace);

  workspace.setSelectedActorId(QStringLiteral("actor-1"));
  workspace.setSelectedPropertyId(QStringLiteral("property-1"));
  workspace.setSelectedContractId(QStringLiteral("contract-1"));
  workspace.setSelectedStatementId(QStringLiteral("statement-1"));
  workspace.setSelectedTransactionId(QStringLiteral("tx-1"));
  workspace.setSelectedAnalysisId(QStringLiteral("analysis-1"));
  workspace.setSelectedAnnualId(QStringLiteral("annual-1"));

  state.navigateToSection(state.actorSection(), true);

  EXPECT_EQ(navigation.sectionValue(), state.actorSection());
  EXPECT_TRUE(workspace.selectedActorId().isEmpty());
  EXPECT_TRUE(workspace.selectedPropertyId().isEmpty());
  EXPECT_TRUE(workspace.selectedContractId().isEmpty());
  EXPECT_TRUE(workspace.selectedStatementId().isEmpty());
  EXPECT_TRUE(workspace.selectedTransactionId().isEmpty());
  EXPECT_TRUE(workspace.selectedAnalysisId().isEmpty());
  EXPECT_TRUE(workspace.selectedAnnualId().isEmpty());
  EXPECT_TRUE(state.actorLoaded());
}

TEST(ShellNavigationStateTest,
     SHL_NAV_002_MenuNavigationPreservesNonBookingSelection)
{
  NavigationState navigation;
  WorkspaceFacade workspace;
  workspace.loadFromState(tests::support::makeWorkspaceCatalog());
  ShellNavigationState state;
  state.setNavigation(&navigation);
  state.setWorkspace(&workspace);

  workspace.setSelectedActorId(QStringLiteral("actor-1"));
  workspace.setSelectedStatementId(QStringLiteral("statement-1"));
  workspace.setSelectedTransactionId(QStringLiteral("tx-1"));

  state.navigateToSection(state.propertySection());

  EXPECT_EQ(navigation.sectionValue(), state.propertySection());
  EXPECT_EQ(workspace.selectedActorId(), QStringLiteral("actor-1"));
  EXPECT_TRUE(workspace.selectedStatementId().isEmpty());
  EXPECT_TRUE(workspace.selectedTransactionId().isEmpty());
  EXPECT_TRUE(state.propertyLoaded());
}

TEST(ShellNavigationStateTest, SHL_NAV_003_BookingCreateNavigationClearsBooking)
{
  NavigationState navigation;
  WorkspaceFacade workspace;
  ShellNavigationState state;
  state.setNavigation(&navigation);
  state.setWorkspace(&workspace);

  workspace.setSelectedStatementId(QStringLiteral("statement-1"));
  workspace.setSelectedTransactionId(QStringLiteral("tx-1"));

  state.navigateToBookingCreate();

  EXPECT_EQ(navigation.sectionValue(), state.bookingSection());
  EXPECT_TRUE(workspace.selectedStatementId().isEmpty());
  EXPECT_TRUE(workspace.selectedTransactionId().isEmpty());
  EXPECT_TRUE(state.bookingLoaded());
}

} // namespace ui
