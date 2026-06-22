/**
 * @file core/tests/application/analysis/TestAnalysisService.cpp
 * @brief Tests for analysis application execution.
 */

#include <gtest/gtest.h>

#include "core/application/analysis/AnalysisService.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Transaction.h"
#include "core/ports/infra/analysis-rendering/IAnalysisRenderer.h"

namespace core::application::analysis {

namespace {

class RecordingAnalysisImageRenderer final
    : public core::ports::analysis_rendering::IAnalysisRenderer {
public:
  bool renderToImage(
      const std::filesystem::path &outputPath, const std::string &title,
      const core::ports::analysis::AnalysisResult &result) const override {
    lastOutputPath = outputPath;
    lastTitle = title;
    lastTableRows = result.table.size();
    return true;
  }

  mutable std::filesystem::path lastOutputPath;
  mutable std::string lastTitle;
  mutable std::size_t lastTableRows = 0;
};

} // namespace

TEST(AnalysisServiceTest, RunsAnalysisAgainstWorkspaceState) {
    core::domain::catalog::WorkspaceCatalog state;

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Contract");
    contract->setType("rent");
    state.setContracts({contract});

    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setName("Rent");
    transaction->setBookingDate("2024-01-31");
    transaction->setAmount(10.0);
    transaction->setContractId("contract-1");
    state.setTransactions({transaction});

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Analysis");
    analysis->setType("tab");
    analysis->setExportFormat("xlsx");
    analysis->setConfigJson(R"({"group":"month"})");
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1");

    EXPECT_TRUE(result.found);
    EXPECT_EQ(result.type, "tab");
    EXPECT_EQ(result.transactions.size(), 1u);
    EXPECT_EQ(result.transactions.front().contractType, "rent");
}

TEST(AnalysisServiceTest, ReturnsUnfoundResultWhenAnalysisIdIsMissing) {
    core::domain::catalog::WorkspaceCatalog state;
    AnalysisService service;

    const auto result = service.runAnalysisById(state, "missing");

    EXPECT_FALSE(result.found);
    EXPECT_TRUE(result.transactions.empty());
}

TEST(AnalysisServiceTest, RunsAnalysisAgainstPersistedTransactionSnapshot) {
    core::domain::catalog::WorkspaceCatalog state;

    auto liveContract = std::make_shared<core::domain::Contract>();
    liveContract->setId("contract-live");
    liveContract->rename("Live Contract");
    liveContract->setType("live-type");
    state.setContracts({liveContract});

    auto liveTransaction = std::make_shared<core::domain::Transaction>();
    liveTransaction->setId("tx-live");
    liveTransaction->setName("Live Transaction");
    liveTransaction->setBookingDate("2026-01-31");
    liveTransaction->setAmount(999.0);
    liveTransaction->setContractId("contract-live");
    state.setTransactions({liveTransaction});

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Frozen Analysis");
    analysis->setType("plot");
    analysis->setExportFormat("png");
    analysis->setConfigJson(R"({"plotType":"pie","plotMeasure":"totalAmount","contractTypes":["snapshot-type"]})");
    analysis->setSnapshotTransactionsJson(R"([
        {
            "id":"tx-snapshot",
            "name":"Snapshot Transaction",
            "date":"2025-01-31",
            "amount":42.0,
            "contractId":"contract-snapshot",
            "contractName":"Snapshot Contract",
            "contractType":"snapshot-type",
            "propertyIds":["property-snapshot"],
            "propertyNames":["Snapshot Property"],
            "allocatable":true
        }
    ])");
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1", "date>=2025-01-01;date<=2025-12-31;contract.type=snapshot-type");

    ASSERT_TRUE(result.found);
    ASSERT_EQ(result.transactions.size(), 1u);
    EXPECT_EQ(result.transactions.front().id, "tx-snapshot");
    EXPECT_EQ(result.transactions.front().contractType, "snapshot-type");
    ASSERT_EQ(result.table.size(), 1u);
    EXPECT_EQ(result.table.front().front(), "snapshot-type");
}

TEST(AnalysisServiceTest, IncludesCalculationAdjustmentsInPlotResultsWhenEnabled) {
    core::domain::catalog::WorkspaceCatalog state;

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Contract");
    contract->setType("rent");
    state.setContracts({contract});

    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setName("Rent");
    transaction->setBookingDate("2026-01-31");
    transaction->setAmount(100.0);
    transaction->setContractId("contract-1");
    state.setTransactions({transaction});

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Adjusted Analysis");
    analysis->setType("plot");
    analysis->setExportFormat("png");
    analysis->setConfigJson(R"({"plotType":"pie","plotMeasure":"totalAmount"})");
    analysis->setIncludeCalculationAdjustments(true);
    analysis->setAdjustment("tx-1", 119.0);
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1");

    ASSERT_TRUE(result.found);
    ASSERT_EQ(result.table.size(), 1u);
    EXPECT_EQ(result.table.front().front(), "rent");
    EXPECT_DOUBLE_EQ(std::stod(result.table.front().at(1)), 119.0);
    ASSERT_EQ(result.transactions.size(), 1u);
    EXPECT_DOUBLE_EQ(result.transactions.front().amount, 119.0);
}

