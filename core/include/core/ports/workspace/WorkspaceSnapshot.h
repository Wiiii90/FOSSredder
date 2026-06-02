/**
 * @file core/include/core/ports/workspace/WorkspaceSnapshot.h
 * @brief Immutable workspace read-model snapshot exposed through workspace ports.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::workspace {

struct AliasSnapshot {
    std::string value;
    std::string kind;
    std::string source;
    int hitCount = 0;
    std::string lastUsedAt;
    std::string createdAt;
    std::string updatedAt;
};

struct ActorSnapshot {
    std::string id;
    std::string name;
    std::vector<AliasSnapshot> aliases;
    std::vector<std::string> contractIds;
    std::string createdAt;
    std::string updatedAt;
};

struct PropertySnapshot {
    std::string id;
    std::string name;
    std::vector<AliasSnapshot> aliases;
    std::vector<std::string> contractIds;
    std::string createdAt;
    std::string updatedAt;
};

struct ContractSnapshot {
    std::string id;
    std::string name;
    std::string type;
    std::string allocatableMode = "mixed";
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<AliasSnapshot> aliases;
    std::string createdAt;
    std::string updatedAt;
};

struct StatementSnapshot {
    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
};

struct TransactionSnapshot {
    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    int status = 0;
    std::string contractId;
    std::string actorId;
    std::string statementId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
    std::string createdAt;
    std::string updatedAt;
};

struct AnalysisSnapshot {
    std::string id;
    std::string name;
    std::string type;
    std::string configJson;
    std::string filterSpec;
    std::string exportFormat;
    bool includeCalculationAdjustments = true;
    std::string exportStateJson;
    std::string snapshotTransactionsJson;
    std::vector<std::pair<std::string, double>> adjustments;
    std::string createdAt;
    std::string updatedAt;
};

struct AnnualSnapshot {
    std::string id;
    std::string name;
    int year = 0;
    std::vector<std::string> analysisIds;
    std::string createdAt;
    std::string updatedAt;
};

struct TransactionDraftSnapshot {
    std::string id;
    std::string statementDraftId;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string actorId;
    std::string contractId;
    std::vector<std::string> propertyIds;
    int status = 0;
    bool allocatable = false;
    int position = 0;
    std::string metadata;
    std::vector<uint8_t> proofImageData;
};

struct StatementDraftSnapshot {
    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
    std::vector<TransactionDraftSnapshot> transactions;
};

struct ImportLogSnapshot {
    std::string id;
    std::string time;
    std::string type;
    std::string file;
    std::string status;
    std::string message;
    bool draftAttached = false;
    std::string draftId;
    std::vector<std::string> statementDraftIds;
    std::string statementId;
};

struct ExportLogSnapshot {
    std::string id;
    std::string time;
    std::string targetPath;
    std::string status;
    std::string message;
    std::string payload;
    std::vector<std::string> annualIds;
    std::vector<std::string> analysisIds;
};

struct WorkspaceSnapshot {
    std::string currentPath;
    bool hasCurrentPath = false;

    std::vector<ActorSnapshot> actors;
    std::vector<PropertySnapshot> properties;
    std::vector<ContractSnapshot> contracts;
    std::vector<StatementSnapshot> statements;
    std::vector<TransactionSnapshot> transactions;
    std::vector<AnalysisSnapshot> analyses;
    std::vector<AnnualSnapshot> annuals;
    std::vector<StatementDraftSnapshot> statementDrafts;
    std::vector<ImportLogSnapshot> importLogs;
    std::vector<ExportLogSnapshot> exportLogs;

    /** @brief Checks whether all snapshot collections are empty and no file is open. */
    [[nodiscard]] bool empty() const noexcept {
        return !hasCurrentPath && actors.empty() && properties.empty() && contracts.empty() &&
               statements.empty() && transactions.empty() && analyses.empty() && annuals.empty() &&
               statementDrafts.empty() && importLogs.empty() && exportLogs.empty();
    }
};

struct DeletionImpact {
    std::vector<std::string> deletedActorIds;
    std::vector<std::string> deletedPropertyIds;
    std::vector<std::string> deletedContractIds;
    std::vector<std::string> deletedStatementIds;
    std::vector<std::string> deletedTransactionIds;
    std::vector<std::string> deletedAnalysisIds;
    std::vector<std::string> deletedAnnualIds;
    std::vector<std::string> deletedStatementDraftIds;
    std::vector<std::string> deletedTransactionDraftIds;
    std::vector<std::string> deletedImportLogIds;
    std::vector<std::string> deletedExportLogIds;

    [[nodiscard]] bool empty() const noexcept {
        return deletedActorIds.empty() && deletedPropertyIds.empty() && deletedContractIds.empty() &&
               deletedStatementIds.empty() && deletedTransactionIds.empty() &&
               deletedAnalysisIds.empty() && deletedAnnualIds.empty() &&
               deletedStatementDraftIds.empty() && deletedTransactionDraftIds.empty() &&
               deletedImportLogIds.empty() && deletedExportLogIds.empty();
    }
};

} // namespace core::ports::workspace
