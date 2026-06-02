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

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui::adapters {

namespace core_importing = core::ports::importing::draft;

namespace {

QVariantMap createdDraftCatalogRow(const QString &id, const QString &display,
                                   const QString &type,
                                   const QStringList &actorIds = {},
                                   const QStringList &propertyIds = {},
                                   const QString &allocatableMode = {}) {
  QVariantMap row;
  row.insert(payload::keys::common::kId, id);
  row.insert(payload::keys::common::kName, display);
  row.insert(payload::keys::common::kDisplay, display);
  row.insert(payload::keys::common::kType, type);
  row.insert(payload::keys::actor::kAliases, QStringList{});
  row.insert(payload::keys::contract::kActorIds, actorIds);
  row.insert(payload::keys::contract::kPropertyIds, propertyIds);
  row.insert(QStringLiteral("synthetic"), false);
  row.insert(QStringLiteral("confidence"), 1.0);
  row.insert(QStringLiteral("sourceText"), display);
  if (!allocatableMode.isEmpty()) {
    row.insert(payload::keys::contract::kAllocatableMode, allocatableMode);
  }
  return row;
}

QStringList toQStringList(const std::vector<std::string> &values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto &value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

std::vector<std::string> toStdStringVector(const QStringList &values) {
  std::vector<std::string> out;
  out.reserve(static_cast<std::size_t>(values.size()));
  for (const auto &value : values) {
    out.push_back(strings::toStdString(value));
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
    ui::WorkspaceFacade *uiWorkspace,
    std::shared_ptr<core::ports::importing::IImportRunner> runner)
    : uiWorkspace_(uiWorkspace), runner_(std::move(runner)) {}

core::ports::importing::StatementImportHandle
ImportAdapter::startStatementImport(
    const core::ports::importing::StatementImportStartRequest &request,
    core::ports::importing::StatementImportEventCallback callback) {
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

core::ports::importing::draft::DraftImportSuggestions
ImportAdapter::buildImportSuggestions(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::TransactionDraft &transaction) const {
  return runner_ ? runner_->buildImportSuggestions(state, transaction)
                 : core::ports::importing::draft::DraftImportSuggestions{};
}

core::ports::importing::draft::DraftTextSignals
ImportAdapter::buildDraftTextSignals(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::TransactionDraft &transaction) const {
  return runner_ ? runner_->buildDraftTextSignals(state, transaction)
                 : core::ports::importing::draft::DraftTextSignals{};
}

core::ports::importing::draft::DraftDerivedState
ImportAdapter::buildDraftDerivedState(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const core::ports::importing::draft::DraftLinkSelection &selection) const {
  return runner_ ? runner_->buildDraftDerivedState(state, selection)
                 : core::ports::importing::draft::DraftDerivedState{};
}

std::string ImportAdapter::resolveActorId(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::string &text) const {
  return runner_ ? runner_->resolveActorId(state, text) : std::string{};
}

std::string ImportAdapter::resolveContractId(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::string &text) const {
  return runner_ ? runner_->resolveContractId(state, text) : std::string{};
}

bool ImportAdapter::contractIsFullyAllocatable(
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::string &contractId) const {
  return runner_ && runner_->contractIsFullyAllocatable(state, contractId);
}

core::ports::workspace::WorkspaceSnapshot ImportAdapter::mergeWorkspaceState(
    core::ports::workspace::WorkspaceSnapshot primary,
    const core::ports::workspace::WorkspaceSnapshot &secondary) const {
  return runner_ ? runner_->mergeWorkspaceState(std::move(primary), secondary)
                 : std::move(primary);
}

std::vector<std::string>
ImportAdapter::referenceAliasesFromMetadata(const std::string &metadata) const {
  return runner_ ? runner_->referenceAliasesFromMetadata(metadata)
                 : std::vector<std::string>{};
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

bool ImportAdapter::applyDerivedSelections(
    core_importing::TransactionDraft &draft,
    const core_importing::DraftDerivedState &derived,
    core_importing::DraftAutoSelectionMode mode) const {
  return runner_ && runner_->applyDerivedSelections(draft, derived, mode);
}

bool ImportAdapter::applyActorSelection(core_importing::TransactionDraft &draft,
                                        const std::string &actorId) const {
  return runner_ && runner_->applyActorSelection(draft, actorId);
}

bool ImportAdapter::clearActorSelection(
    core_importing::TransactionDraft &draft) const {
  return runner_ && runner_->clearActorSelection(draft);
}

bool ImportAdapter::applyPropertySelection(
    core_importing::TransactionDraft &draft,
    const std::string &propertyId) const {
  return runner_ && runner_->applyPropertySelection(draft, propertyId);
}

bool ImportAdapter::setPropertySelected(core_importing::TransactionDraft &draft,
                                        const std::string &propertyId,
                                        bool selected) const {
  return runner_ && runner_->setPropertySelected(draft, propertyId, selected);
}

bool ImportAdapter::applyContractSelection(
    core_importing::TransactionDraft &draft,
    const core_importing::DraftChoiceRow &contract) const {
  return runner_ && runner_->applyContractSelection(draft, contract);
}

bool ImportAdapter::applyContractSelection(
    core_importing::TransactionDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::string &contractId) const {
  return runner_ && runner_->applyContractSelection(draft, state, contractId);
}

bool ImportAdapter::clearContractSelection(
    core_importing::TransactionDraft &draft) const {
  return runner_ && runner_->clearContractSelection(draft);
}

bool ImportAdapter::applyTransactionPatch(
    core_importing::TransactionDraft &draft,
    const core_importing::TransactionDraftPatch &patch) const {
  return runner_ && runner_->applyTransactionPatch(draft, patch);
}

int ImportAdapter::insertTransactionAfter(core_importing::StatementDraft &draft,
                                          int currentIndex) const {
  return runner_ ? runner_->insertTransactionAfter(draft, currentIndex) : -1;
}

int ImportAdapter::removeTransactionAt(core_importing::StatementDraft &draft,
                                       int index) const {
  return runner_ ? runner_->removeTransactionAt(draft, index) : -1;
}

bool ImportAdapter::renameStatementDraft(core_importing::StatementDraft &draft,
                                         const std::string &name) const {
  return runner_ && runner_->renameStatementDraft(draft, name);
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

core::ports::workspace::StatementDraftSnapshot
ImportAdapter::buildImportedStatementDraftSnapshot(
    const std::string &sourceFile, const std::string &draftId,
    const core::ports::workspace::StatementSnapshot &statement,
    const std::vector<core_importing::TransactionDraft> &transactions) const {
  return runner_ ? runner_->buildImportedStatementDraftSnapshot(
                       sourceFile, draftId, statement, transactions)
                 : core::ports::workspace::StatementDraftSnapshot{};
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

QVariantMap ImportAdapter::actorIdentityByName(const QString &name) const {
  return uiWorkspace_ ? uiWorkspace_->actorIdentityByName(name) : QVariantMap{};
}

QVariantMap ImportAdapter::propertyIdentityByName(const QString &name) const {
  return uiWorkspace_ ? uiWorkspace_->propertyIdentityByName(name)
                      : QVariantMap{};
}

QVariantMap ImportAdapter::contractIdentityBySignature(
    const QString &name, const QString &type, const QStringList &actorIds,
    const QStringList &propertyIds) const {
  return uiWorkspace_ ? uiWorkspace_->contractIdentityBySignature(
                            name, type, actorIds, propertyIds)
                      : QVariantMap{};
}

QString ImportAdapter::nextContractName() const {
  return uiWorkspace_ ? uiWorkspace_->nextContractName() : QString{};
}

QString ImportAdapter::saveActor(const QString &name) {
  return uiWorkspace_ ? uiWorkspace_->saveActor({}, name) : QString{};
}

QString ImportAdapter::saveProperty(const QString &name) {
  return uiWorkspace_ ? uiWorkspace_->saveProperty({}, name) : QString{};
}

QString ImportAdapter::saveContract(const QString &name, const QString &type,
                                    const QStringList &actorIds,
                                    const QStringList &propertyIds,
                                    const QString &allocatableMode) {
  return uiWorkspace_
             ? uiWorkspace_->saveContract({}, name, type, actorIds, propertyIds,
                                          {}, allocatableMode)
             : QString{};
}

QVariantMap ImportAdapter::createActorForDraft(
    core::ports::importing::draft::TransactionDraft &draft,
    const QString &actorName) {
  const QString trimmedName = actorName.trimmed();
  if (trimmedName.isEmpty()) {
    return {};
  }

  QString actorId;
  QString displayName = trimmedName;
  const QVariantMap existing = actorIdentityByName(trimmedName);
  if (!existing.isEmpty()) {
    actorId = existing.value(payload::keys::common::kId).toString();
    displayName =
        existing.value(payload::keys::common::kDisplay, displayName)
            .toString();
  }

  if (actorId.isEmpty()) {
    actorId = saveActor(trimmedName);
    if (actorId.isEmpty()) {
      return {};
    }
  }

  if (!runner_) {
    return {};
  }
  applyActorSelection(draft, strings::toStdString(actorId));
  return createdDraftCatalogRow(actorId, displayName, QStringLiteral("actor"));
}

QVariantMap ImportAdapter::createPropertyForDraft(
    core::ports::importing::draft::TransactionDraft &draft,
    const QString &propertyName) {
  const QString trimmedName = propertyName.trimmed();
  if (trimmedName.isEmpty()) {
    return {};
  }

  QString propertyId;
  QString displayName = trimmedName;
  const QVariantMap existing = propertyIdentityByName(trimmedName);
  if (!existing.isEmpty()) {
    propertyId = existing.value(payload::keys::common::kId).toString();
    displayName =
        existing.value(payload::keys::common::kDisplay, displayName)
            .toString();
  }

  if (propertyId.isEmpty()) {
    propertyId = saveProperty(trimmedName);
    if (propertyId.isEmpty()) {
      return {};
    }
  }

  if (!runner_) {
    return {};
  }
  applyPropertySelection(draft, strings::toStdString(propertyId));
  return createdDraftCatalogRow(propertyId, displayName,
                                QStringLiteral("property"));
}

QVariantMap ImportAdapter::createOrSelectContractForDraft(
    core::ports::importing::draft::TransactionDraft &draft,
    const QString &contractName, const QString &contractType,
    const QString &allocatableMode) {
  const QString trimmedType = contractType.trimmed();
  if (trimmedType.isEmpty()) {
    return {};
  }

  const QString effectiveName =
      contractName.trimmed().isEmpty() ? nextContractName()
                                       : contractName.trimmed();
  QStringList actorIds;
  const QString currentActorId = QString::fromStdString(draft.actorId);
  if (!currentActorId.trimmed().isEmpty()) {
    actorIds.push_back(currentActorId.trimmed());
  }
  const QStringList propertyIds = toQStringList(draft.propertyIds);

  QString contractId;
  QString displayName = effectiveName;
  QString contractAllocatableMode = allocatableMode.trimmed().toLower();
  if (contractAllocatableMode.isEmpty()) {
    contractAllocatableMode = QStringLiteral("mixed");
  }

  const QVariantMap existing = contractIdentityBySignature(
      effectiveName, trimmedType, actorIds, propertyIds);
  if (!existing.isEmpty()) {
    contractId = existing.value(payload::keys::common::kId).toString();
    displayName =
        existing.value(payload::keys::common::kDisplay, displayName)
            .toString();
    contractAllocatableMode =
        existing
            .value(payload::keys::contract::kAllocatableMode,
                   contractAllocatableMode)
            .toString();
  }

  if (contractId.isEmpty()) {
    contractId = saveContract(effectiveName, trimmedType, actorIds, propertyIds,
                              contractAllocatableMode);
    if (contractId.isEmpty()) {
      return {};
    }
  }

  core_importing::DraftChoiceRow contractRow;
  contractRow.id = strings::toStdString(contractId);
  contractRow.name = strings::toStdString(displayName);
  contractRow.display = strings::toStdString(displayName);
  contractRow.type = strings::toStdString(trimmedType);
  contractRow.allocatableMode = strings::toStdString(contractAllocatableMode);
  contractRow.actorIds = toStdStringVector(actorIds);
  contractRow.propertyIds = toStdStringVector(propertyIds);
  if (!runner_) {
    return {};
  }
  applyContractSelection(draft, contractRow);
  return createdDraftCatalogRow(contractId, displayName, trimmedType, actorIds,
                                propertyIds, contractAllocatableMode);
}

} // namespace ui::adapters
