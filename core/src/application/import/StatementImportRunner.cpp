/**
 * @file core/src/application/import/StatementImportRunner.cpp
 * @brief Implements the application-level asynchronous statement import runner.
 */

#include "core/application/import/StatementImportRunner.h"

#include "core/application/import/IImportStatement.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/import/transaction/AmountParser.h"
#include "core/application/workspace/WorkspaceSnapshotCatalogMapper.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/policies/TransactionPolicy.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "core/errors/IErrorReporter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace core::application::importing {
namespace {

constexpr int kKeepImportRuns = 20;
constexpr auto kRunNameImport = "import";

std::string normalizedAllocatableMode(std::string mode) {
  mode = core::domain::policies::transaction::trimCopy(std::move(mode));
  std::transform(mode.begin(), mode.end(), mode.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return mode;
}

std::string fallbackDraftTransactionId() {
  static thread_local std::mt19937_64 generator{std::random_device{}()};
  std::uniform_int_distribution<std::uint64_t> distribution;
  std::ostringstream out;
  out << std::hex << distribution(generator) << distribution(generator);
  return out.str();
}

std::string fileStemFromPath(const std::string &path) {
  try {
    return std::filesystem::path(path).stem().string();
  } catch (...) {
    return {};
  }
}

core::ports::importing::StatementImportState
toStatementImportState(core::jobs::JobState state) {
  switch (state) {
  case core::jobs::JobState::Running:
    return core::ports::importing::StatementImportState::Running;
  case core::jobs::JobState::Paused:
    return core::ports::importing::StatementImportState::Paused;
  case core::jobs::JobState::Finished:
    return core::ports::importing::StatementImportState::Finished;
  case core::jobs::JobState::Failed:
    return core::ports::importing::StatementImportState::Failed;
  case core::jobs::JobState::Canceled:
    return core::ports::importing::StatementImportState::Canceled;
  case core::jobs::JobState::Pending:
  default:
    return core::ports::importing::StatementImportState::Pending;
  }
}

core::ports::workspace::StatementSnapshot
toStatementSnapshot(const std::shared_ptr<core::domain::Statement> &statement) {
  core::ports::workspace::StatementSnapshot out;
  if (!statement)
    return out;
  out.id = statement->id();
  out.name = statement->name();
  out.transactionIds = statement->transactionIds();
  out.createdAt = statement->createdAt();
  out.updatedAt = statement->updatedAt();
  return out;
}

core::ports::importing::draft::TransactionDraft
toPortDraft(const core::application::importing::draft::TransactionDraft &draft) {
  core::ports::importing::draft::TransactionDraft out;
  out.id = draft.id;
  out.statementDraftId = draft.statementDraftId;
  out.position = draft.position;
  out.name = draft.name;
  out.bookingDate = draft.bookingDate;
  out.valuta = draft.valuta;
  out.amount = draft.amount;
  out.actorText = draft.actorText;
  out.propertyText = draft.propertyText;
  out.actorId = draft.actorId;
  out.actorSelected = draft.actorSelected;
  out.contractId = draft.contractId;
  out.contractSelected = draft.contractSelected;
  out.metadata = draft.metadata;
  out.proofImageData = draft.proofImageData;
  out.type = draft.type;
  out.allocatable = draft.allocatable;
  out.allocatableSelected = draft.allocatableSelected;
  out.status = static_cast<int>(draft.status);
  out.propertyIds = draft.propertyIds;
  out.createdAt = draft.createdAt;
  out.updatedAt = draft.updatedAt;
  return out;
}

core::application::importing::draft::TransactionDraft
toCoreDraft(const core::ports::importing::draft::TransactionDraft &draft) {
  core::application::importing::draft::TransactionDraft out;
  out.id = draft.id;
  out.statementDraftId = draft.statementDraftId;
  out.position = draft.position;
  out.name = draft.name;
  out.bookingDate = draft.bookingDate;
  out.valuta = draft.valuta;
  out.amount = draft.amount;
  out.actorText = draft.actorText;
  out.propertyText = draft.propertyText;
  out.actorId = draft.actorId;
  out.actorSelected = draft.actorSelected;
  out.contractId = draft.contractId;
  out.contractSelected = draft.contractSelected;
  out.metadata = draft.metadata;
  out.proofImageData = draft.proofImageData;
  out.type = draft.type;
  out.allocatable = draft.allocatable;
  out.allocatableSelected = draft.allocatableSelected;
  out.status = static_cast<core::domain::Transaction::Status>(draft.status);
  out.propertyIds = draft.propertyIds;
  out.createdAt = draft.createdAt;
  out.updatedAt = draft.updatedAt;
  return out;
}

core::ports::importing::draft::StatementDraft toPortStatementDraft(
    const core::application::importing::draft::StatementDraft &draft) {
  core::ports::importing::draft::StatementDraft out;
  out.id = draft.id;
  out.name = draft.name;
  out.transactionIds = draft.transactionIds;
  out.createdAt = draft.createdAt;
  out.updatedAt = draft.updatedAt;
  out.transactions.reserve(draft.transactions.size());
  for (const auto &transaction : draft.transactions) {
    out.transactions.push_back(toPortDraft(transaction));
  }
  return out;
}

core::application::importing::draft::StatementDraft toCoreStatementDraft(
    const core::ports::importing::draft::StatementDraft &draft) {
  core::application::importing::draft::StatementDraft out;
  out.id = draft.id;
  out.name = draft.name;
  out.transactionIds = draft.transactionIds;
  out.createdAt = draft.createdAt;
  out.updatedAt = draft.updatedAt;
  out.transactions.reserve(draft.transactions.size());
  for (const auto &transaction : draft.transactions) {
    out.transactions.push_back(toCoreDraft(transaction));
  }
  return out;
}

core::application::importing::draft::DraftSuggestionBucket toCoreBucket(
    const core::ports::importing::draft::DraftSuggestionBucket &bucket) {
  core::application::importing::draft::DraftSuggestionBucket out;
  out.sourceText = bucket.sourceText;
  out.candidates.reserve(bucket.candidates.size());
  for (const auto &candidate : bucket.candidates) {
    core::application::importing::draft::DraftSuggestionCandidate item;
    item.entityId = candidate.entityId;
    item.label = candidate.label;
    item.entityType = candidate.entityType;
    item.sourceText = candidate.sourceText;
    item.rationale = candidate.rationale;
    item.lastUsedAt = candidate.lastUsedAt;
    item.matchedAliases = candidate.matchedAliases;
    item.confidence = candidate.confidence;
    item.score = candidate.score;
    item.aliasWeight = candidate.aliasWeight;
    item.recencyWeight = candidate.recencyWeight;
    item.hitCount = candidate.hitCount;
    out.candidates.push_back(std::move(item));
  }
  return out;
}

core::ports::importing::draft::DraftSuggestionBucket toPortBucket(
    const core::application::importing::draft::DraftSuggestionBucket &bucket) {
  core::ports::importing::draft::DraftSuggestionBucket out;
  out.sourceText = bucket.sourceText;
  out.candidates.reserve(bucket.candidates.size());
  for (const auto &candidate : bucket.candidates) {
    core::ports::importing::draft::DraftSuggestionCandidate item;
    item.entityId = candidate.entityId;
    item.label = candidate.label;
    item.entityType = candidate.entityType;
    item.sourceText = candidate.sourceText;
    item.rationale = candidate.rationale;
    item.lastUsedAt = candidate.lastUsedAt;
    item.matchedAliases = candidate.matchedAliases;
    item.confidence = candidate.confidence;
    item.score = candidate.score;
    item.aliasWeight = candidate.aliasWeight;
    item.recencyWeight = candidate.recencyWeight;
    item.hitCount = candidate.hitCount;
    out.candidates.push_back(std::move(item));
  }
  return out;
}

core::application::importing::draft::DraftLinkSelection toCoreSelection(
    const core::ports::importing::draft::DraftLinkSelection &selection) {
  core::application::importing::draft::DraftLinkSelection out;
  out.name = selection.name;
  out.metadata = selection.metadata;
  out.actorText = selection.actorText;
  out.propertyText = selection.propertyText;
  out.actorId = selection.actorId;
  out.actorSelected = selection.actorSelected;
  out.contractId = selection.contractId;
  out.contractSelected = selection.contractSelected;
  out.type = selection.type;
  out.allocatable = selection.allocatable;
  out.allocatableSelected = selection.allocatableSelected;
  out.propertyIds = selection.propertyIds;
  out.actorSuggestions = toCoreBucket(selection.actorSuggestions);
  out.propertySuggestions = toCoreBucket(selection.propertySuggestions);
  out.contractSuggestions = toCoreBucket(selection.contractSuggestions);
  return out;
}

core::ports::importing::draft::DraftDerivedState toPortDerivedState(
    const core::application::importing::draft::DraftDerivedState &derived) {
  core::ports::importing::draft::DraftDerivedState out;
  out.proofSource = derived.proofSource;
  out.actorSeedText = derived.actorSeedText;
  out.actorDisplayText = derived.actorDisplayText;
  out.contractSeedText = derived.contractSeedText;
  out.contractDisplayText = derived.contractDisplayText;
  out.actorSuggestionSummary = derived.actorSuggestionSummary;
  out.propertySuggestionSummary = derived.propertySuggestionSummary;
  out.contractSuggestionSummary = derived.contractSuggestionSummary;
  out.allocatableSuggestionSummary = derived.allocatableSuggestionSummary;
  out.actorSuggestionConfidence = derived.actorSuggestionConfidence;
  out.propertySuggestionConfidence = derived.propertySuggestionConfidence;
  out.contractSuggestionConfidence = derived.contractSuggestionConfidence;
  out.allocatableSuggestionConfidence = derived.allocatableSuggestionConfidence;
  out.effectiveAllocatable = derived.effectiveAllocatable;
  out.actorCurrentIndex = derived.actorCurrentIndex;
  out.contractCurrentIndex = derived.contractCurrentIndex;
  out.actorTopSuggestion =
      toPortBucket({{}, {derived.actorTopSuggestion}}).candidates.front();
  out.propertyTopSuggestion =
      toPortBucket({{}, {derived.propertyTopSuggestion}}).candidates.front();
  out.contractTopSuggestion =
      toPortBucket({{}, {derived.contractTopSuggestion}}).candidates.front();
  out.hasActorTopSuggestion = derived.hasActorTopSuggestion;
  out.hasPropertyTopSuggestion = derived.hasPropertyTopSuggestion;
  out.hasContractTopSuggestion = derived.hasContractTopSuggestion;
  for (const auto &row : derived.actorChoices)
    out.actorChoices.push_back({row.id, row.name, row.display, row.type,
                                row.allocatableMode, row.aliases, row.actorIds,
                                row.propertyIds, row.synthetic, row.confidence,
                                row.sourceText});
  for (const auto &row : derived.contractChoices)
    out.contractChoices.push_back({row.id, row.name, row.display, row.type,
                                   row.allocatableMode, row.aliases,
                                   row.actorIds, row.propertyIds,
                                   row.synthetic, row.confidence,
                                   row.sourceText});
  for (const auto &row : derived.propertyRows)
    out.propertyRows.push_back({row.id, row.name, row.display, row.type,
                                row.allocatableMode, row.aliases, row.actorIds,
                                row.propertyIds, row.synthetic, row.confidence,
                                row.sourceText});
  out.autoPropertyIds = derived.autoPropertyIds;
  return out;
}

core::ports::importing::draft::DraftLinkSelection toPortSelection(
    const core::ports::importing::draft::TransactionDraft &draft) {
  core::ports::importing::draft::DraftLinkSelection out;
  out.name = draft.name;
  out.metadata = draft.metadata;
  out.actorText = draft.actorText;
  out.propertyText = draft.propertyText;
  out.actorId = draft.actorId;
  out.actorSelected = draft.actorSelected;
  out.contractId = draft.contractId;
  out.contractSelected = draft.contractSelected;
  out.type = draft.type;
  out.allocatable = draft.allocatable;
  out.allocatableSelected = draft.allocatableSelected;
  out.propertyIds = draft.propertyIds;
  return out;
}

core::application::importing::draft::DraftSuggestionCandidate toCoreCandidate(
    const core::ports::importing::draft::DraftSuggestionCandidate &candidate) {
  return {candidate.entityId,
          candidate.label,
          candidate.entityType,
          candidate.sourceText,
          candidate.rationale,
          candidate.lastUsedAt,
          candidate.matchedAliases,
          candidate.confidence,
          candidate.score,
          candidate.aliasWeight,
          candidate.recencyWeight,
          candidate.hitCount};
}

core::application::importing::draft::DraftChoiceRow toCoreChoiceRow(
    const core::ports::importing::draft::DraftChoiceRow &row) {
  return {row.id,          row.name,      row.display,   row.type,
          row.allocatableMode, row.aliases, row.actorIds, row.propertyIds,
          row.synthetic,   row.confidence, row.sourceText};
}

core::application::importing::draft::DraftDerivedState toCoreDerivedState(
    const core::ports::importing::draft::DraftDerivedState &derived) {
  core::application::importing::draft::DraftDerivedState out;
  out.proofSource = derived.proofSource;
  out.actorSeedText = derived.actorSeedText;
  out.actorDisplayText = derived.actorDisplayText;
  out.contractSeedText = derived.contractSeedText;
  out.contractDisplayText = derived.contractDisplayText;
  out.actorSuggestionSummary = derived.actorSuggestionSummary;
  out.propertySuggestionSummary = derived.propertySuggestionSummary;
  out.contractSuggestionSummary = derived.contractSuggestionSummary;
  out.allocatableSuggestionSummary = derived.allocatableSuggestionSummary;
  out.actorSuggestionConfidence = derived.actorSuggestionConfidence;
  out.propertySuggestionConfidence = derived.propertySuggestionConfidence;
  out.contractSuggestionConfidence = derived.contractSuggestionConfidence;
  out.allocatableSuggestionConfidence = derived.allocatableSuggestionConfidence;
  out.effectiveAllocatable = derived.effectiveAllocatable;
  out.actorCurrentIndex = derived.actorCurrentIndex;
  out.contractCurrentIndex = derived.contractCurrentIndex;
  out.actorTopSuggestion = toCoreCandidate(derived.actorTopSuggestion);
  out.propertyTopSuggestion = toCoreCandidate(derived.propertyTopSuggestion);
  out.contractTopSuggestion = toCoreCandidate(derived.contractTopSuggestion);
  out.hasActorTopSuggestion = derived.hasActorTopSuggestion;
  out.hasPropertyTopSuggestion = derived.hasPropertyTopSuggestion;
  out.hasContractTopSuggestion = derived.hasContractTopSuggestion;
  for (const auto &row : derived.actorChoices) {
    out.actorChoices.push_back(toCoreChoiceRow(row));
  }
  for (const auto &row : derived.contractChoices) {
    out.contractChoices.push_back(toCoreChoiceRow(row));
  }
  for (const auto &row : derived.propertyRows) {
    out.propertyRows.push_back(toCoreChoiceRow(row));
  }
  out.autoPropertyIds = derived.autoPropertyIds;
  return out;
}

core::application::importing::draft::DraftAutoSelectionMode
toCoreAutoSelectionMode(
    core::ports::importing::draft::DraftAutoSelectionMode mode) {
  switch (mode) {
  case core::ports::importing::draft::DraftAutoSelectionMode::InteractiveSync:
    return core::application::importing::draft::DraftAutoSelectionMode::
        InteractiveSync;
  case core::ports::importing::draft::DraftAutoSelectionMode::InitialImport:
  default:
    return core::application::importing::draft::DraftAutoSelectionMode::
        InitialImport;
  }
}

core::application::importing::draft::TransactionDraftPatch toCorePatch(
    const core::ports::importing::draft::TransactionDraftPatch &patch) {
  core::application::importing::draft::TransactionDraftPatch out;
  out.hasName = patch.hasName;
  out.name = patch.name;
  out.hasBookingDate = patch.hasBookingDate;
  out.bookingDate = patch.bookingDate;
  out.hasValuta = patch.hasValuta;
  out.valuta = patch.valuta;
  out.hasAmount = patch.hasAmount;
  out.amount = patch.amount;
  out.hasStatus = patch.hasStatus;
  out.status = patch.status;
  out.hasAllocatable = patch.hasAllocatable;
  out.allocatable = patch.allocatable;
  out.allocatableSelected = patch.allocatableSelected;
  return out;
}

core::ports::workspace::WorkspaceSnapshot
mergeSnapshots(core::ports::workspace::WorkspaceSnapshot primary,
               const core::ports::workspace::WorkspaceSnapshot &secondary) {
  if (primary.actors.empty())
    primary.actors = secondary.actors;
  if (primary.properties.empty())
    primary.properties = secondary.properties;
  if (primary.contracts.empty())
    primary.contracts = secondary.contracts;
  if (primary.statements.empty())
    primary.statements = secondary.statements;
  if (primary.transactions.empty())
    primary.transactions = secondary.transactions;
  if (primary.analyses.empty())
    primary.analyses = secondary.analyses;
  if (primary.annuals.empty())
    primary.annuals = secondary.annuals;
  if (primary.statementDrafts.empty())
    primary.statementDrafts = secondary.statementDrafts;
  if (primary.importLogs.empty())
    primary.importLogs = secondary.importLogs;
  if (primary.exportLogs.empty())
    primary.exportLogs = secondary.exportLogs;
  if (!primary.hasCurrentPath && secondary.hasCurrentPath) {
    primary.currentPath = secondary.currentPath;
    primary.hasCurrentPath = true;
  }
  return primary;
}

std::filesystem::path defaultRunBasePath() {
  return std::filesystem::temp_directory_path() / "fossredder" / "import-runs";
}

std::string utcTimestamp() {
  const auto now = std::chrono::system_clock::now();
  const auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch()) %
      1000;
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  gmtime_s(&tm, &time);
#else
  gmtime_r(&time, &tm);
#endif
  std::ostringstream out;
  out << std::put_time(&tm, "%Y%m%d%H%M%S") << std::setw(3)
      << std::setfill('0') << millis.count();
  return out.str();
}

bool isImportRunDir(const std::filesystem::directory_entry &entry) {
  if (!entry.is_directory())
    return false;
  const auto name = entry.path().filename().string();
  return name.find(std::string("_") + kRunNameImport + "_") !=
         std::string::npos;
}

void cleanupOldImportRuns(const std::filesystem::path &basePath) {
  if (!std::filesystem::exists(basePath))
    return;

  std::vector<std::filesystem::directory_entry> entries;
  for (const auto &entry : std::filesystem::directory_iterator(basePath)) {
    if (isImportRunDir(entry))
      entries.push_back(entry);
  }

  std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
    return a.last_write_time() > b.last_write_time();
  });

  for (std::size_t i = kKeepImportRuns; i < entries.size(); ++i) {
    std::error_code ec;
    std::filesystem::remove_all(entries[i].path(), ec);
  }
}

