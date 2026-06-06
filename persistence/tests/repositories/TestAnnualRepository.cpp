/**
 * @file persistence/tests/repositories/TestAnnualRepository.cpp
 * @brief Tests for the SQLite-backed annual repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/repositories/SqliteAnnualRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(AnnualRepositoryTest, AddsUpdatesRemovesAndClearsAnnuals)
{
    TempDatabase dbFile("annual-repository");
    SqliteAnalysisRepository analyses(dbFile.string());
    SqliteAnnualRepository repo(dbFile.string());

    analyses.addAnalysis(makeAnalysis());
    analyses.addAnalysis(makeAnalysis("analysis-2"));

    const auto annual = makeAnnual();
    repo.addAnnual(annual);

    ASSERT_EQ(repo.getAnnuals().size(), 1);
    ASSERT_TRUE(repo.getAnnualById("annual-1").has_value());
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->name(), "2026");
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->year(), 2026);
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->analysisIds(),
              std::vector<std::string>({"analysis-1"}));

    auto updated = makeAnnual();
    updated->rename("2027");
    updated->setYear(2027);
    updated->setAnalysisIds({ "analysis-1", "analysis-2" });
    repo.updateAnnual(updated);

    ASSERT_TRUE(repo.getAnnualById("annual-1").has_value());
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->name(), "2027");
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->year(), 2027);
    EXPECT_EQ(repo.getAnnualById("annual-1").value()->analysisIds(),
              std::vector<std::string>({"analysis-1", "analysis-2"}));

    repo.removeAnnual("annual-1");
    EXPECT_FALSE(repo.getAnnualById("annual-1").has_value());
    EXPECT_TRUE(repo.getAnnuals().empty());

    repo.addAnnual(makeAnnual("annual-2"));
    repo.clearAnnuals();
    EXPECT_TRUE(repo.getAnnuals().empty());
}

} // namespace persistence::tests
