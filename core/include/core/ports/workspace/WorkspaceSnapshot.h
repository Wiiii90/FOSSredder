/**
 * @file core/include/core/ports/workspace/WorkspaceSnapshot.h
 * @brief Immutable workspace read-model snapshot exposed through workspace ports.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/ports/usecases/analysis/AnalysisRequest.h"

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
    std::string contractType;
    std::string actorId;
    std::string statementId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
    std::vector<std::string> propertyNames;
    std::string createdAt;
    std::string updatedAt;
};

struct AnalysisSnapshot {
    std::string id;
    std::string name;
    std::string type;
    core::ports::analysis::AnalysisConfigInput config;
    core::ports::analysis::AnalysisFilterSelection filter;
    std::string exportFormat;
    bool includeCalculationAdjustments = true;
    std::vector<TransactionSnapshot> snapshotTransactions;
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
    std::vector<std::string> annualIds;
    std::vector<std::string> analysisIds;
};

struct WorkspaceIdentitySnapshot {
    std::string id;
    std::string name;
    std::string type;
    std::string allocatableMode;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;

    /**
     * @brief Reports whether the identity has no id.
     * @return True when the id is empty.
     */
    [[nodiscard]] bool empty() const noexcept {
        return id.empty();
    }
};

/**
 * @brief Transaction catalog ids selected by a UI form or workspace query.
 */
struct TransactionCatalogSelection {
    /** @brief Selected actor id. */
    std::string actorId;
    /** @brief Selected contract id. */
    std::string contractId;
    /** @brief Selected property ids. */
    std::vector<std::string> propertyIds;
};

/**
 * @brief Catalog selection change request for transaction forms.
 */
struct TransactionCatalogSelectionChange {
    /** @brief Current transaction catalog selection. */
    TransactionCatalogSelection current;
    /** @brief True when contractId contains a user-selected contract. */
    bool contractChanged = false;
    /** @brief True when actorId contains a user-selected actor. */
    bool actorChanged = false;
    /** @brief True when propertyIds contains user-selected properties. */
    bool propertiesChanged = false;
    /** @brief New contract id when contractChanged is true. */
    std::string contractId;
    /** @brief New actor id when actorChanged is true. */
    std::string actorId;
    /** @brief New property ids when propertiesChanged is true. */
    std::vector<std::string> propertyIds;
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

    /**
     * @brief Checks whether all snapshot collections are empty and no file is open.
     * @return True when no workspace data is present.
     */
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

    /**
     * @brief Reports whether no ids were deleted.
     * @return True when all deletion lists are empty.
     */
    [[nodiscard]] bool empty() const noexcept {
        return deletedActorIds.empty() && deletedPropertyIds.empty() && deletedContractIds.empty() &&
               deletedStatementIds.empty() && deletedTransactionIds.empty() &&
               deletedAnalysisIds.empty() && deletedAnnualIds.empty() &&
               deletedStatementDraftIds.empty() && deletedTransactionDraftIds.empty() &&
               deletedImportLogIds.empty() && deletedExportLogIds.empty();
    }
};

} // namespace core::ports::workspace
