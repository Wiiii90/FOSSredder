/**
 * @file ui/src/adapters/ImportAdapter.cpp
 * @brief Implements the import adapter used by UI import workflows.
 */

#include "ui/adapters/ImportAdapter.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <QVariant>

#include "ui/presentation/PayloadKeys.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"

namespace ui::adapters {

namespace core_importing = core::ports::importing::draft;

namespace {

QStringList toQStringList(const std::vector<std::string> &values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto &value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

QString proofImageDataUrl(const std::vector<std::uint8_t> &bytes) {
  if (bytes.empty()) {
    return {};
  }
  const QByteArray data(reinterpret_cast<const char *>(bytes.data()),
                        static_cast<int>(bytes.size()));
  return QString::fromLatin1(data.toBase64());
}

QVariantMap
toVariantMap(const core_importing::DraftSuggestionCandidate &suggestion) {
  QVariantMap map;
  map.insert(QStringLiteral("entityId"),
             QString::fromStdString(suggestion.entityId));
  map.insert(QStringLiteral("label"), QString::fromStdString(suggestion.label));
  map.insert(QStringLiteral("confidence"), suggestion.confidence);
  return map;
}

} // namespace

ImportAdapter::ImportAdapter(
    std::shared_ptr<core::ports::importing::IImportRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::importing::StatementImportHandle
ImportAdapter::startStatementImport(
    const core::ports::importing::ImportRequest &request,
    core::ports::importing::StatementImportEventCallback callback) {
  observability::traceAdapter(
      "ImportAdapter::startStatementImport", "Import runner invoked",
      {{observability::context::kFile, request.sourcePath}});
  return runner_ ? runner_->startStatementImport(request, std::move(callback))
                 : core::ports::importing::StatementImportHandle{};
}

void ImportAdapter::unsubscribe(
    const core::ports::importing::StatementImportHandle &handle) {
  if (runner_)
    runner_->unsubscribe(handle);
}

void ImportAdapter::cancel(
    const core::ports::importing::StatementImportHandle &handle) {
  if (runner_)
    runner_->cancel(handle);
}

void ImportAdapter::pause(
    const core::ports::importing::StatementImportHandle &handle) {
  if (runner_)
    runner_->pause(handle);
}

void ImportAdapter::resume(
    const core::ports::importing::StatementImportHandle &handle) {
  if (runner_)
    runner_->resume(handle);
}

core::ports::importing::ImportResult ImportAdapter::importResult(
    const core::ports::importing::StatementImportHandle &handle) {
  return runner_ ? runner_->importResult(handle)
                 : core::ports::importing::ImportResult{};
}

core::ports::importing::draft::DraftDerivedState
ImportAdapter::buildDraftDerivedState(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::DraftLinkSelection &selection) const {
  return runner_ ? runner_->buildDraftDerivedState(state, selection)
                 : core::ports::importing::draft::DraftDerivedState{};
}

core::ports::workspace::WorkspaceSnapshot ImportAdapter::mergeWorkspaceState(
    core::ports::workspace::WorkspaceSnapshot primary,
    const core::ports::workspace::WorkspaceSnapshot &secondary) const {
  return runner_ ? runner_->mergeWorkspaceState(std::move(primary), secondary)
                 : std::move(primary);
}

core_importing::DraftLinkSelection
ImportAdapter::toCoreSelection(const core_importing::TransactionDraft &draft) {
  core_importing::DraftLinkSelection out;
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

bool ImportAdapter::updateTransactionDraft(
    core_importing::TransactionDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core_importing::TransactionDraftEdit &edit) const {
  return runner_ && runner_->updateTransactionDraft(draft, state, edit);
}

core_importing::StatementDraftEditResult ImportAdapter::updateStatementDraft(
    core_importing::StatementDraft &draft,
    const core_importing::StatementDraftEdit &edit) const {
  return runner_ ? runner_->updateStatementDraft(draft, edit)
                 : core_importing::StatementDraftEditResult{};
}

core_importing::StatementDraft ImportAdapter::buildStatementDraft(
    const std::string &sourceFile,
    const core::ports::workspace::StatementSnapshot &statement,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::vector<core_importing::TransactionDraft> &transactions,
    const std::string &draftId) const {
  return runner_ ? runner_->buildStatementDraft(sourceFile, statement, state,
                                                transactions, draftId)
                 : core_importing::StatementDraft{};
}

core_importing::StatementDraft ImportAdapter::restoreStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot &draft) const {
  return runner_ ? runner_->restoreStatementDraft(draft)
                 : core_importing::StatementDraft{};
}

core::ports::workspace::StatementDraftSnapshot
ImportAdapter::buildStatementDraftSnapshot(
    const core_importing::StatementDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state) const {
  return runner_ ? runner_->buildStatementDraftSnapshot(draft, state)
                 : core::ports::workspace::StatementDraftSnapshot{};
}

QVariantMap ImportAdapter::toViewState(
    const core_importing::DraftDerivedState &derived) {
  QVariantMap map;
  map.insert(payload::keys::draftView::kProofSource,
             QString::fromStdString(derived.proofSource));
  map.insert(payload::keys::draftView::kActorSeedText,
             QString::fromStdString(derived.actorSeedText));
  map.insert(payload::keys::draftView::kActorDisplayText,
             QString::fromStdString(derived.actorDisplayText));
  map.insert(payload::keys::draftView::kContractSeedText,
             QString::fromStdString(derived.contractSeedText));
  map.insert(payload::keys::draftView::kContractDisplayText,
             QString::fromStdString(derived.contractDisplayText));
  map.insert(payload::keys::draftView::kActorSuggestionSummary,
             QString::fromStdString(derived.actorSuggestionSummary));
  map.insert(payload::keys::draftView::kPropertySuggestionSummary,
             QString::fromStdString(derived.propertySuggestionSummary));
  map.insert(payload::keys::draftView::kContractSuggestionSummary,
             QString::fromStdString(derived.contractSuggestionSummary));
  map.insert(payload::keys::draftView::kAllocatableSuggestionSummary,
             QString::fromStdString(derived.allocatableSuggestionSummary));
  map.insert(payload::keys::draftView::kActorSuggestionConfidence,
             derived.actorSuggestionConfidence);
  map.insert(payload::keys::draftView::kPropertySuggestionConfidence,
             derived.propertySuggestionConfidence);
  map.insert(payload::keys::draftView::kContractSuggestionConfidence,
             derived.contractSuggestionConfidence);
  map.insert(payload::keys::draftView::kAllocatableSuggestionConfidence,
             derived.allocatableSuggestionConfidence);
  map.insert(payload::keys::draftView::kEffectiveAllocatable,
             derived.effectiveAllocatable);
  map.insert(payload::keys::draftView::kActorCurrentIndex,
             derived.actorCurrentIndex);
  map.insert(payload::keys::draftView::kContractCurrentIndex,
             derived.contractCurrentIndex);
  map.insert(payload::keys::draftView::kActorTopSuggestion,
             derived.hasActorTopSuggestion
                 ? toVariantMap(derived.actorTopSuggestion)
                 : QVariantMap{});
  map.insert(payload::keys::draftView::kPropertyTopSuggestion,
             derived.hasPropertyTopSuggestion
                 ? toVariantMap(derived.propertyTopSuggestion)
                 : QVariantMap{});
  map.insert(payload::keys::draftView::kContractTopSuggestion,
             derived.hasContractTopSuggestion
                 ? toVariantMap(derived.contractTopSuggestion)
                 : QVariantMap{});
  return map;
}

TransactionDraftView ImportAdapter::toTransactionDraftView(
    const core_importing::TransactionDraft &draft,
    const QVariantMap &viewState) {
  TransactionDraftView view;
  view.id = QString::fromStdString(draft.id);
  view.name = QString::fromStdString(draft.name);
  view.bookingDate = QString::fromStdString(draft.bookingDate);
  view.valuta = QString::fromStdString(draft.valuta);
  view.amountText = QVariant(draft.amount).toString();
  view.metadata = QString::fromStdString(draft.metadata);
  const QString proofData = proofImageDataUrl(draft.proofImageData);
  view.proofSource = proofData.isEmpty()
                         ? QString{}
                         : QStringLiteral("data:image/jpeg;base64,") +
                               proofData;
  view.actorId = QString::fromStdString(draft.actorId);
  view.contractId = QString::fromStdString(draft.contractId);
  view.propertyIds = toQStringList(draft.propertyIds);
  view.status = draft.status;
  view.effectiveAllocatable =
      draft.allocatableSelected
          ? draft.allocatable
          : viewState
                .value(payload::keys::draftView::kEffectiveAllocatable,
                       draft.allocatable)
                .toBool();
  view.actorSuggestionConfidence =
      viewState.value(payload::keys::draftView::kActorSuggestionConfidence)
          .toDouble();
  view.actorSuggestionSummary =
      viewState.value(payload::keys::draftView::kActorSuggestionSummary)
          .toString();
  view.propertySuggestionConfidence =
      viewState.value(payload::keys::draftView::kPropertySuggestionConfidence)
          .toDouble();
  view.propertySuggestionSummary =
      viewState.value(payload::keys::draftView::kPropertySuggestionSummary)
          .toString();
  view.contractSuggestionConfidence =
      viewState.value(payload::keys::draftView::kContractSuggestionConfidence)
          .toDouble();
  view.contractSuggestionSummary =
      viewState.value(payload::keys::draftView::kContractSuggestionSummary)
          .toString();
  view.allocatableSuggestionConfidence =
      viewState
          .value(payload::keys::draftView::kAllocatableSuggestionConfidence)
          .toDouble();
  view.allocatableSuggestionSummary =
      viewState.value(payload::keys::draftView::kAllocatableSuggestionSummary)
          .toString();
  return view;
}

} // namespace ui::adapters
