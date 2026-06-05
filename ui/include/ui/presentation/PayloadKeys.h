/**
 * @file ui/include/ui/presentation/PayloadKeys.h
 * @brief Declarations for the UI PayloadKeys component.
 */

#pragma once

#include <QString>

namespace ui::payload::keys {

namespace common {
inline const auto kId = QStringLiteral("id");
inline const auto kName = QStringLiteral("name");
inline const auto kDisplay = QStringLiteral("display");
inline const auto kLabel = QStringLiteral("label");
inline const auto kValue = QStringLiteral("value");
inline const auto kType = QStringLiteral("type");
inline const auto kDescription = QStringLiteral("description");
inline const auto kAmount = QStringLiteral("amount");
inline const auto kStatus = QStringLiteral("status");
inline const auto kMetadata = QStringLiteral("metadata");
inline const auto kDisplayTime = QStringLiteral("displayTime");
inline const auto kDisplayTitle = QStringLiteral("displayTitle");
inline const auto kDisplayStatusDetail = QStringLiteral("displayStatusDetail");
} // namespace common

namespace actor {
inline const auto kAliases = QStringLiteral("aliases");
}

namespace property {
inline const auto kAddress = QStringLiteral("address");
inline const auto kConsumption = QStringLiteral("consumption");
inline const auto kConsumptionUnit = QStringLiteral("consumptionUnit");
inline const auto kAliases = QStringLiteral("aliases");
} // namespace property

namespace contract {
inline const auto kStartDate = QStringLiteral("startDate");
inline const auto kEndDate = QStringLiteral("endDate");
inline const auto kBasePrice = QStringLiteral("basePrice");
inline const auto kConsumptionPrice = QStringLiteral("consumptionPrice");
inline const auto kMonthlyAdvance = QStringLiteral("monthlyAdvance");
inline const auto kActorIds = QStringLiteral("actorIds");
inline const auto kPropertyIds = QStringLiteral("propertyIds");
inline const auto kAliases = QStringLiteral("aliases");
inline const auto kAllocatableMode = QStringLiteral("allocatableMode");
} // namespace contract

namespace statement {
inline const auto kStatementId = QStringLiteral("statementId");
}

namespace exportLog {
inline const auto kLogId = QStringLiteral("logId");
inline const auto kTime = QStringLiteral("time");
inline const auto kFile = QStringLiteral("file");
inline const auto kMessage = QStringLiteral("message");
} // namespace exportLog

namespace transaction {
inline const auto kBookingDate = QStringLiteral("bookingDate");
inline const auto kValuta = QStringLiteral("valuta");
inline const auto kActorId = QStringLiteral("actorId");
inline const auto kProofImageData = QStringLiteral("proofImageData");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kPropertyIds = QStringLiteral("propertyIds");
inline const auto kContractId = QStringLiteral("contractId");
inline const auto kContractType = QStringLiteral("contractType");
inline const auto kDate = QStringLiteral("date");
} // namespace transaction

namespace analysis {
inline const auto kMetrics = QStringLiteral("metrics");
inline const auto kTable = QStringLiteral("table");
inline const auto kConfig = QStringLiteral("config");
inline const auto kFilter = QStringLiteral("filterSpec");
inline const auto kAdjustments = QStringLiteral("adjustments");
inline const auto kExportFormat = QStringLiteral("exportFormat");
inline const auto kIncludeCalcAdjustments =
    QStringLiteral("includeCalcAdjustments");
inline const auto kSnapshotTransactions =
    QStringLiteral("snapshotTransactions");
inline const auto kCreatedAt = QStringLiteral("createdAt");
inline const auto kUpdatedAt = QStringLiteral("updatedAt");
inline const auto kTransactions = QStringLiteral("transactions");
inline const auto kArtifacts = QStringLiteral("artifacts");
inline const auto kGeneratedAt = QStringLiteral("generatedAt");
} // namespace analysis

namespace annual {
inline const auto kName = QStringLiteral("name");
inline const auto kYear = QStringLiteral("year");
inline const auto kVerificationState = QStringLiteral("verificationState");
inline const auto kAnalysisIds = QStringLiteral("analysisIds");
} // namespace annual

namespace fileSystem {
inline const auto kPath = QStringLiteral("path");
inline const auto kIsDir = QStringLiteral("isDir");
} // namespace fileSystem

namespace importLog {
inline const auto kLogId = QStringLiteral("logId");
inline const auto kTime = QStringLiteral("time");
inline const auto kFile = QStringLiteral("file");
inline const auto kMessage = QStringLiteral("message");
inline const auto kDraftAttached = QStringLiteral("draftAttached");
inline const auto kDraftId = QStringLiteral("draftId");
inline const auto kStatementId = QStringLiteral("statementId");
} // namespace importLog

namespace draft {
inline const auto kActorText = QStringLiteral("actorText");
inline const auto kActorSelected = QStringLiteral("actorSelected");
inline const auto kContractSelected = QStringLiteral("contractSelected");
inline const auto kActorSuggestions = QStringLiteral("actorSuggestions");
inline const auto kPropertySuggestions = QStringLiteral("propertySuggestions");
inline const auto kContractSuggestions = QStringLiteral("contractSuggestions");
inline const auto kAllocatableSelected = QStringLiteral("allocatableSelected");
} // namespace draft

namespace draftView {
inline const auto kProofSource = QStringLiteral("proofSource");
inline const auto kActorSeedText = QStringLiteral("actorSeedText");
inline const auto kActorDisplayText = QStringLiteral("actorDisplayText");
inline const auto kContractSeedText = QStringLiteral("contractSeedText");
inline const auto kContractDisplayText = QStringLiteral("contractDisplayText");
inline const auto kActorSuggestionSummary =
    QStringLiteral("actorSuggestionSummary");
inline const auto kPropertySuggestionSummary =
    QStringLiteral("propertySuggestionSummary");
inline const auto kContractSuggestionSummary =
    QStringLiteral("contractSuggestionSummary");
inline const auto kAllocatableSuggestionSummary =
    QStringLiteral("allocatableSuggestionSummary");
inline const auto kActorSuggestionConfidence =
    QStringLiteral("actorSuggestionConfidence");
inline const auto kPropertySuggestionConfidence =
    QStringLiteral("propertySuggestionConfidence");
inline const auto kContractSuggestionConfidence =
    QStringLiteral("contractSuggestionConfidence");
inline const auto kAllocatableSuggestionConfidence =
    QStringLiteral("allocatableSuggestionConfidence");
inline const auto kEffectiveAllocatable =
    QStringLiteral("effectiveAllocatable");
inline const auto kActorCurrentIndex = QStringLiteral("actorCurrentIndex");
inline const auto kContractCurrentIndex = QStringLiteral("contractCurrentIndex");
inline const auto kActorTopSuggestion = QStringLiteral("actorTopSuggestion");
inline const auto kPropertyTopSuggestion =
    QStringLiteral("propertyTopSuggestion");
inline const auto kContractTopSuggestion =
    QStringLiteral("contractTopSuggestion");
} // namespace draftView

namespace language {
inline const auto kCode = QStringLiteral("code");
inline const auto kLabel = QStringLiteral("label");
inline const auto kAvailable = QStringLiteral("available");
} // namespace language

namespace metrics {
inline const auto kTotal = QStringLiteral("total");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kNonAllocatable = QStringLiteral("nonAllocatable");
} // namespace metrics

namespace state {
inline const auto kRows = QStringLiteral("rows");
inline const auto kOrderIds = QStringLiteral("orderIds");
inline const auto kIndex = QStringLiteral("index");
inline const auto kDraft = QStringLiteral("draft");
inline const auto kDrafts = QStringLiteral("drafts");
inline const auto kAliasInputText = QStringLiteral("aliasInputText");
inline const auto kAliasIndex = QStringLiteral("aliasIndex");
inline const auto kSelectedIds = QStringLiteral("selectedIds");
inline const auto kSelectedActorIds = QStringLiteral("selectedActorIds");
inline const auto kSelectedPropertyIds = QStringLiteral("selectedPropertyIds");
} // namespace state

} // namespace ui::payload::keys
