/**
 * @file core/include/core/ports/workspace/WorkspaceCommands.h
 * @brief Write-side boundary command models for workspace operations.
 */

#pragma once

#include <string>
#include <vector>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::workspace {

struct ActorCommand {
    std::string id;
    std::string name;
    std::vector<AliasSnapshot> aliases;
    std::vector<std::string> contractIds;
};

struct PropertyCommand {
    std::string id;
    std::string name;
    std::vector<AliasSnapshot> aliases;
    std::vector<std::string> contractIds;
};

struct ContractCommand {
    std::string id;
    std::string name;
    std::string type;
    std::string allocatableMode = "mixed";
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<AliasSnapshot> aliases;
};

struct StatementCommand {
    std::string id;
    std::string name;
};

struct TransactionCommand {
    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string statementId;
    std::string insertAfterTransactionId;
    int status = 0;
    std::string actorId;
    std::string contractId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
};

struct AnalysisCommand {
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
};

struct AnnualCommand {
    std::string id;
    std::string name;
    int year = 0;
    std::vector<std::string> assignedAnalysisIds;
};

struct StatementDraftCommand {
    StatementDraftSnapshot draft;
};

struct FinalizeStatementDraftCommand {
    StatementDraftSnapshot draft;
};

struct ImportLogsCommand {
    std::vector<ImportLogSnapshot> logs;
};

struct ImportLogCommand {
    ImportLogSnapshot log;
};

struct ExportLogsCommand {
    std::vector<ExportLogSnapshot> logs;
};

struct ExportLogCommand {
    ExportLogSnapshot log;
};

} // namespace core::ports::workspace
