/**
 * @file ui/src/viewmodels/ImportViewModel.cpp
 * @brief Implements import overview and draft review UI state.
 */

#include "ui/viewmodels/ImportViewModel.h"

#include <QFileInfo>

#include "ui/i18n/Text.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadMapper.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/Navigation.h"
#include "ui/shell/Settings.h"
#include "ui/shell/Status.h"
#include "ui/utils/StringConversions.h"
#include "ui/workflows/ImportWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

constexpr int kSuggestionDanger = 0;
constexpr int kSuggestionWarning = 1;
constexpr int kSuggestionSuccess = 2;

} // namespace

ImportViewModel::ImportViewModel(QObject* parent)
    : QObject(parent),
      contractAllocatableMode_(payload::contract_allocatable_mode::kMixed) {}

void ImportViewModel::setImportWorkflow(ImportWorkflow* value) {
  if (importWorkflow_ == value) {
    return;
  }
  bindImportWorkflow(value);
  if (importWorkflow_) {
    importWorkflow_->setWorkspaceRoles(commands_, selectors_);
  }
  syncTransactionDraftFromWorkflow();
  updateManualPathFromWorkflow();
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::setSettings(Settings* value) {
  if (settings_ == value) {
    return;
  }
  bindSettings(value);
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::setActions(Actions* value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emit changed();
}

void ImportViewModel::setNavigation(Navigation* value) {
  if (navigation_ == value) {
    return;
  }
  navigation_ = value;
  emit changed();
}

void ImportViewModel::setStatus(Status* value) {
  if (status_ == value) {
    return;
  }
  status_ = value;
  emit changed();
}

void ImportViewModel::setWorkspaceRoles(WorkspaceStore* store,
                                        WorkspaceCommands* commands,
                                        WorkspaceSelection* selection,
                                        WorkspaceSelectors* selectors) {
  if (store_ == store && commands_ == commands && selection_ == selection &&
      selectors_ == selectors) {
    return;
  }
  bindWorkspaceRoles(store, commands, selection, selectors);
  if (importWorkflow_) {
    importWorkflow_->setWorkspaceRoles(commands_, selectors_);
  }
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::bindImportWorkflow(ImportWorkflow* value) {
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (!importWorkflow_) {
    return;
  }
  connect(importWorkflow_, &ImportWorkflow::stateChanged, this, [this]() {
    updateManualPathFromWorkflow();
    syncTransactionDraftFromWorkflow();
    emit changed();
  });
  connect(importWorkflow_, &ImportWorkflow::importCanceled, this, [this]() {
    setStatusText(tr("Import canceled"));
  });
  connect(importWorkflow_, &ImportWorkflow::importFinished, this, [this]() {
    setStatusText(tr("Import finished"));
  });
  connect(importWorkflow_, &ImportWorkflow::importFailed, this,
          [this](const QString& error) {
            setStatusText(error.isEmpty() ? tr("Import failed") : error);
          });
}

void ImportViewModel::bindSettings(Settings* value) {
  if (settings_) {
    disconnect(settings_, nullptr, this, nullptr);
  }
  settings_ = value;
  if (settings_) {
    connect(settings_, &Settings::importDefaultPathChanged, this, [this]() {
      applyDefaultImportSelection();
      emit changed();
    });
  }
}

void ImportViewModel::bindActions(Actions* value) {
  if (actions_) {
    disconnect(actions_, nullptr, this, nullptr);
  }
  actions_ = value;
  if (!actions_) {
    return;
  }
  connect(actions_, &Actions::importFileSelected, this,
          [this](const QString& path) {
            manualPathText_ = path;
            setSelectedImportFiles(QStringList{path});
          });
  connect(actions_, &Actions::importFilesSelected, this,
          [this](const QStringList& paths) {
            manualPathText_ = paths.isEmpty() ? QString{} : paths.front();
            setSelectedImportFiles(paths);
          });
  connect(actions_, &Actions::importFileDropped, this,
          [this](const QString& path) {
            queueImportFiles(QStringList{path});
          });
  connect(actions_, &Actions::importFilesDropped, this,
          [this](const QStringList& paths) {
            queueImportFiles(paths);
          });
}

void ImportViewModel::bindWorkspaceRoles(WorkspaceStore* store,
                                         WorkspaceCommands* commands,
                                         WorkspaceSelection* selection,
                                         WorkspaceSelectors* selectors) {
  if (store_) {
    disconnect(store_, nullptr, this, nullptr);
  }
  store_ = store;
  commands_ = commands;
  selection_ = selection;
  selectors_ = selectors;
  if (store_) {
    connect(store_, &WorkspaceStore::dataRevisionChanged, this, [this]() {
      applyDefaultImportSelection();
      reloadCurrentTransactionView();
      emit changed();
    });
  }
}

void ImportViewModel::setManualPathText(const QString& value) {
  if (manualPathText_ == value) {
    return;
  }
  manualPathText_ = value;
  selectedImportFiles_.clear();
  emit changed();
}

QStringList ImportViewModel::importFiles() const {
  QStringList files;
  if (!importWorkflow_) {
    return files;
  }
  if (!importWorkflow_->selectedFile().isEmpty()) {
    files.push_back(importWorkflow_->selectedFile());
  }
  files.append(importWorkflow_->queuedFiles());
  return files;
}

QString ImportViewModel::importFileSummary() const {
  const QStringList files = importFiles();
  if (files.isEmpty()) {
    return {};
  }

  QStringList names;
  names.reserve(files.size());
  for (const QString& file : files) {
    const QString name = QFileInfo(file).fileName();
    names.push_back(name.isEmpty() ? file : name);
  }
  return tr("Selected: %1").arg(names.join(QStringLiteral(", ")));
}

void ImportViewModel::initializeImportView() {
  applyDefaultImportSelection();
}

void ImportViewModel::applyDefaultImportSelection() {
  if (!importWorkflow_ || !settings_ || importWorkflow_->isRunning()) {
    return;
  }
  if (importWorkflow_->queuedCount() > 0) {
    return;
  }
  const QString currentFile = importWorkflow_->selectedFile();
  const QString defaultPath = settings_->importDefaultPath();
  const bool canReplaceSelection =
      currentFile.isEmpty() || currentFile == appliedDefaultImportPath_;
  if (!canReplaceSelection || currentFile == defaultPath) {
    return;
  }
  importWorkflow_->setSelectedFile(defaultPath);
  appliedDefaultImportPath_ = defaultPath;
  updateManualPathFromWorkflow();
  emit changed();
}

void ImportViewModel::browseImportPdf() {
  if (actions_) {
    actions_->browseImportPdf();
  }
}

void ImportViewModel::addSelectedImportFiles() {
  QStringList files = selectedImportFiles_;
  if (files.isEmpty() && !manualPathText_.trimmed().isEmpty()) {
    files.push_back(manualPathText_);
  }
  queueImportFiles(files);
  if (!files.isEmpty()) {
    manualPathText_.clear();
    selectedImportFiles_.clear();
    emit changed();
  }
}

void ImportViewModel::queueImportFiles(const QStringList& paths) {
  if (!importWorkflow_) {
    return;
  }
  const QStringList supported = supportedImportFiles(paths);
  if (supported.isEmpty()) {
    return;
  }
  observability::traceViewModel(
      "ImportViewModel::queueImportFiles", "Import files queued",
      {{observability::context::kQueuedCount, std::to_string(supported.size())},
       {observability::context::kFirstFile, supported.front().toStdString()}});
  importWorkflow_->addFiles(supported);
  selectedImportFiles_.clear();
  emit changed();
}

void ImportViewModel::updateManualPathFromWorkflow() {
  if (!importWorkflow_ || !selectedImportFiles_.isEmpty()) {
    return;
  }
  const QString selectedFile = importWorkflow_->selectedFile();
  if (manualPathText_ != selectedFile) {
    manualPathText_ = selectedFile;
  }
}

void ImportViewModel::setSelectedImportFiles(const QStringList& paths) {
  selectedImportFiles_ = paths;
  emit changed();
}

QStringList
ImportViewModel::supportedImportFiles(const QStringList& paths) const {
  QStringList supported;
  for (const QString& path : paths) {
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }
    if (QFileInfo(trimmed).suffix().compare(QStringLiteral("pdf"),
                                            Qt::CaseInsensitive) == 0) {
      supported.push_back(trimmed);
    }
  }
  return supported;
}

int ImportViewModel::contentIndex() const noexcept {
  return hasDraft() ? 1 : 0;
}

bool ImportViewModel::hasImportWorkflow() const noexcept {
  return importWorkflow_ != nullptr;
}

bool ImportViewModel::hasDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->hasDraft();
}

bool ImportViewModel::hasDraftNavigation() const noexcept {
  return selectors_ && !selectors_->attachedImportDraftIds().isEmpty();
}

bool ImportViewModel::canClearImport() const noexcept {
  return importWorkflow_ && !importWorkflow_->isRunning();
}

bool ImportViewModel::canCancel() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

bool ImportViewModel::canPause() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

bool ImportViewModel::canStart() const noexcept {
  if (!importWorkflow_ || importWorkflow_->isRunning()) {
    return false;
  }
  return !importWorkflow_->selectedFile().isEmpty() ||
         importWorkflow_->queuedCount() > 0;
}

bool ImportViewModel::importRunning() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

bool ImportViewModel::importPaused() const noexcept {
  return importWorkflow_ && importWorkflow_->isPaused();
}

QString ImportViewModel::pauseText() const {
  return importWorkflow_ && importWorkflow_->isPaused() ? tr("Resume")
                                                        : tr("Pause");
}

QString ImportViewModel::progressText() const {
  if (!importWorkflow_) {
    return tr("Ready");
  }
  if (!importWorkflow_->error().isEmpty()) {
    return importWorkflow_->error();
  }
  return importWorkflow_->phase().isEmpty() ? tr("Ready")
                                            : importWorkflow_->phase();
}

bool ImportViewModel::progressHasError() const noexcept {
  return importWorkflow_ && !importWorkflow_->error().isEmpty();
}

double ImportViewModel::progressValue() const noexcept {
  return importWorkflow_ ? importWorkflow_->progress() : 0.0;
}

int ImportViewModel::queuedCount() const noexcept {
  return importWorkflow_ ? importWorkflow_->queuedCount() : 0;
}

QVariantList ImportViewModel::importLogs() const {
  return selectors_ ? selectors_->importLogRows() : QVariantList{};
}

QString ImportViewModel::selectedDraftId() const {
  return importWorkflow_ ? importWorkflow_->currentDraftId() : QString();
}

QStringList ImportViewModel::importSourceLabels() const {
  return {tr("PDF")};
}

QStringList ImportViewModel::statementStrategyLabels() const {
  return {tr("Commerzbank26")};
}

void ImportViewModel::clearImport() {
  manualPathText_.clear();
  selectedImportFiles_.clear();
  appliedDefaultImportPath_.clear();
  if (importWorkflow_) {
    importWorkflow_->resetStatus();
  }
  emit changed();
}

void ImportViewModel::cancelCurrentImport() {
  if (importWorkflow_) {
    importWorkflow_->cancelImport();
  }
}

void ImportViewModel::cancelQueuedImports() {
  if (importWorkflow_) {
    importWorkflow_->cancelQueuedImports();
  }
}

void ImportViewModel::pauseImport() {
  if (importWorkflow_ && importWorkflow_->isRunning() &&
      !importWorkflow_->isPaused()) {
    importWorkflow_->pauseImport();
  }
}

void ImportViewModel::resumeImport() {
  if (importWorkflow_ && importWorkflow_->isRunning() &&
      importWorkflow_->isPaused()) {
    importWorkflow_->resumeImport();
  }
}

void ImportViewModel::startImport() {
  if (importWorkflow_) {
    importWorkflow_->startStatementImport();
  }
}

int ImportViewModel::activeDraftStackIndex() const {
  if (!importWorkflow_ || !selectors_) {
    return -1;
  }
  const auto ids = selectors_->attachedImportDraftIds();
  if (ids.isEmpty() || !importWorkflow_->hasDraft() ||
      importWorkflow_->currentDraftId().isEmpty()) {
    return -1;
  }
  return ids.indexOf(importWorkflow_->currentDraftId());
}

bool ImportViewModel::openDraftAtStackIndex(int index) {
  if (!importWorkflow_ || !selectors_) {
    return false;
  }
  const auto ids = selectors_->attachedImportDraftIds();
  if (index < 0 || index >= ids.size()) {
    return false;
  }
  return importWorkflow_->openStoredDraft(ids.at(index));
}

void ImportViewModel::selectPreviousDraft() {
  if (!importWorkflow_ || !selectors_) {
    return;
  }
  const auto ids = selectors_->attachedImportDraftIds();
  if (ids.isEmpty()) {
    return;
  }
  const int index = activeDraftStackIndex();
  if (index < 0 || index >= ids.size()) {
    openDraftAtStackIndex(ids.size() - 1);
    return;
  }
  if (index == 0) {
    importWorkflow_->clearDraft();
    emit changed();
    return;
  }
  openDraftAtStackIndex((index + ids.size() - 1) % ids.size());
  emit changed();
}

void ImportViewModel::selectNextDraft() {
  if (!importWorkflow_ || !selectors_) {
    return;
  }
  const auto ids = selectors_->attachedImportDraftIds();
  if (ids.isEmpty()) {
    return;
  }
  const int index = activeDraftStackIndex();
  if (index < 0 || index >= ids.size()) {
    openDraftAtStackIndex(0);
    return;
  }
  if (index == ids.size() - 1) {
    importWorkflow_->clearDraft();
    emit changed();
    return;
  }
  openDraftAtStackIndex((index + 1) % ids.size());
  emit changed();
}

void ImportViewModel::openImportLog(const QString& logId, bool draftAttached,
                                    const QString& statementId,
                                    const QString& draftId) {
  if (!importWorkflow_) {
    return;
  }
  if (draftAttached) {
    const QString targetDraftId = !draftId.isEmpty() ? draftId : logId;
    observability::traceViewModel("ImportViewModel::openImportLog",
                                  "Import draft log opened",
                                  {{"logId", logId.toStdString()},
                                   {"draftId", targetDraftId.toStdString()}});
    const QString currentDraftId = importWorkflow_->currentDraftId();
    if (targetDraftId != currentDraftId) {
      importWorkflow_->openStoredDraft(targetDraftId);
    }
    if (navigation_) {
      navigation_->setSection(Navigation::Section::Import);
    }
    return;
  }

  if (statementId.isEmpty() || !selection_) {
    return;
  }
  selection_->selectTransaction(statementId, {});
  observability::traceViewModel("ImportViewModel::openImportLog",
                                "Import statement log opened",
                                {{"logId", logId.toStdString()},
                                 {"statementId", statementId.toStdString()}});
  if (navigation_) {
    navigation_->setSection(Navigation::Section::Booking);
  }
}

void ImportViewModel::deleteImportLog(const QString& logId, bool draftAttached,
                                      const QString& draftId) {
  if (importWorkflow_) {
    importWorkflow_->removeAttachedImportLog(logId, draftAttached, draftId);
  } else if (commands_ && !logId.isEmpty()) {
    commands_->deleteImportLog(logId);
  }
}

void ImportViewModel::setStatusText(const QString& text) {
  if (status_) {
    status_->setText(text);
  }
  emit changed();
}

QString ImportViewModel::statementName() const {
  return importWorkflow_ ? importWorkflow_->currentStatementName() : QString();
}

void ImportViewModel::setStatementName(const QString& value) {
  if (!importWorkflow_ || statementName() == value) {
    return;
  }
  if (importWorkflow_->renameCurrentStatementDraft(value)) {
    emit changed();
  }
}

int ImportViewModel::currentTransactionNumber() const noexcept {
  if (!hasDraft()) {
    return 0;
  }
  return importWorkflow_->currentTransactionIndex() + 1;
}

int ImportViewModel::transactionCount() const noexcept {
  return importWorkflow_ ? importWorkflow_->transactionCount() : 0;
}

bool ImportViewModel::canDeleteTransaction() const noexcept {
  return importWorkflow_ && importWorkflow_->transactionCount() > 1;
}

bool ImportViewModel::canSelectPreviousTransactionDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->currentTransactionIndex() > 0;
}

