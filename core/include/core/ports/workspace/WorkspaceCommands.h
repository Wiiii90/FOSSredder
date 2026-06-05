/**
 * @file core/include/core/ports/workspace/WorkspaceCommands.h
 * @brief Write-side boundary command models for workspace operations.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::workspace {

enum class ValidationSeverity {
    Info,
    Warning,
    Error
};

struct ValidationIssue {
    std::string field;
    std::string code;
    std::string message;
    ValidationSeverity severity = ValidationSeverity::Error;
};

struct ValidationResult {
    std::vector<ValidationIssue> issues;

    /**
     * @brief Reports whether the validation result has no errors.
     * @return True when no issue has error severity.
     */
    [[nodiscard]] bool valid() const noexcept {
        for (const auto& issue : issues) {
            if (issue.severity == ValidationSeverity::Error) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Adds an error issue.
     * @param field Field name.
     * @param code Error code.
     * @param message Error message.
     */
    void addError(std::string field, std::string code, std::string message) {
        issues.push_back({std::move(field), std::move(code), std::move(message), ValidationSeverity::Error});
    }
};

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
    /** Raw user-entered amount text; parsed and validated by the core workspace command service. */
    std::string amountText;
    std::string statementId;
    std::string insertAfterTransactionId;
    int status = 0;
    std::string actorId;
    std::string contractId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
};

/**
 * @brief Creates a statement together with its initial transactions.
 */
struct StatementWithTransactionsCommand {
    /** @brief Statement creation data. */
    StatementCommand statement;
    /** @brief Transaction creation data; statement ids are assigned by core. */
    std::vector<TransactionCommand> transactions;
};

struct AnalysisCommand {
    std::string id;
    std::string name;
    std::string type;
    core::ports::analysis::AnalysisConfigInput config;
    core::ports::analysis::AnalysisFilterSelection filter;
    std::string exportFormat;
    bool includeCalculationAdjustments = true;
    std::vector<TransactionSnapshot> snapshotTransactions;
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

struct ImportLogCommand {
    ImportLogSnapshot log;
};

struct ExportLogCommand {
    ExportLogSnapshot log;
};

} // namespace core::ports::workspace
