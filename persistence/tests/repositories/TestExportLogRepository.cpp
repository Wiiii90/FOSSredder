/**
 * @file persistence/tests/repositories/TestExportLogRepository.cpp
 * @brief Tests for the SQLite-backed export log repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/repositories/SqliteAnnualRepository.h"
#include "persistence/repositories/SqliteExportLogRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(ExportLogRepositoryTest, AddsUpdatesRemovesAndClearsExportLogs)
{
    TempDatabase dbFile("export-log-repository");
    SqliteAnalysisRepository analyses(dbFile.string());
    SqliteAnnualRepository annuals(dbFile.string());
    SqliteExportLogRepository repo(dbFile.string());

    analyses.addAnalysis(makeAnalysis());
    analyses.addAnalysis(makeAnalysis("analysis-2"));
    annuals.addAnnual(makeAnnual());
    annuals.addAnnual(makeAnnual("annual-2"));

    const auto log = makeExportLog();
    repo.addExportLog(std::make_shared<core::application::exporting::ExportLog>(log));

    ASSERT_EQ(repo.getExportLogs().size(), 1);
    ASSERT_TRUE(repo.getExportLogById("export-log-1").has_value());
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->targetPath, "export.csv");
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->annualIds,
              std::vector<std::string>({"annual-1"}));
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->analysisIds,
              std::vector<std::string>({"analysis-1"}));

    auto updated = makeExportLog();
    updated.status = "running";
    updated.message = "Export is running";
    updated.annualIds = { "annual-1", "annual-2" };
    updated.analysisIds = { "analysis-1", "analysis-2" };
    repo.updateExportLog(std::make_shared<core::application::exporting::ExportLog>(updated));

    ASSERT_TRUE(repo.getExportLogById("export-log-1").has_value());
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->status, "running");
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->annualIds,
              std::vector<std::string>({"annual-1", "annual-2"}));
    EXPECT_EQ(repo.getExportLogById("export-log-1").value()->analysisIds,
              std::vector<std::string>({"analysis-1", "analysis-2"}));

    repo.removeExportLog("export-log-1");
    EXPECT_FALSE(repo.getExportLogById("export-log-1").has_value());
    EXPECT_TRUE(repo.getExportLogs().empty());

    repo.addExportLog(std::make_shared<core::application::exporting::ExportLog>(makeExportLog("export-log-2")));
    repo.clearExportLogs();
    EXPECT_TRUE(repo.getExportLogs().empty());
}

} // namespace persistence::tests