bool ImportViewModel::canSelectNextTransactionDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->currentTransactionIndex() <
                                importWorkflow_->transactionCount() - 1;
}

void ImportViewModel::returnToImport() {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->pauseActiveDraft();
  emit changed();
}

void ImportViewModel::discard() {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->discardActiveDraft();
  emit changed();
}

void ImportViewModel::addTransactionAfterCurrent() {
  if (!hasDraft() || !importWorkflow_) {
    return;
  }
  if (importWorkflow_->insertTransactionAfterCurrent() >= 0) {
    emit changed();
  }
}

void ImportViewModel::deleteCurrentTransaction() {
  if (!canDeleteTransaction() || !importWorkflow_) {
    return;
  }
  if (importWorkflow_->removeCurrentTransaction() >= 0) {
    emit changed();
  }
}

void ImportViewModel::finalize() {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->finalizeActiveDraft();
  emit changed();
}

void ImportViewModel::selectPreviousTransactionDraft() {
  if (!canSelectPreviousTransactionDraft()) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(
      importWorkflow_->currentTransactionIndex() - 1);
}

void ImportViewModel::selectNextTransactionDraft() {
  if (!canSelectNextTransactionDraft()) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(
      importWorkflow_->currentTransactionIndex() + 1);
}

void ImportViewModel::syncTransactionDraftFromWorkflow() {
  const QString previousId = lastTransactionId_;
  reloadCurrentTransactionView();
  const QString id = currentTransactionView_.id;
  if (!id.isEmpty() && id == previousId) {
    return;
  }
  lastTransactionId_ = id;
  resetTransactionDraftFields();
}

