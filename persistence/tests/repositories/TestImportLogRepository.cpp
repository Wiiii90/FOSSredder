/**
 * @file persistence/tests/repositories/TestImportLogRepository.cpp
 * @brief Tests for the SQLite-backed import log repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteImportLogRepository.h"
#include "persistence/repositories/SqliteStatementDraftRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(ImportLogRepositoryTest, AddsUpdatesRemovesAndClearsImportLogs)
{
    TempDatabase dbFile("import-log-repository");
    SqliteStatementDraftRepository statementDrafts(dbFile.string());
    SqliteImportLogRepository repo(dbFile.string());

    statementDrafts.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(makeStatementDraft()));
    statementDrafts.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(makeStatementDraft("statement-draft-2")));

    const auto log = makeImportLog();
    repo.addImportLog(std::make_shared<core::application::importing::ImportLog>(log));

    ASSERT_EQ(repo.getImportLogs().size(), 1);
    ASSERT_TRUE(repo.getImportLogById("import-log-1").has_value());
    EXPECT_EQ(repo.getImportLogById("import-log-1").value()->type, "statement");
    EXPECT_EQ(repo.getImportLogById("import-log-1").value()->statementDraftIds,
              std::vector<std::string>({"statement-draft-1"}));

    auto updated = makeImportLog();
    updated.status = "running";
    updated.message = "Import is running";
    updated.statementDraftIds = { "statement-draft-1", "statement-draft-2" };
    repo.updateImportLog(std::make_shared<core::application::importing::ImportLog>(updated));

    ASSERT_TRUE(repo.getImportLogById("import-log-1").has_value());
    EXPECT_EQ(repo.getImportLogById("import-log-1").value()->status, "running");
    EXPECT_EQ(repo.getImportLogById("import-log-1").value()->statementDraftIds,
              std::vector<std::string>({"statement-draft-1", "statement-draft-2"}));

    repo.removeImportLog("import-log-1");
    EXPECT_FALSE(repo.getImportLogById("import-log-1").has_value());
    EXPECT_TRUE(repo.getImportLogs().empty());

    repo.addImportLog(std::make_shared<core::application::importing::ImportLog>(makeImportLog("import-log-2")));
    repo.clearImportLogs();
    EXPECT_TRUE(repo.getImportLogs().empty());
}

} // namespace persistence::tests
