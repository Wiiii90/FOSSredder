/**
 * @file ui/tests/unit/adapters/TestAnnualAdapter.cpp
 * @brief Tests for the UI AnnualAdapter boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnnualAdapter.h"

namespace ui {

TEST(AnnualAdapterTest, ADP_ANNUAL_001_RunDelegatesToAnnualRunner) {
  adapters::AnnualAdapter adapter(
      std::make_shared<tests::support::FakeAnnualRunner>());
  const auto snapshot = tests::support::makeWorkspaceSnapshot();

  core::ports::annual::AnnualRequest request;
  request.annualId = "annual-1";
  const auto result = adapter.runAnnual(snapshot, request);

  EXPECT_EQ(result.annualId, "annual-1");
  EXPECT_EQ(result.stats.assignedAnalysisCount, 1);
}

TEST(AnnualAdapterTest, ADP_ANNUAL_002_MapsAnnualResultToQmlPayloadShape) {
  adapters::AnnualAdapter adapter(
      std::make_shared<tests::support::FakeAnnualRunner>());
  const auto snapshot = tests::support::makeWorkspaceSnapshot();
  core::ports::annual::AnnualRequest request;
  request.annualId = "annual-1";

  const QVariantMap payload =
      adapter.mapAnnualResult(adapter.runAnnual(snapshot, request));

  EXPECT_EQ(payload.value(QStringLiteral("annualId")).toString(),
            QStringLiteral("annual-1"));
  EXPECT_TRUE(payload.contains(QStringLiteral("stats")));
  EXPECT_TRUE(payload.contains(QStringLiteral("transactions")));
}

} // namespace ui
