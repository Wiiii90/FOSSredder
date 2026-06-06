/**
 * @file ui/tests/unit/adapters/TestAnalysisAdapter.cpp
 * @brief Tests for the UI AnalysisAdapter boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/AnalysisAdapter.h"

namespace ui {

TEST(AnalysisAdapterTest,
     ADP_ANALYSIS_001_RunAndPreviewDelegateToAnalysisRunner) {
  const auto runner = std::make_shared<tests::support::FakeAnalysisRunner>();
  adapters::AnalysisAdapter adapter(runner);
  const auto snapshot = tests::support::makeWorkspaceSnapshot();

  core::ports::analysis::AnalysisRequest request;
  request.analysisId = "analysis-1";
  const auto result = adapter.runAnalysis(snapshot, request);
  const auto preview = adapter.previewTransactions(snapshot, {});

  EXPECT_TRUE(result.found);
  EXPECT_EQ(preview.transactions.size(), snapshot.transactions.size());
}

TEST(AnalysisAdapterTest,
     ADP_ANALYSIS_002_MapsAnalysisResultToQmlPayloadShape) {
  adapters::AnalysisAdapter adapter(
      std::make_shared<tests::support::FakeAnalysisRunner>());
  const auto snapshot = tests::support::makeWorkspaceSnapshot();
  core::ports::analysis::AnalysisRequest request;
  request.analysisId = "analysis-1";

  const QVariantMap payload =
      adapter.mapAnalysisResult(adapter.runAnalysis(snapshot, request));

  EXPECT_TRUE(payload.contains(QStringLiteral("metrics")));
  EXPECT_TRUE(payload.contains(QStringLiteral("transactions")));
  EXPECT_TRUE(payload.contains(QStringLiteral("artifacts")));
}

TEST(AnalysisAdapterTest,
     ADP_ANALYSIS_003_BuildAnalysisRequestParsesFilterThroughRunner) {
  adapters::AnalysisAdapter adapter(
      std::make_shared<tests::support::FakeAnalysisRunner>());

  const auto request = adapter.buildAnalysisRequest(
      QStringLiteral(" analysis-1 "),
      QStringLiteral("propertyId=property-1;allocatable=allocatable"));

  EXPECT_EQ(request.analysisId, "analysis-1");
  ASSERT_EQ(request.filter.propertyIds.size(), 1U);
  EXPECT_EQ(request.filter.propertyIds.front(), "property-1");
  EXPECT_EQ(request.filter.allocatableMode, "allocatable");
}

} // namespace ui
