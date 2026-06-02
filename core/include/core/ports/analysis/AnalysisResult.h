/**
 * @file core/include/core/ports/analysis/AnalysisResult.h
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
    std::string actorId;
    std::string contractId;
    std::string contractType;
    std::string contractName;
    std::vector<std::string> propertyIds;
    std::vector<std::string> propertyNames;
    bool allocatable = false;
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

} // namespace core::ports::analysis