void ImportViewModel::resetTransactionDraftFields() {
  nameText_ = currentTransactionView_.name;
  bookingDateText_ = currentTransactionView_.bookingDate;
  valutaText_ = currentTransactionView_.valuta;
  amountText_ = currentTransactionView_.amountText;
  actorName_.clear();
  contractName_.clear();
  contractType_.clear();
  contractAllocatableMode_ = payload::contract_allocatable_mode::kMixed;
  propertyName_.clear();
}

void ImportViewModel::reloadCurrentTransactionView() {
  currentTransactionView_ = {};
  if (!importWorkflow_) {
    return;
  }
  currentTransactionView_ = importWorkflow_->currentTransactionView();
}

void ImportViewModel::setNameText(const QString& value) {
  if (nameText_ == value) {
    return;
  }
  nameText_ = value;
  emit changed();
}

void ImportViewModel::setBookingDateText(const QString& value) {
  if (bookingDateText_ == value) {
    return;
  }
  bookingDateText_ = value;
  emit changed();
}

void ImportViewModel::setValutaText(const QString& value) {
  if (valutaText_ == value) {
    return;
  }
  valutaText_ = value;
  emit changed();
}

void ImportViewModel::setAmountText(const QString& value) {
  if (amountText_ == value) {
    return;
  }
  amountText_ = value;
  emit changed();
}