TEST(AnalysisServiceTest, AppliesCalculationAdjustmentsToTransactionIdSnapshots) {
    core::domain::catalog::WorkspaceCatalog state;

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Adjusted Snapshot Analysis");
    analysis->setType("plot");
    analysis->setExportFormat("png");
    analysis->setConfigJson(R"({"plotType":"pie","plotMeasure":"totalAmount"})");
    analysis->setSnapshotTransactionsJson(R"([
        {
            "transactionId":"tx-1",
            "name":"Rent",
            "date":"2026-01-31",
            "amount":100.0,
            "contractId":"contract-1",
            "contractName":"Contract",
            "contractType":"rent"
        }
    ])");
    analysis->setIncludeCalculationAdjustments(true);
    analysis->setAdjustment("tx-1", 119.0);
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1");

    ASSERT_TRUE(result.found);
    ASSERT_EQ(result.table.size(), 1u);
    EXPECT_EQ(result.table.front().front(), "rent");
    EXPECT_DOUBLE_EQ(std::stod(result.table.front().at(1)), 119.0);
    ASSERT_EQ(result.transactions.size(), 1u);
    EXPECT_EQ(result.transactions.front().id, "tx-1");
    EXPECT_DOUBLE_EQ(result.transactions.front().amount, 119.0);
}

TEST(AnalysisServiceTest, AppliesCalculationAdjustmentsToTransactionObjectSnapshots) {
    core::domain::catalog::WorkspaceCatalog state;

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Adjusted Object Snapshot Analysis");
    analysis->setType("plot");
    analysis->setExportFormat("png");
    analysis->setConfigJson(R"({"plotType":"pie","plotMeasure":"totalAmount"})");
    analysis->setSnapshotTransactionsJson(R"({
        "transactions": [
            {
                "transactionId":"tx-1",
                "name":"Rent",
                "date":"2026-01-31",
                "amount":100.0,
                "contractId":"contract-1",
                "contractName":"Contract",
                "contractType":"rent"
            }
        ]
    })");
    analysis->setIncludeCalculationAdjustments(true);
    analysis->setAdjustment("tx-1", 119.0);
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1");

    ASSERT_TRUE(result.found);
    ASSERT_EQ(result.table.size(), 1u);
    EXPECT_DOUBLE_EQ(std::stod(result.table.front().at(1)), 119.0);
    ASSERT_EQ(result.transactions.size(), 1u);
    EXPECT_EQ(result.transactions.front().id, "tx-1");
}

TEST(AnalysisServiceTest, PortRunRendersAnalysisArtifactsBehindRunnerBoundary) {
    core::ports::workspace::WorkspaceSnapshot workspace;

    core::ports::workspace::ContractSnapshot contract;
    contract.id = "contract-1";
    contract.name = "Contract";
    contract.type = "rent";
    workspace.contracts.push_back(contract);

    core::ports::workspace::TransactionSnapshot transaction;
    transaction.id = "tx-1";
    transaction.name = "Rent";
    transaction.bookingDate = "2026-01-31";
    transaction.amount = 100.0;
    transaction.contractId = "contract-1";
    workspace.transactions.push_back(transaction);

    core::ports::workspace::AnalysisSnapshot analysis;
    analysis.id = "analysis-1";
    analysis.name = "Analysis";
    analysis.type = "plot";
    analysis.config.type = "plot";
    analysis.config.plotType = "pie";
    analysis.config.plotMeasure = "totalAmount";
    workspace.analyses.push_back(analysis);

    auto renderer = std::make_shared<RecordingAnalysisImageRenderer>();
    AnalysisService service(renderer);
    core::ports::analysis::AnalysisRequest request;
    request.analysisId = "analysis-1";

    const auto result = service.runAnalysis(workspace, request);

    ASSERT_TRUE(result.found);
    ASSERT_EQ(result.artifacts.size(), 1u);
    EXPECT_FALSE(renderer->lastOutputPath.empty());
    EXPECT_EQ(renderer->lastTitle, "analysis-1");
    EXPECT_EQ(renderer->lastTableRows, result.table.size());
}

} // namespace core::application::analysis
