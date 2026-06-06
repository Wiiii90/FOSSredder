/**
 * @file core/tests/domain/entities/TestAnalysis.cpp
 * @brief Tests for analysis behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Analysis.h"

namespace core::domain {

TEST(AnalysisTest, ConfiguresCoreFieldsAndFlags) {
    Analysis analysis;

    analysis.rename("  Cashflow Analysis  ");
    analysis.setType("  tab  ");
    analysis.setConfigJson(R"({"group":"month"})");
    analysis.setFilterSpec("  contract:rent  ");
    analysis.setExportFormat(" xlsx ");
    analysis.setIncludeCalculationAdjustments(false);
    analysis.setExportStateJson(R"({"state":"ready"})");
    analysis.setSnapshotTransactionsJson(R"(["tx-1"])");
    analysis.setAdjustment("  rent  ", 12.5);

    EXPECT_EQ(analysis.name(), "Cashflow Analysis");
    EXPECT_EQ(analysis.type(), "tab");
    EXPECT_EQ(analysis.configJson(), R"({"group":"month"})");
    EXPECT_EQ(analysis.filterSpec(), "contract:rent");
    EXPECT_EQ(analysis.exportFormat(), "xlsx");
    EXPECT_FALSE(analysis.includeCalculationAdjustments());
    EXPECT_EQ(analysis.exportStateJson(), R"({"state":"ready"})");
    EXPECT_EQ(analysis.snapshotTransactionsJson(), R"([{"transactionId":"tx-1"}])");
    EXPECT_TRUE(analysis.hasAdjustment("rent"));
    EXPECT_EQ(analysis.adjustmentCount(), 1u);
    EXPECT_TRUE(analysis.hasType());
    EXPECT_TRUE(analysis.hasExportFormat());
    EXPECT_TRUE(analysis.hasConfig());
    EXPECT_TRUE(analysis.hasFilterSpec());
    EXPECT_TRUE(analysis.hasSnapshotTransactions());
    EXPECT_TRUE(analysis.isReadyForExport());
    EXPECT_TRUE(analysis.isConfigured());
    EXPECT_TRUE(analysis.isTabular());
    EXPECT_FALSE(analysis.isChartLike());
    EXPECT_TRUE(analysis.supportsExportFormat(" xlsx "));
    EXPECT_EQ(analysis.executionType(), "tab");
    EXPECT_EQ(analysis.outputType(), "tab");
}

TEST(AnalysisTest, SupportsChartLikeOutputAndAdjustmentRemoval) {
    Analysis analysis;

    analysis.setType("pie");
    analysis.setExportFormat("csv");
    analysis.setConfigJson(R"({"plotType":"histogram"})");
    analysis.setAdjustment("alpha", 1.0);

    EXPECT_TRUE(analysis.isChartLike());
    EXPECT_TRUE(analysis.isReadyForExport());
    EXPECT_EQ(analysis.outputType(), "pie");
    EXPECT_EQ(analysis.adjustmentCount(), 1u);

    analysis.removeAdjustment(" alpha ");
    EXPECT_FALSE(analysis.hasAdjustments());
    EXPECT_FALSE(analysis.hasAdjustment("alpha"));
}

TEST(AnalysisTest, NormalizesLegacySnapshotTransactionListsToObjectRows) {
    Analysis analysis;

    analysis.setSnapshotTransactionsJson(R"({"transactions":["tx-a","tx-b"]})");

    EXPECT_EQ(analysis.snapshotTransactionsJson(),
              R"({"transactions":[{"transactionId":"tx-a"},{"transactionId":"tx-b"}]})");
}

} // namespace core::domain