QVariantList ImportViewModel::statusOptions() const {
  return payload::transaction_status::options(
      ui::text::transactionStatus::neutral(),
      ui::text::transactionStatus::unverified(),
      ui::text::transactionStatus::verified(),
      ui::text::transactionStatus::completed());
}

int ImportViewModel::statusIndex() const {
  return payload::transaction_status::indexForStatus(
      statusOptions(), currentTransactionView_.status);
}

QString ImportViewModel::metadataText() const {
  return currentTransactionView_.metadata;
}

QString ImportViewModel::proofSource() const {
  return currentTransactionView_.proofSource;
}

bool ImportViewModel::effectiveAllocatable() const {
  return currentTransactionView_.effectiveAllocatable;
}

void ImportViewModel::commitNameText() {
  if (!importWorkflow_ || nameText_ == currentTransactionView_.name) {
    return;
  }
  importWorkflow_->commitCurrentTransactionName(nameText_);
}

void ImportViewModel::commitBookingDateText() {
  if (!importWorkflow_ ||
      bookingDateText_ == currentTransactionView_.bookingDate) {
    return;
  }
  importWorkflow_->commitCurrentTransactionBookingDate(bookingDateText_);
}

void ImportViewModel::commitValutaText() {
  if (!importWorkflow_ || valutaText_ == currentTransactionView_.valuta) {
    return;
  }
  importWorkflow_->commitCurrentTransactionValuta(valutaText_);
}

