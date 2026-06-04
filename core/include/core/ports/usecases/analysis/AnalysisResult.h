/**
 * @file core/include/core/ports/usecases/analysis/AnalysisResult.h
 * @brief Declares the analysis execution result port contract.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace core::ports::analysis {

struct AnalysisTransaction {
    std::string id;
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string contractId;
    std::string contractType;
    std::vector<std::string> propertyIds;
    std::vector<std::string> propertyNames;
};

struct AnalysisPreviewTransaction {
    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string statementId;
    std::string statementName;
    std::string actorId;
    std::string actorName;
    std::string contractId;
    std::string contractType;
    std::string contractName;
    std::vector<std::string> propertyIds;
    std::vector<std::string> propertyNames;
    bool allocatable = false;
};

struct AnalysisPreviewMetrics {
    int statementCount = 0;
    int transactionCount = 0;
    double amountSum = 0.0;
};

struct AnalysisPreviewResult {
    std::vector<AnalysisPreviewTransaction> transactions;
    AnalysisPreviewMetrics metrics;
};

struct AnalysisResult {
    std::string type;
    std::string configJson;
    std::map<std::string, double> metrics;
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> artifacts;
    std::vector<AnalysisTransaction> transactions;
    bool found = false;
    std::string generatedAt;
};

struct AnalysisTablePropertyRow {
    std::string propertyName;
    std::vector<double> amounts;
    double total = 0.0;
};

struct AnalysisTableState {
    std::vector<std::string> contractTypes;
    std::vector<AnalysisTablePropertyRow> propertyRows;
    double grandTotal = 0.0;
};

} // namespace core::ports::analysis