core::jobs::ImportStatementJobSpec
buildImportSpec(const core::ports::importing::ImportRequest &request,
                const std::filesystem::path &basePath) {
  cleanupOldImportRuns(basePath);
  std::filesystem::create_directories(basePath);

  const auto timestamp = utcTimestamp();
  int suffix = 1;
  while (true) {
    const auto runName = timestamp + "_" + kRunNameImport + "_" +
                         std::to_string(suffix);
    const auto runRoot = basePath / runName;
    if (!std::filesystem::exists(runRoot)) {
      std::filesystem::create_directories(runRoot);
      return {request.sourcePath, runRoot.string(), timestamp};
    }
    ++suffix;
  }
}

} // namespace

class StatementImportRunner::Impl {
public:
  Impl(std::shared_ptr<core::jobs::JobSystem> jobSystem,
       std::string runBasePath,
       std::shared_ptr<core::errors::IErrorReporter> errorReporter)
      : jobSystem(std::move(jobSystem)),
        runBasePath(runBasePath.empty() ? defaultRunBasePath()
                                        : std::filesystem::path(runBasePath)),
        errorReporter(std::move(errorReporter)) {}

  std::shared_ptr<core::jobs::JobSystem> jobSystem;
  std::filesystem::path runBasePath;
  std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

StatementImportRunner::StatementImportRunner(
    std::shared_ptr<IImportStatement> importService, std::string runBasePath,
    std::shared_ptr<core::errors::IErrorReporter> errorReporter)
    : StatementImportRunner(
          std::make_shared<core::jobs::JobSystem>(std::move(importService)),
          std::move(runBasePath), std::move(errorReporter)) {}

StatementImportRunner::StatementImportRunner(
    std::shared_ptr<core::jobs::JobSystem> jobSystem, std::string runBasePath,
    std::shared_ptr<core::errors::IErrorReporter> errorReporter)
    : impl_(std::make_unique<Impl>(std::move(jobSystem), std::move(runBasePath),
                                   std::move(errorReporter))) {}

StatementImportRunner::~StatementImportRunner() = default;

core::ports::importing::StatementImportHandle
StatementImportRunner::startStatementImport(
    const core::ports::importing::ImportRequest &request,
    core::ports::importing::StatementImportEventCallback callback) {
  if (!impl_->jobSystem)
    return {};

  try {
    const auto spec = buildImportSpec(request, impl_->runBasePath);
    const auto importId = impl_->jobSystem->startImportStatement(spec);
    const auto subscriptionId = impl_->jobSystem->subscribe(
        importId, [callback = std::move(callback)](
                      const core::jobs::JobEvent &event) {
          if (!callback)
            return;
          callback({event.jobId, toStatementImportState(event.state),
                    event.progress, event.message});
        });
    return {importId, subscriptionId};
  } catch (const std::exception &) {
    if (impl_->errorReporter) {
      impl_->errorReporter->reportException(
          core::errors::ErrorSeverity::Error,
          "core::application::importing::StatementImportRunner::startStatementImport",
          std::current_exception());
    }
    return {};
  } catch (...) {
    if (impl_->errorReporter) {
      impl_->errorReporter->reportException(
          core::errors::ErrorSeverity::Error,
          "core::application::importing::StatementImportRunner::startStatementImport",
          std::current_exception());
    }
    return {};
  }
}

void StatementImportRunner::unsubscribe(
    const core::ports::importing::StatementImportHandle &handle) {
  if (impl_->jobSystem && !handle.importId.empty() && handle.subscriptionId != 0)
    impl_->jobSystem->unsubscribe(handle.importId, handle.subscriptionId);
}

void StatementImportRunner::cancel(
    const core::ports::importing::StatementImportHandle &handle) {
  if (impl_->jobSystem && !handle.importId.empty())
    impl_->jobSystem->cancel(handle.importId);
}

void StatementImportRunner::pause(
    const core::ports::importing::StatementImportHandle &handle) {
  if (impl_->jobSystem && !handle.importId.empty())
    impl_->jobSystem->pause(handle.importId);
}

void StatementImportRunner::resume(
    const core::ports::importing::StatementImportHandle &handle) {
  if (impl_->jobSystem && !handle.importId.empty())
    impl_->jobSystem->resume(handle.importId);
}

core::ports::importing::ImportResult
StatementImportRunner::importResult(
    const core::ports::importing::StatementImportHandle &handle) {
  core::ports::importing::ImportResult result;
  if (!impl_->jobSystem || handle.importId.empty())
    return result;

  result.statement =
      toStatementSnapshot(impl_->jobSystem->statementResult(handle.importId));
  result.hasStatement =
      !result.statement.id.empty() || !result.statement.name.empty();
  const auto transactions =
      impl_->jobSystem->statementTransactions(handle.importId);
  result.transactions.reserve(transactions.size());
  for (const auto &transaction : transactions) {
    result.transactions.push_back(toPortDraft(transaction));
  }
  result.artifacts =
      impl_->jobSystem->takeStatementArtifacts(handle.importId);
  return result;
}

core::ports::importing::draft::DraftDerivedState
StatementImportRunner::buildDraftDerivedState(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::DraftLinkSelection &selection) const {
  return toPortDerivedState(draft::buildDraftDerivedState(
      core::application::workspace::toWorkspaceCatalog(state),
      toCoreSelection(selection)));
}

bool StatementImportRunner::updateTransactionDraft(
    core::ports::importing::draft::TransactionDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::TransactionDraftEdit &edit) const {
  auto coreDraft = toCoreDraft(draft);
  bool changed = false;
  switch (edit.kind) {
  case core::ports::importing::draft::TransactionDraftEditKind::Patch:
    changed = draft::applyTransactionPatch(coreDraft, toCorePatch(edit.patch));
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::ApplyAmountText:
    if (const auto parsed = transaction::parseAmountString(edit.text)) {
      core::application::importing::draft::TransactionDraftPatch patch;
      patch.hasAmount = true;
      patch.amount = *parsed;
      changed = draft::applyTransactionPatch(coreDraft, patch);
    }
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::SelectActor:
    changed = draft::applyActorSelection(coreDraft, edit.id);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::ClearActor:
    changed = draft::clearActorSelection(coreDraft);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::ApplyProperty:
    changed = draft::applyPropertySelection(coreDraft, edit.id);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::
      SetPropertySelected:
    changed = draft::setPropertySelected(coreDraft, edit.id, edit.selected);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::SelectContract:
    changed = draft::applyContractSelection(coreDraft,
                                            toCoreChoiceRow(edit.choice));
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::
      SelectContractFromCatalog:
    changed = draft::applyContractSelection(
        coreDraft, core::application::workspace::toWorkspaceCatalog(state),
        edit.id);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::ClearContract:
    changed = draft::clearContractSelection(coreDraft);
    break;
  case core::ports::importing::draft::TransactionDraftEditKind::
      ApplyDerivedSelections:
    changed = draft::applyDerivedSelections(
        coreDraft, toCoreDerivedState(edit.derived),
        toCoreAutoSelectionMode(edit.autoSelectionMode));
    break;
  }
  if (!changed) {
    return false;
  }
  draft = toPortDraft(coreDraft);
  return true;
}

core::ports::importing::draft::StatementDraftEditResult
StatementImportRunner::updateStatementDraft(
    core::ports::importing::draft::StatementDraft &draft,
    const core::ports::importing::draft::StatementDraftEdit &edit) const {
  auto coreDraft = toCoreStatementDraft(draft);
  core::ports::importing::draft::StatementDraftEditResult result;
  switch (edit.kind) {
  case core::ports::importing::draft::StatementDraftEditKind::Rename:
    result.changed = draft::renameStatementDraft(coreDraft, edit.text);
    result.selectedTransactionIndex = edit.index;
    break;
  case core::ports::importing::draft::StatementDraftEditKind::
      InsertTransactionAfter:
    result.selectedTransactionIndex =
        draft::insertTransactionAfter(coreDraft, edit.index);
    result.changed = result.selectedTransactionIndex >= 0;
    break;
  case core::ports::importing::draft::StatementDraftEditKind::
      RemoveTransactionAt:
    result.selectedTransactionIndex =
        draft::removeTransactionAt(coreDraft, edit.index);
    result.changed = result.selectedTransactionIndex >= 0;
    break;
  }
  if (result.changed) {
    draft = toPortStatementDraft(coreDraft);
  }
  return result;
}

core::ports::importing::draft::StatementDraft
StatementImportRunner::buildStatementDraft(
    const std::string &sourceFile,
    const core::ports::workspace::StatementSnapshot &statement,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::vector<core::ports::importing::draft::TransactionDraft>
        &transactions,
    const std::string &draftId) const {
  core::ports::importing::draft::StatementDraft draft;
  draft.id = draftId;
  draft.name = !statement.name.empty() ? statement.name : fileStemFromPath(sourceFile);
  draft.transactionIds = statement.transactionIds;
  draft.createdAt = statement.createdAt;
  draft.updatedAt = statement.updatedAt;
  draft.transactions.reserve(transactions.size());

  for (std::size_t i = 0; i < transactions.size(); ++i) {
    auto transaction = transactions[i];
    const auto signals = draft::buildDraftTextSignals(
        core::application::workspace::toWorkspaceCatalog(state),
        toCoreDraft(transaction));
    if (transaction.id.empty()) {
      transaction.id = fallbackDraftTransactionId();
    }
    transaction.statementDraftId = draft.id;
    transaction.position = static_cast<int>(i);
    transaction.name = "Transaction " + std::to_string(i + 1);
    transaction.actorText = signals.actorText;
    transaction.propertyText = signals.propertyText;
    transaction.type = signals.typeText;

    core::ports::importing::draft::TransactionDraftEdit edit;
    edit.kind = core::ports::importing::draft::TransactionDraftEditKind::
        ApplyDerivedSelections;
    edit.derived = buildDraftDerivedState(state, toPortSelection(transaction));
    edit.autoSelectionMode =
        core::ports::importing::draft::DraftAutoSelectionMode::InitialImport;
    updateTransactionDraft(transaction, state, edit);
    draft.transactionIds.push_back(transaction.id);
    draft.transactions.push_back(std::move(transaction));
  }

  return draft;
}

core::ports::importing::draft::StatementDraft
StatementImportRunner::restoreStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot &snapshot) const {
  core::ports::importing::draft::StatementDraft draft;
  draft.id = snapshot.id;
  draft.name = snapshot.name;
  draft.transactionIds = snapshot.transactionIds;
  draft.createdAt = snapshot.createdAt;
  draft.updatedAt = snapshot.updatedAt;
  draft.transactions.reserve(snapshot.transactions.size());

  for (const auto &tx : snapshot.transactions) {
    core::ports::importing::draft::TransactionDraft transaction;
    transaction.id = tx.id;
    transaction.statementDraftId = tx.statementDraftId;
    transaction.name = tx.name;
    transaction.bookingDate = tx.bookingDate;
    transaction.valuta = tx.valuta;
    transaction.amount = tx.amount;
    transaction.actorId = tx.actorId;
    transaction.contractId = tx.contractId;
    transaction.propertyIds = tx.propertyIds;
    transaction.status = tx.status;
    transaction.allocatable = tx.allocatable;
    transaction.position = tx.position;
    transaction.metadata = tx.metadata;
    transaction.proofImageData = tx.proofImageData;
    draft.transactions.push_back(std::move(transaction));
  }

  std::sort(draft.transactions.begin(), draft.transactions.end(),
            [](const auto &lhs, const auto &rhs) {
              return lhs.position < rhs.position;
            });
  if (draft.transactionIds.empty()) {
    draft.transactionIds.reserve(draft.transactions.size());
    for (const auto &transaction : draft.transactions) {
      draft.transactionIds.push_back(transaction.id);
    }
  }
  return draft;
}