void ImportViewModel::commitAmountText() {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->applyCurrentTransactionAmountText(amountText_);
}

void ImportViewModel::selectStatusAtIndex(int index) {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->setCurrentTransactionStatus(
      payload::transaction_status::statusAt(statusOptions(), index));
}

void ImportViewModel::toggleAllocatable() {
  if (!importWorkflow_) {
    return;
  }
  importWorkflow_->setCurrentTransactionAllocatable(!effectiveAllocatable());
}

QVariantList ImportViewModel::actorOptions() const {
  return selectors_ ? selectors_->actorDropdownRows() : QVariantList{};
}

int ImportViewModel::selectedActorOptionIndex() const {
  return payload::mapper::indexById(actorOptions(),
                                    currentTransactionActorId());
}

void ImportViewModel::setActorName(const QString& value) {
  if (actorName_ == value) {
    return;
  }
  actorName_ = value;
  emit changed();
}

bool ImportViewModel::canAddActor() const {
  return selectors_ && !strings::normalizedText(actorName_).isEmpty() &&
         selectors_->actorIdByName(actorName_).isEmpty();
}

void ImportViewModel::selectActorAtIndex(int index) {
  if (!importWorkflow_) {
    return;
  }
  const QString id =
      payload::mapper::rowId(payload::mapper::mapAt(actorOptions(), index));
  if (id.isEmpty()) {
    importWorkflow_->clearCurrentTransactionActor();
  } else {
    importWorkflow_->selectCurrentTransactionActor(id);
  }
}

