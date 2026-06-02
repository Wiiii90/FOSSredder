/**
 * @file core/include/core/ports/import/ImportDraft.h
 * @brief Draft statement models exposed through import ports.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::importing::draft {

struct DraftSuggestionCandidate {
    std::string entityId;
    std::string label;
    std::string entityType;
    std::string sourceText;
    std::string rationale;
    std::string lastUsedAt;
    std::vector<std::string> matchedAliases;
    double confidence = 0.0;
    double score = 0.0;
    double aliasWeight = 0.0;
    double recencyWeight = 0.0;
    int hitCount = 0;
};

struct DraftSuggestionBucket {
    std::string sourceText;
    std::vector<DraftSuggestionCandidate> candidates;
};

struct DraftImportSuggestions {
    DraftSuggestionBucket actor;
    DraftSuggestionBucket property;
    DraftSuggestionBucket contract;
};

struct DraftTextSignals {
    std::string sharedText;
    std::string actorText;
    std::string propertyText;
    std::string contractText;
    std::string typeText;
};

struct DraftLinkSelection {
    std::string name;
    std::string metadata;
    std::string actorText;
    std::string propertyText;
    std::string actorId;
    bool actorSelected = false;
    std::string contractId;
    bool contractSelected = false;
    std::string type;
    bool allocatable = false;
    bool allocatableSelected = false;
    std::vector<std::string> propertyIds;
    DraftSuggestionBucket actorSuggestions;
    DraftSuggestionBucket propertySuggestions;
    DraftSuggestionBucket contractSuggestions;
};

struct DraftChoiceRow {
    std::string id;
    std::string name;
    std::string display;
    std::string type;
    std::string allocatableMode;
    std::vector<std::string> aliases;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    bool synthetic = false;
    double confidence = 0.0;
    std::string sourceText;
};

struct DraftDerivedState {
    std::string proofSource;
    std::string actorSeedText;
    std::string actorDisplayText;
    std::string contractSeedText;
    std::string contractDisplayText;
    std::string actorSuggestionSummary;
    std::string propertySuggestionSummary;
    std::string contractSuggestionSummary;
    std::string allocatableSuggestionSummary;
    double actorSuggestionConfidence = 0.0;
    double propertySuggestionConfidence = 0.0;
    double contractSuggestionConfidence = 0.0;
    double allocatableSuggestionConfidence = 0.0;
    bool effectiveAllocatable = false;
    int actorCurrentIndex = -1;
    int contractCurrentIndex = -1;
    DraftSuggestionCandidate actorTopSuggestion;
    DraftSuggestionCandidate propertyTopSuggestion;
    DraftSuggestionCandidate contractTopSuggestion;
    bool hasActorTopSuggestion = false;
    bool hasPropertyTopSuggestion = false;
    bool hasContractTopSuggestion = false;
    std::vector<DraftChoiceRow> actorChoices;
    std::vector<DraftChoiceRow> contractChoices;
    std::vector<DraftChoiceRow> propertyRows;
    std::vector<std::string> autoPropertyIds;
};

enum class DraftAutoSelectionMode { InitialImport, InteractiveSync };

struct TransactionDraftPatch {
    bool hasName = false;
    std::string name;
    bool hasBookingDate = false;
    std::string bookingDate;
    bool hasValuta = false;
    std::string valuta;
    bool hasAmount = false;
    double amount = 0.0;
    bool hasStatus = false;
    int status = 0;
    bool hasAllocatable = false;
    bool allocatable = false;
    bool allocatableSelected = false;
};

struct TransactionDraft {
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
    std::vector<std::uint8_t> proofImageData;

    std::string type;

    bool allocatable = false;
    bool allocatableSelected = false;

    int status = 0;

    std::vector<std::string> propertyIds;
    std::string createdAt;
    std::string updatedAt;
};

struct StatementDraft {
    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
    std::vector<TransactionDraft> transactions;
};

} // namespace core::ports::importing::draft
