/**
 * @file core/include/core/application/import/draft/StatementDraft.h
 * @brief Draft statement model used by import finalization and core application flow.
 */

#pragma once

#include <string>
#include <vector>

#include "core/application/import/draft/TransactionDraft.h"

namespace core::application::importing::draft {

class StatementDraft {
public:
    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
    std::vector<TransactionDraft> transactions;
};

}
