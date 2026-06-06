/**
 * @file core/tests/application/annual/TestAnnualService.cpp
 * @brief Tests deterministic annual derived projections.
 */

#include <gtest/gtest.h>

#include "core/application/annual/AnnualService.h"

namespace core::application::annual {

TEST(AnnualServiceTest, ParsesArraySnapshotPayloads)
{
    core::ports::workspace::WorkspaceSnapshot workspace;
    workspace.annuals.push_back({"annual-1", "Annual 2026", 2026, {"analysis-a", "analysis-b"}});
    workspace.analyses.push_back({"analysis-a", "Analysis A", "plot", "", "", "", true, "", R"([{"id":"tx-a"}])"});
    workspace.analyses.push_back({"analysis-b", "Analysis B", "plot", "", "", "", true, "", R"([{"id":"tx-b","bookingDate":"2026-02-01","amount":20.0,"allocatable":false}])"});
    core::ports::workspace::TransactionSnapshot liveA;
    liveA.id = "tx-a";
    liveA.name = "Live A";
    liveA.bookingDate = "2026-01-01";
    liveA.amount = 10.0;
    workspace.transactions.push_back(liveA);

    AnnualService service;
    core::ports::annual::AnnualRequest request;
    request.annualId = "annual-1";
    const auto result = service.runAnnual(workspace, request);

    EXPECT_EQ(result.stats.assignedAnalysisCount, 2);
    EXPECT_EQ(result.stats.snapshotTransactionCount, 2);
    EXPECT_EQ(result.deduplicated.size() + result.similar.size() + result.divergent.size(), 2u);
}

TEST(AnnualServiceTest, CategorizesRowsAndBuildsStatsDeterministically)
{
    core::ports::workspace::WorkspaceSnapshot workspace;
    workspace.annuals.push_back({"annual-1", "Annual 2026", 2026, {"analysis-a", "analysis-b"}});

    workspace.analyses.push_back({"analysis-a", "Analysis A", "plot", "", "", "", true, "", R"([
        {"id":"dup","date":"2026-03-01","amount":100.0,"allocatable":true,"contractId":"c-1","statementId":"s-1"},
        {"id":"sim-1","date":"2026-04-01","amount":10.0,"allocatable":true,"contractId":"c-1","statementId":"s-2"},
        {"id":"div-1","date":"2025-12-31","amount":33.0,"allocatable":false,"contractId":"c-2","statementId":"s-3"}
    ])"});
    workspace.analyses.push_back({"analysis-b", "Analysis B", "plot", "", "", "", true, "", R"([
        {"id":"dup","date":"2026-03-01","amount":100.0,"allocatable":true,"contractId":"c-1","statementId":"s-1"},
        {"id":"sim-2","date":"2026-04-01","amount":12.0,"allocatable":true,"contractId":"c-1","statementId":"s-2"}
    ])"});

    workspace.contracts.push_back({"c-1", "Main", "rent"});
    workspace.contracts.push_back({"c-2", "Legacy", "service"});

    core::ports::workspace::TransactionSnapshot liveDup;
    liveDup.id = "dup";
    liveDup.bookingDate = "2026-03-01";
    liveDup.status = 2;
    liveDup.allocatable = true;
    liveDup.contractId = "c-1";
    workspace.transactions.push_back(liveDup);

    core::ports::workspace::TransactionSnapshot workspaceOnly;
    workspaceOnly.id = "live-only";
    workspaceOnly.bookingDate = "2026-10-15";
    workspaceOnly.status = 1;
    workspaceOnly.allocatable = false;
    workspaceOnly.contractId = "c-2";
    workspace.transactions.push_back(workspaceOnly);

    AnnualService service;
    const auto result = service.buildAnnualResult(workspace, "annual-1");

    EXPECT_EQ(result.deduplicated.size(), 1u);
    EXPECT_EQ(result.similar.size(), 1u);
    EXPECT_EQ(result.divergent.size(), 1u);
    EXPECT_EQ(result.workspaceOnly.size(), 1u);

    EXPECT_EQ(result.stats.duplicateCount, 1);
    EXPECT_EQ(result.stats.missingLive, 2);
    EXPECT_EQ(result.missingLive.size(), 2u);
    EXPECT_EQ(result.stats.mixedYear, 1);
    EXPECT_EQ(result.stats.missingFromYear, 1);
    EXPECT_EQ(result.stats.verified, 1);
}

TEST(AnnualServiceTest, SingleAnalysisProjectsSnapshotRowsAsDeduplicatedOnly)
{
    core::ports::workspace::WorkspaceSnapshot workspace;
    workspace.annuals.push_back({"annual-1", "Annual 2026", 2026, {"analysis-a"}});
    workspace.analyses.push_back({"analysis-a", "Analysis A", "plot", "", "", "", true, "", R"([
        {"id":"tx-1","date":"2026-04-01","amount":10.0,"allocatable":true,"contractId":"c-1","statementId":"s-1"},
        {"id":"tx-2","date":"2026-04-01","amount":12.0,"allocatable":true,"contractId":"c-1","statementId":"s-1"}
    ])"});

    AnnualService service;
    const auto result = service.buildAnnualResult(workspace, "annual-1");

    EXPECT_EQ(result.deduplicated.size(), 2u);
    EXPECT_TRUE(result.similar.empty());
    EXPECT_TRUE(result.divergent.empty());
}

} // namespace core::application::annual