core::ports::workspace::StatementDraftSnapshot
StatementImportRunner::buildStatementDraftSnapshot(
    const core::ports::importing::draft::StatementDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state) const {
  core::ports::workspace::StatementDraftSnapshot input;
  input.id = draft.id;
  input.name = draft.name;
  input.createdAt = draft.createdAt;
  input.updatedAt = draft.updatedAt;
  input.transactions.reserve(draft.transactions.size());

  for (const auto &draftTransaction : draft.transactions) {
    core::ports::workspace::TransactionDraftSnapshot transaction;
    transaction.id = draftTransaction.id;
    transaction.statementDraftId = input.id;
    transaction.name = draftTransaction.name;
    transaction.bookingDate = draftTransaction.bookingDate;
    transaction.valuta = draftTransaction.valuta;
    transaction.amount = draftTransaction.amount;
    transaction.proofImageData = draftTransaction.proofImageData;
    transaction.status = draftTransaction.status;
    transaction.actorId = draftTransaction.actorId;
    transaction.contractId = draftTransaction.contractId;

    bool inferredAllocatable = draft::contractIsFullyAllocatable(
        core::application::workspace::toWorkspaceCatalog(state),
        draftTransaction.contractId);
    if (!draftTransaction.contractId.empty()) {
      for (const auto &contract : state.contracts) {
        if (contract.id != draftTransaction.contractId) {
          continue;
        }
        const auto mode = normalizedAllocatableMode(contract.allocatableMode);
        if (mode == "allocatable") {
          inferredAllocatable = true;
        } else if (mode == "non-allocatable") {
          inferredAllocatable = false;
        }
        break;
      }
    }
    transaction.allocatable =
        draftTransaction.allocatableSelected
            ? draftTransaction.allocatable
            : (inferredAllocatable || draftTransaction.allocatable);
    transaction.propertyIds = draftTransaction.propertyIds;
    transaction.position = static_cast<int>(input.transactions.size());
    input.transactionIds.push_back(transaction.id);
    input.transactions.push_back(std::move(transaction));
  }

  return input;
}

core::ports::workspace::WorkspaceSnapshot
StatementImportRunner::mergeWorkspaceState(
    core::ports::workspace::WorkspaceSnapshot primary,
    const core::ports::workspace::WorkspaceSnapshot &secondary) const {
  return mergeSnapshots(std::move(primary), secondary);
}

} // namespace core::application::importing