void ImportViewModel::addActor() {
  if (!canAddActor() || !importWorkflow_) {
    return;
  }
  importWorkflow_->createActorForCurrentTransaction(actorName_);
}

QVariantList ImportViewModel::propertyOptions() const {
  return selectors_ ? selectors_->propertyDropdownRows() : QVariantList{};
}

void ImportViewModel::setPropertyName(const QString& value) {
  if (propertyName_ == value) {
    return;
  }
  propertyName_ = value;
  emit changed();
}

bool ImportViewModel::canAddProperty() const {
  return selectors_ && !strings::normalizedText(propertyName_).isEmpty() &&
         selectors_->propertyIdByName(propertyName_).isEmpty();
}

bool ImportViewModel::isPropertySelected(const QString& propertyId) const {
  return currentTransactionPropertyIds().contains(propertyId);
}

void ImportViewModel::setPropertySelected(const QString& propertyId,
                                          bool selected) {
  if (importWorkflow_) {
    importWorkflow_->setCurrentTransactionPropertySelected(propertyId,
                                                           selected);
  }
}

void ImportViewModel::addProperty() {
  if (!canAddProperty() || !importWorkflow_) {
    return;
  }
  if (importWorkflow_->createPropertyForCurrentTransaction(propertyName_)) {
    propertyName_.clear();
    emit changed();
  }
}

QVariantList ImportViewModel::contractOptions() const {
  return selectors_ ? selectors_->contractDropdownRows() : QVariantList{};
}

int ImportViewModel::selectedContractOptionIndex() const {
  return payload::mapper::indexById(contractOptions(),
                                    currentTransactionView_.contractId);
}

QString ImportViewModel::selectedContractType() const {
  return payload::mapper::rowType(
      payload::mapper::mapAt(contractOptions(), selectedContractOptionIndex()));
}

void ImportViewModel::selectContractAtIndex(int index) {
  if (!importWorkflow_) {
    return;
  }
  const QString id =
      payload::mapper::rowId(payload::mapper::mapAt(contractOptions(), index));
  if (id.isEmpty()) {
    importWorkflow_->clearCurrentTransactionContract();
  } else {
    importWorkflow_->selectCurrentTransactionContract(id);
  }
}

