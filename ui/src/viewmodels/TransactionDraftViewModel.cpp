/**
 * @file ui/src/viewmodels/TransactionDraftViewModel.cpp
 * @brief Binds TransactionDraftViewModel to import workflow, workspace, and draft.
 */

#include "ui/viewmodels/TransactionDraftViewModel.h"

#include "core/application/import/transaction/AmountParser.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr int kSuggestionDanger = 0;
constexpr int kSuggestionWarning = 1;
constexpr int kSuggestionSuccess = 2;

adapters::ImportAdapter *importAdapter(ImportWorkflow *workflow) {
  return workflow && workflow->importAdapter()
             ? workflow->importAdapter().get()
             : nullptr;
}

core::ports::importing::draft::TransactionDraft *
currentTransaction(ImportWorkflow *workflow) {
  return workflow ? workflow->currentTransactionDraft() : nullptr;
}

bool applyTransactionPatch(
    ImportWorkflow *workflow,
    const core::ports::importing::draft::TransactionDraftPatch &patch) {
  auto *transaction = currentTransaction(workflow);
  auto *adapter = importAdapter(workflow);
  if (!transaction || !adapter ||
      !adapter->applyTransactionPatch(*transaction, patch)) {
    return false;
  }
  workflow->notifyDraftChanged();
  return true;
}

} // namespace

TransactionDraftViewModel::TransactionDraftViewModel(QObject *parent)
    : QObject(parent) {}

void TransactionDraftViewModel::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (importWorkflow_) {
    connect(importWorkflow_, &ImportWorkflow::stateChanged, this,
            &TransactionDraftViewModel::handleWorkflowStateChanged);
  }
  resetFormFieldsFromCurrentTransaction();
  emit changed();
}

void TransactionDraftViewModel::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  emit changed();
}

void TransactionDraftViewModel::bindWorkspace(WorkspaceFacade *workspace) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = workspace;
  if (workspace_) {
    connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
            &TransactionDraftViewModel::notifyCurrentTransactionViewChanged);
  }
}

void TransactionDraftViewModel::handleWorkflowStateChanged() {
  const QString previousId = lastTransactionId_;
  loadCurrentTransactionView();
  const QString id = currentView_.id;
  if (!id.isEmpty() && id == previousId) {
    emit changed();
    return;
  }
  lastTransactionId_ = id;
  nameText_ = currentView_.name;
  bookingDateText_ = currentView_.bookingDate;
  valutaText_ = currentView_.valuta;
  amountText_ = currentView_.amountText;
  actorName_.clear();
  contractName_.clear();
  contractType_.clear();
  contractAllocatableMode_ = payload::contract_allocatable_mode::kMixed;
  propertyName_.clear();
  emit changed();
}

void TransactionDraftViewModel::resetFormFieldsFromCurrentTransaction() {
  loadCurrentTransactionView();
  lastTransactionId_ = currentView_.id;
  nameText_ = currentView_.name;
  bookingDateText_ = currentView_.bookingDate;
  valutaText_ = currentView_.valuta;
  amountText_ = currentView_.amountText;
  actorName_.clear();
  contractName_.clear();
  contractType_.clear();
  contractAllocatableMode_ = payload::contract_allocatable_mode::kMixed;
  propertyName_.clear();
}

void TransactionDraftViewModel::setNameText(const QString &value) {
  if (nameText_ == value) {
    return;
  }
  nameText_ = value;
  emit changed();
}

void TransactionDraftViewModel::setBookingDateText(const QString &value) {
  if (bookingDateText_ == value) {
    return;
  }
  bookingDateText_ = value;
  emit changed();
}

void TransactionDraftViewModel::setValutaText(const QString &value) {
  if (valutaText_ == value) {
    return;
  }
  valutaText_ = value;
  emit changed();
}

void TransactionDraftViewModel::setAmountText(const QString &value) {
  if (amountText_ == value) {
    return;
  }
  amountText_ = value;
  emit changed();
}

QVariantList TransactionDraftViewModel::statusOptions() const {
  return payload::transaction_status::options(
      tr("Neutral"), tr("Unverified"), tr("Verified"), tr("Completed"));
}

int TransactionDraftViewModel::statusIndex() const {
  return payload::transaction_status::indexForStatus(statusOptions(),
                                                     currentView_.status);
}

QString TransactionDraftViewModel::metadataText() const {
  return currentView_.metadata;
}

QString TransactionDraftViewModel::proofSource() const {
  return currentView_.proofSource;
}

bool TransactionDraftViewModel::effectiveAllocatable() const {
  return currentView_.effectiveAllocatable;
}

void TransactionDraftViewModel::commitNameText() {
  if (!importWorkflow_ || nameText_ == currentView_.name) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasName = true;
  patch.name = strings::toStdString(nameText_);
  applyTransactionPatch(importWorkflow_, patch);
}

void TransactionDraftViewModel::commitBookingDateText() {
  if (!importWorkflow_ || bookingDateText_ == currentView_.bookingDate) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasBookingDate = true;
  patch.bookingDate = strings::toStdString(bookingDateText_);
  applyTransactionPatch(importWorkflow_, patch);
}

