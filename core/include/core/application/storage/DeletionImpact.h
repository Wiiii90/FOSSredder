#pragma once

#include <string>
#include <vector>

namespace core::domain {

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

    bool empty() const noexcept {
        return deletedActorIds.empty() && deletedPropertyIds.empty() && deletedContractIds.empty() &&
               deletedStatementIds.empty() && deletedTransactionIds.empty() &&
               deletedAnalysisIds.empty() && deletedAnnualIds.empty() &&
               deletedStatementDraftIds.empty() && deletedTransactionDraftIds.empty() &&
               deletedImportLogIds.empty() && deletedExportLogIds.empty();
    }
};

}

