/**
 * @file persistence/tests/support/PersistenceTestData.h
 * @brief Shared persistence test fixtures and sample workspace data.
 */

#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <system_error>

#include <sqlite3.h>

#include "core/application/export/ExportLog.h"
#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Alias.h"

namespace persistence::tests {

class TempDatabase {
public:
    explicit TempDatabase(std::string stem)
    {
        static std::atomic<unsigned long long> counter{0};
        const auto id = ++counter;
        path_ = std::filesystem::temp_directory_path() /
                (stem + "-" + std::to_string(id) + ".sqlite");
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    ~TempDatabase()
    {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept
    {
        return path_;
    }

    [[nodiscard]] std::string string() const
    {
        return path_.string();
    }

private:
    std::filesystem::path path_;
};

inline core::domain::Alias makeAlias(std::string value,
                                     std::string kind = "manual",
                                     std::string source = "tests",
                                     std::string createdAt = "2026-01-01T00:00:00Z",
                                     std::string updatedAt = "2026-01-02T00:00:00Z",
                                     int hitCount = 1,
                                     std::string lastUsedAt = "2026-01-03T00:00:00Z")
{
    return core::domain::Alias(std::move(value),
                               std::move(kind),
                               std::move(source),
                               std::move(createdAt),
                               std::move(updatedAt),
                               hitCount,
                               std::move(lastUsedAt));
}

inline std::shared_ptr<core::domain::Actor> makeActor(const std::string& id = "actor-1")
{
    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId(id);
    actor->rename("Main Actor");
    actor->addAlias(makeAlias("Primary Actor"));
    actor->addAlias(makeAlias("Main Actor"));
    actor->setCreatedAt("2026-01-01T08:00:00Z");
    actor->setUpdatedAt("2026-01-02T08:00:00Z");
    return actor;
}

inline std::shared_ptr<core::domain::Property> makeProperty(const std::string& id = "property-1")
{
    auto property = std::make_shared<core::domain::Property>();
    property->setId(id);
    property->rename("Primary Property");
    property->addAlias(makeAlias("Property Alias"));
    property->setCreatedAt("2026-01-01T08:00:00Z");
    property->setUpdatedAt("2026-01-02T08:00:00Z");
    return property;
}

inline std::shared_ptr<core::domain::Contract> makeContract(const std::string& id = "contract-1")
{
    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId(id);
    contract->rename("Lease Contract");
    contract->setType("lease");
    contract->setActorIds({ "actor-1" });
    contract->setPropertyIds({ "property-1" });
    contract->addAlias(makeAlias("Lease"));
    contract->setCreatedAt("2026-01-01T08:00:00Z");
    contract->setUpdatedAt("2026-01-02T08:00:00Z");
    return contract;
}

inline std::shared_ptr<core::domain::Statement> makeStatement(const std::string& id = "statement-1")
{
    auto statement = std::make_shared<core::domain::Statement>();
    statement->setId(id);
    statement->rename("January Statement");
    statement->setTransactionIds({ "tx-1", "tx-2" });
    statement->setCreatedAt("2026-01-01T08:00:00Z");
    statement->setUpdatedAt("2026-01-02T08:00:00Z");
    return statement;
}

inline std::shared_ptr<core::domain::Transaction> makeTransaction(const std::string& id = "tx-1",
                                                                   const std::string& statementId = "statement-1",
                                                                   const std::string& propertyId = "property-1")
{
    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId(id);
    transaction->setName(id == "tx-1" ? "Rent" : "Fee");
    transaction->setBookingDate(id == "tx-1" ? "2026-01-05" : "2026-01-06");
    transaction->setValuta("EUR");
    transaction->setAmount(id == "tx-1" ? 1250.0 : 19.25);
    transaction->setStatus(id == "tx-1" ? core::domain::Transaction::Status::Verified
                                         : core::domain::Transaction::Status::Unverified);
    transaction->setStatementId(statementId);
    transaction->setContractId("contract-1");
    transaction->setActorId("actor-1");
    transaction->setPropertyIds({ propertyId });
    transaction->setAllocatable(id == "tx-1");
    transaction->setCreatedAt("2026-01-01T08:00:00Z");
    transaction->setUpdatedAt("2026-01-02T08:00:00Z");
    return transaction;
}

inline std::shared_ptr<core::domain::Analysis> makeAnalysis(const std::string& id = "analysis-1")
{
    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId(id);
    analysis->rename("Monthly Analysis");
    analysis->setType("tabular");
    analysis->setConfigJson("{\"groupBy\":\"month\"}");
    analysis->setFilterSpec("{}");
    analysis->setExportFormat("csv");
    analysis->setIncludeCalculationAdjustments(true);
    analysis->setExportStateJson("{}");
    analysis->setSnapshotTransactionsJson("[\"tx-1\"]");
    analysis->setAdjustment("actor-1", 19.25);
    analysis->setCreatedAt("2026-01-01T08:00:00Z");
    analysis->setUpdatedAt("2026-01-02T08:00:00Z");
    return analysis;
}

inline std::shared_ptr<core::domain::Annual> makeAnnual(const std::string& id = "annual-1")
{
    auto annual = std::make_shared<core::domain::Annual>();
    annual->setId(id);
    annual->rename("2026");
    annual->setYear(2026);
    annual->setAnalysisIds({ "analysis-1" });
    annual->setCreatedAt("2026-01-01T08:00:00Z");
    annual->setUpdatedAt("2026-01-02T08:00:00Z");
    return annual;
}

inline core::application::importing::draft::TransactionDraft makeTransactionDraft(const std::string& id = "draft-tx-1")
{
    core::application::importing::draft::TransactionDraft draft;
    draft.id = id;
    draft.statementDraftId = "statement-draft-1";
    draft.position = id == "draft-tx-1" ? 0 : 1;
    draft.name = id == "draft-tx-1" ? "Draft Rent" : "Draft Fee";
    draft.bookingDate = id == "draft-tx-1" ? "2026-01-05" : "2026-01-06";
    draft.valuta = "EUR";
    draft.amount = id == "draft-tx-1" ? 1250.0 : 19.25;
    draft.actorText = "Main Actor";
    draft.propertyText = "Primary Property";
    draft.actorId = "actor-1";
    draft.actorSelected = true;
    draft.contractId = "contract-1";
    draft.contractSelected = true;
    draft.metadata = "{}";
    draft.type = "lease";
    draft.allocatable = id == "draft-tx-1";
    draft.allocatableSelected = true;
    draft.status = core::domain::Transaction::Status::Verified;
    draft.propertyIds = { "property-1" };
    draft.createdAt = "2026-01-01T08:00:00Z";
    draft.updatedAt = "2026-01-02T08:00:00Z";
    return draft;
}

inline core::application::importing::draft::StatementDraft makeStatementDraft(const std::string& id = "statement-draft-1")
{
    core::application::importing::draft::StatementDraft draft;
    draft.id = id;
    draft.name = "Imported Statement";
    draft.transactionIds = { "draft-tx-1", "draft-tx-2" };
    draft.createdAt = "2026-01-01T08:00:00Z";
    draft.updatedAt = "2026-01-02T08:00:00Z";
    draft.transactions = { makeTransactionDraft("draft-tx-1"), makeTransactionDraft("draft-tx-2") };
    return draft;
}

inline core::application::importing::ImportLog makeImportLog(const std::string& id = "import-log-1")
{
    core::application::importing::ImportLog log;
    log.id = id;
    log.time = "2026-01-10T10:00:00Z";
    log.type = "statement";
    log.file = "import.csv";
    log.status = "done";
    log.message = "Import completed";
    log.draftAttached = true;
    log.draftId = "statement-draft-1";
    log.statementDraftIds = { "statement-draft-1" };
    log.statementId = "statement-1";
    return log;
}

inline core::application::exporting::ExportLog makeExportLog(const std::string& id = "export-log-1")
{
    core::application::exporting::ExportLog log;
    log.id = id;
    log.time = "2026-01-10T11:00:00Z";
    log.targetPath = "export.csv";
    log.status = "done";
    log.message = "Export completed";
    log.annualIds = { "annual-1" };
    log.analysisIds = { "analysis-1" };
    return log;
}

inline core::domain::catalog::WorkspaceCatalog makeWorkspaceCatalog()
{
    core::domain::catalog::WorkspaceCatalog catalog;
    catalog.setActors({ makeActor() });
    catalog.setProperties({ makeProperty() });
    catalog.setContracts({ makeContract() });
    catalog.setStatements({ makeStatement() });
    catalog.setTransactions({ makeTransaction("tx-1", "statement-1", "property-1"),
                              makeTransaction("tx-2", "statement-1", "property-1") });
    catalog.setAnalyses({ makeAnalysis() });
    catalog.setAnnuals({ makeAnnual() });
    return catalog;
}

inline core::application::workspace::WorkspaceSessionState makeWorkspaceSessionState()
{
    core::application::workspace::WorkspaceSessionState state;
    state.catalog = makeWorkspaceCatalog();
    state.workflow.statementDrafts = { std::make_shared<core::application::importing::draft::StatementDraft>(makeStatementDraft()) };
    state.workflow.transactionDrafts = { std::make_shared<core::application::importing::draft::TransactionDraft>(makeTransactionDraft("draft-tx-1")),
                                         std::make_shared<core::application::importing::draft::TransactionDraft>(makeTransactionDraft("draft-tx-2")) };
    state.workflow.importLogs = { std::make_shared<core::application::importing::ImportLog>(makeImportLog()) };
    state.workflow.exportLogs = { std::make_shared<core::application::exporting::ExportLog>(makeExportLog()) };
    return state;
}

inline bool tableExists(sqlite3* db, const std::string& tableName)
{
    if (!db) {
        return false;
    }

    const char* sql = "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);
    const bool found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return found;
}

inline int pragmaInt(sqlite3* db, const std::string& pragma)
{
    if (!db) {
        return 0;
    }

    const std::string sql = "PRAGMA " + pragma + ";";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    int value = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return value;
}

inline int rowCount(sqlite3* db, const std::string& tableName)
{
    if (!db) {
        return 0;
    }

    const std::string sql = "SELECT COUNT(*) FROM " + tableName + ";";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

} // namespace persistence::tests