void ImportViewModel::setContractName(const QString& value) {
  if (contractName_ == value) {
    return;
  }
  contractName_ = value;
  emit changed();
}

void ImportViewModel::setContractType(const QString& value) {
  if (contractType_ == value) {
    return;
  }
  contractType_ = value;
  emit changed();
}

QString ImportViewModel::contractNamePlaceholder() const {
  if (selectors_) {
    return selectors_->nextContractName();
  }
  return tr("Contract 1");
}

QVariantList ImportViewModel::contractAllocatableModes() const {
  return payload::contract_allocatable_mode::options(
      tr("Mixed"), tr("All allocatable"), tr("Never allocatable"));
}

int ImportViewModel::contractAllocatableModeIndex() const {
  return payload::contract_allocatable_mode::indexForMode(
      contractAllocatableModes(), contractAllocatableMode_);
}

void ImportViewModel::setContractAllocatableModeIndex(int index) {
  const QString mode = payload::contract_allocatable_mode::modeAt(
      contractAllocatableModes(), index);
  if (contractAllocatableMode_ == mode) {
    return;
  }
  contractAllocatableMode_ = mode;
  emit changed();
}

bool ImportViewModel::canAddContract() const {
  if (!selectors_ || strings::normalizedText(contractType_).isEmpty()) {
    return false;
  }
  const QString actorId = currentTransactionActorId();
  QStringList actorIds;
  if (!actorId.trimmed().isEmpty()) {
    actorIds.push_back(actorId.trimmed());
  }
  const QString effectiveName = contractName_.trimmed().isEmpty()
                                    ? selectors_->nextContractName()
                                    : contractName_.trimmed();
  return selectors_
      ->contractIdBySignature(effectiveName, contractType_, actorIds,
                              currentTransactionPropertyIds())
      .isEmpty();
}

void ImportViewModel::addContract() {
  if (!canAddContract() || !importWorkflow_) {
    return;
  }
  if (importWorkflow_->createOrSelectContractForCurrentTransaction(
          contractName_, contractType_, contractAllocatableMode_)) {
    emit changed();
  }
}

QString ImportViewModel::displaySuggestionSummary(const QString& value) const {
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

QString ImportViewModel::currentTransactionId() const {
  return currentTransactionView_.id;
}

QStringList ImportViewModel::currentTransactionPropertyIds() const {
  return currentTransactionView_.propertyIds;
}

QString ImportViewModel::currentTransactionActorId() const {
  return currentTransactionView_.actorId;
}

int ImportViewModel::suggestionTone(double confidence) const {
  if (confidence >= 0.9) {
    return kSuggestionSuccess;
  }
  if (confidence >= 0.4) {
    return kSuggestionWarning;
  }
  return kSuggestionDanger;
}

double ImportViewModel::actorSuggestionConfidence() const {
  return currentTransactionView_.actorSuggestionConfidence;
}

QString ImportViewModel::actorSuggestionSummary() const {
  return displaySuggestionSummary(
      currentTransactionView_.actorSuggestionSummary);
}

double ImportViewModel::propertySuggestionConfidence() const {
  return currentTransactionView_.propertySuggestionConfidence;
}

QString ImportViewModel::propertySuggestionSummary() const {
  return displaySuggestionSummary(
      currentTransactionView_.propertySuggestionSummary);
}

double ImportViewModel::contractSuggestionConfidence() const {
  return currentTransactionView_.contractSuggestionConfidence;
}

QString ImportViewModel::contractSuggestionSummary() const {
  return displaySuggestionSummary(
      currentTransactionView_.contractSuggestionSummary);
}

double ImportViewModel::allocatableSuggestionConfidence() const {
  return currentTransactionView_.allocatableSuggestionConfidence;
}

QString ImportViewModel::allocatableSuggestionText() const {
  return displaySuggestionSummary(
      currentTransactionView_.allocatableSuggestionSummary);
}

} // namespace ui
