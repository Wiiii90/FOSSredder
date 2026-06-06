/**
 * @file persistence/tests/repositories/TestAnalysisRepository.cpp
 * @brief Tests for the SQLite-backed analysis repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(AnalysisRepositoryTest, AddsUpdatesRemovesAndClearsAnalyses)
{
    TempDatabase dbFile("analysis-repository");
    SqliteAnalysisRepository repo(dbFile.string());

    const auto analysis = makeAnalysis();
    repo.addAnalysis(analysis);

    ASSERT_EQ(repo.getAnalyses().size(), 1);
    ASSERT_TRUE(repo.getAnalysisById("analysis-1").has_value());
    EXPECT_EQ(repo.getAnalysisById("analysis-1").value()->name(), "Monthly Analysis");
    EXPECT_EQ(repo.getAnalysisById("analysis-1").value()->type(), "tabular");
    EXPECT_TRUE(repo.getAnalysisById("analysis-1").value()->includeCalculationAdjustments());
    EXPECT_EQ(repo.getAnalysisById("analysis-1").value()->adjustments().at("actor-1"), 19.25);

    auto updated = makeAnalysis();
    updated->rename("Updated Analysis");
    updated->setExportFormat("xlsx");
    updated->setIncludeCalculationAdjustments(false);
    repo.updateAnalysis(updated);

    ASSERT_TRUE(repo.getAnalysisById("analysis-1").has_value());
    EXPECT_EQ(repo.getAnalysisById("analysis-1").value()->name(), "Updated Analysis");
    EXPECT_EQ(repo.getAnalysisById("analysis-1").value()->exportFormat(), "xlsx");
    EXPECT_FALSE(repo.getAnalysisById("analysis-1").value()->includeCalculationAdjustments());

    repo.removeAnalysis("analysis-1");
    EXPECT_FALSE(repo.getAnalysisById("analysis-1").has_value());
    EXPECT_TRUE(repo.getAnalyses().empty());

    repo.addAnalysis(makeAnalysis("analysis-2"));
    repo.clearAnalyses();
    EXPECT_TRUE(repo.getAnalyses().empty());
}

} // namespace persistence::tests
