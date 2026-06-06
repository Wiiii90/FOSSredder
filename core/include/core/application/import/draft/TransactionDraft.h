/**
 * @file core/include/core/application/import/draft/TransactionDraft.h
 * @brief Draft transaction model used by import finalization and core application flow.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/domain/entities/Transaction.h"

namespace core::application::importing::draft {

class TransactionDraft {
public:
    std::string id;
    std::string statementDraftId;
    int position = 0;

    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;

    std::string actorText;
    std::string propertyText;
    std::string actorId;
    bool actorSelected = false;
    std::string contractId;
    bool contractSelected = false;

    std::string metadata;
    std::vector<uint8_t> proofImageData;

    std::string type;

    bool allocatable = false;
    bool allocatableSelected = false;

    core::domain::Transaction::Status status = core::domain::Transaction::Status::Unverified;

    std::vector<std::string> propertyIds;
    std::string createdAt;
    std::string updatedAt;
};

}