void TransactionDraftViewModel::commitValutaText() {
  if (!importWorkflow_ || valutaText_ == currentView_.valuta) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasValuta = true;
  patch.valuta = strings::toStdString(valutaText_);
  applyTransactionPatch(importWorkflow_, patch);
}

void TransactionDraftViewModel::commitAmountText() {
  if (!importWorkflow_) {
    return;
  }
  const auto parsed = core::application::importing::transaction::parseAmountString(
      strings::toStdString(amountText_));
  if (!parsed) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasAmount = true;
  patch.amount = *parsed;
  applyTransactionPatch(importWorkflow_, patch);
}

void TransactionDraftViewModel::selectStatusAtIndex(int index) {
  if (!importWorkflow_) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasStatus = true;
  patch.status =
      payload::transaction_status::statusAt(statusOptions(), index);
  applyTransactionPatch(importWorkflow_, patch);
}

void TransactionDraftViewModel::toggleAllocatable() {
  if (!importWorkflow_) {
    return;
  }
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasAllocatable = true;
  patch.allocatable = !effectiveAllocatable();
  patch.allocatableSelected = true;
  applyTransactionPatch(importWorkflow_, patch);
}

QVariantList TransactionDraftViewModel::actorOptions() const {
  return workspace_ ? workspace_->actorDropdownRows() : QVariantList{};
}

int TransactionDraftViewModel::selectedActorOptionIndex() const {
  return payload::mapper::indexById(actorOptions(), currentTransactionActorId());
}

void TransactionDraftViewModel::setActorName(const QString &value) {
  if (actorName_ == value) {
    return;
  }
  actorName_ = value;
  emit changed();
}

bool TransactionDraftViewModel::canAddActor() const {
  return workspace_ && !strings::normalizedText(actorName_).isEmpty() &&
         workspace_->actorIdentityByName(actorName_).isEmpty();
}

void TransactionDraftViewModel::selectActorAtIndex(int index) {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!transaction || !adapter) {
    return;
  }
  const QString id =
      payload::mapper::rowId(payload::mapper::mapAt(actorOptions(), index));
  const bool changed =
      id.isEmpty() ? adapter->clearActorSelection(*transaction)
                   : adapter->applyActorSelection(*transaction,
                                                  strings::toStdString(id));
  if (changed) {
    importWorkflow_->notifyDraftChanged();
  }
}

void TransactionDraftViewModel::addActor() {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!canAddActor() || !transaction || !adapter) {
    return;
  }
  if (!adapter->createActorForDraft(*transaction, actorName_).isEmpty()) {
    importWorkflow_->notifyDraftChanged();
  }
}

QVariantList TransactionDraftViewModel::propertyOptions() const {
  return workspace_ ? workspace_->propertyDropdownRows() : QVariantList{};
}

void TransactionDraftViewModel::setPropertyName(const QString &value) {
  if (propertyName_ == value) {
    return;
  }
  propertyName_ = value;
  emit changed();
}

bool TransactionDraftViewModel::canAddProperty() const {
  return workspace_ && !strings::normalizedText(propertyName_).isEmpty() &&
         workspace_->propertyIdentityByName(propertyName_).isEmpty();
}

bool TransactionDraftViewModel::isPropertySelected(
    const QString &propertyId) const {
  return currentTransactionPropertyIds().contains(propertyId);
}

void TransactionDraftViewModel::setPropertySelected(const QString &propertyId,
                                                    bool selected) {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!transaction || !adapter) {
    return;
  }
  if (adapter->setPropertySelected(*transaction, strings::toStdString(propertyId),
                                 selected)) {
    importWorkflow_->notifyDraftChanged();
  }
}

void TransactionDraftViewModel::addProperty() {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!canAddProperty() || !transaction || !adapter) {
    return;
  }
  if (!adapter->createPropertyForDraft(*transaction, propertyName_).isEmpty()) {
    propertyName_.clear();
    importWorkflow_->notifyDraftChanged();
    emit changed();
  }
}

QVariantList TransactionDraftViewModel::contractOptions() const {
  return workspace_ ? workspace_->contractDropdownRows() : QVariantList{};
}

int TransactionDraftViewModel::selectedContractOptionIndex() const {
  return payload::mapper::indexById(contractOptions(), currentView_.contractId);
}

QString TransactionDraftViewModel::selectedContractType() const {
  return payload::mapper::rowType(
      payload::mapper::mapAt(contractOptions(), selectedContractOptionIndex()));
}

void TransactionDraftViewModel::selectContractAtIndex(int index) {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!transaction || !adapter || !importWorkflow_) {
    return;
  }
  const QString id =
      payload::mapper::rowId(payload::mapper::mapAt(contractOptions(), index));
  const bool changed =
      id.isEmpty()
          ? adapter->clearContractSelection(*transaction)
          : adapter->applyContractSelection(
                *transaction, importWorkflow_->catalogSnapshotForDraft(),
                strings::toStdString(id));
  if (changed) {
    importWorkflow_->notifyDraftChanged();
  }
}

void TransactionDraftViewModel::setContractName(const QString &value) {
  if (contractName_ == value) {
    return;
  }
  contractName_ = value;
  emit changed();
}

void TransactionDraftViewModel::setContractType(const QString &value) {
  if (contractType_ == value) {
    return;
  }
  contractType_ = value;
  emit changed();
}

QString TransactionDraftViewModel::contractNamePlaceholder() const {
  if (workspace_) {
    return workspace_->nextContractName();
  }
  return tr("Contract 1");
}

QVariantList TransactionDraftViewModel::contractAllocatableModes() const {
  return payload::contract_allocatable_mode::options(
      tr("Mixed"), tr("All allocatable"), tr("Never allocatable"));
}

int TransactionDraftViewModel::contractAllocatableModeIndex() const {
  return payload::contract_allocatable_mode::indexForMode(
      contractAllocatableModes(), contractAllocatableMode_);
}

void TransactionDraftViewModel::setContractAllocatableModeIndex(int index) {
  const QString mode = payload::contract_allocatable_mode::modeAt(
      contractAllocatableModes(), index);
  if (contractAllocatableMode_ == mode) {
    return;
  }
  contractAllocatableMode_ = mode;
  emit changed();
}

bool TransactionDraftViewModel::canAddContract() const {
  if (!workspace_ || strings::normalizedText(contractType_).isEmpty()) {
    return false;
  }
  const QString actorId = currentTransactionActorId();
  QStringList actorIds;
  if (!actorId.trimmed().isEmpty()) {
    actorIds.push_back(actorId.trimmed());
  }
  const QString effectiveName = contractName_.trimmed().isEmpty()
                                    ? workspace_->nextContractName()
                                    : contractName_.trimmed();
  return workspace_
      ->contractIdentityBySignature(effectiveName, contractType_, actorIds,
                                    currentTransactionPropertyIds())
      .isEmpty();
}

void TransactionDraftViewModel::addContract() {
  auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!canAddContract() || !transaction || !adapter) {
    return;
  }
  const QVariantMap row = adapter->createOrSelectContractForDraft(
      *transaction, contractName_, contractType_, contractAllocatableMode_);
  if (!payload::mapper::rowId(row).isEmpty()) {
    contractName_ = payload::mapper::rowDisplayName(row);
    importWorkflow_->notifyDraftChanged();
    emit changed();
  }
}

void TransactionDraftViewModel::loadCurrentTransactionView() {
  currentView_ = {};
  const auto *transaction = currentTransaction(importWorkflow_);
  auto *adapter = importAdapter(importWorkflow_);
  if (!transaction || !adapter || !importWorkflow_) {
    return;
  }

  const char *origin = observability::origins::workflow::import::kFinalize;
  try {
    const QVariantMap viewState = adapters::ImportAdapter::toViewState(
        adapter->buildDraftDerivedState(
            importWorkflow_->catalogSnapshotForDraft(),
            adapters::ImportAdapter::toCoreSelection(*transaction)));
    currentView_ =
        adapters::ImportAdapter::toTransactionDraftView(*transaction, viewState);
  } catch (...) {
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError, origin,
                                  std::current_exception());
  }
}

void TransactionDraftViewModel::notifyCurrentTransactionViewChanged() {
  loadCurrentTransactionView();
  emit changed();
}

QString TransactionDraftViewModel::suggestionText(const QString &value) const {
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

QString TransactionDraftViewModel::currentTransactionId() const {
  return currentView_.id;
}

QStringList TransactionDraftViewModel::currentTransactionPropertyIds() const {
  return currentView_.propertyIds;
}

QString TransactionDraftViewModel::currentTransactionActorId() const {
  return currentView_.actorId;
}

int TransactionDraftViewModel::suggestionTone(double confidence) const {
  if (confidence >= 0.9) {
    return kSuggestionSuccess;
  }
  if (confidence >= 0.4) {
    return kSuggestionWarning;
  }
  return kSuggestionDanger;
}

double TransactionDraftViewModel::actorSuggestionConfidence() const {
  return currentView_.actorSuggestionConfidence;
}

QString TransactionDraftViewModel::actorSuggestionSummary() const {
  return suggestionText(currentView_.actorSuggestionSummary);
}

double TransactionDraftViewModel::propertySuggestionConfidence() const {
  return currentView_.propertySuggestionConfidence;
}

QString TransactionDraftViewModel::propertySuggestionSummary() const {
  return suggestionText(currentView_.propertySuggestionSummary);
}

double TransactionDraftViewModel::contractSuggestionConfidence() const {
  return currentView_.contractSuggestionConfidence;
}

QString TransactionDraftViewModel::contractSuggestionSummary() const {
  return suggestionText(currentView_.contractSuggestionSummary);
}

double TransactionDraftViewModel::allocatableSuggestionConfidence() const {
  return currentView_.allocatableSuggestionConfidence;
}

QString TransactionDraftViewModel::allocatableSuggestionText() const {
  return suggestionText(currentView_.allocatableSuggestionSummary);
}

} // namespace ui
