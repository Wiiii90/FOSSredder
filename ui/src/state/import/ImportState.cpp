/**
 * @file ui/src/state/import/ImportState.cpp
 * @brief Implements the import overview UI state adapter.
 */

#include "ui/state/import/ImportState.h"

#include <QFileInfo>

#include "ui/shell/AppActions.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/state/status/StatusState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

ImportState::ImportState(QObject *parent) : QObject(parent) {}

void ImportState::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  bindImportWorkflow(value);
  updateManualPathFromWorkflow();
  applyDefaultImportSelection();
  emit changed();
}

void ImportState::setSettingsViewModel(SettingsViewModel *value) {
  if (settingsViewModel_ == value) {
    return;
  }
  bindSettings(value);
  applyDefaultImportSelection();
  emit changed();
}

void ImportState::setActions(Actions *value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emit changed();
}

void ImportState::setNavigation(NavigationState *value) {
  if (navigation_ == value) {
    return;
  }
  navigation_ = value;
  emit changed();
}

void ImportState::setStatus(StatusState *value) {
  if (status_ == value) {
    return;
  }
  status_ = value;
  emit changed();
}

void ImportState::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  refreshFromWorkspace();
  emit changed();
}

void ImportState::setPageActivated(bool value) {
  if (pageActivated_ == value) {
    return;
  }
  pageActivated_ = value;
  emit changed();
}

int ImportState::contentIndex() const noexcept {
  return hasDraft() ? 1 : 0;
}

bool ImportState::hasImportWorkflow() const noexcept {
  return importWorkflow_ != nullptr;
}

bool ImportState::hasDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->draft();
}

bool ImportState::hasDraftNavigation() const noexcept {
  return importWorkflow_ && importWorkflow_->hasDraftStack();
}

bool ImportState::canOpenPreviousDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->hasPrevDraft();
}

bool ImportState::canOpenNextDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->hasNextDraft();
}

bool ImportState::canClear() const noexcept {
  return importWorkflow_ && !importWorkflow_->isRunning();
}

bool ImportState::canCancel() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

bool ImportState::canCancelAll() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning() &&
         importWorkflow_->queuedCount() > 0;
}

bool ImportState::canPause() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

bool ImportState::canStart() const noexcept {
  if (!importWorkflow_ || importWorkflow_->isRunning()) {
    return false;
  }
  return !importWorkflow_->selectedFile().isEmpty() ||
         importWorkflow_->queuedCount() > 0;
}

bool ImportState::importRunning() const noexcept {
  return importWorkflow_ && importWorkflow_->isRunning();
}

QString ImportState::pauseText() const {
  return importWorkflow_ && importWorkflow_->isPaused() ? tr("Resume")
                                                       : tr("Pause");
}

QString ImportState::progressText() const {
  if (!importWorkflow_) {
    return tr("Ready");
  }
  if (!importWorkflow_->error().isEmpty()) {
    return importWorkflow_->error();
  }
  return importWorkflow_->phase().isEmpty() ? tr("Ready")
                                            : importWorkflow_->phase();
}

bool ImportState::progressHasError() const noexcept {
  return importWorkflow_ && !importWorkflow_->error().isEmpty();
}

double ImportState::progressValue() const noexcept {
  return importWorkflow_ ? importWorkflow_->progress() : 0.0;
}

void ImportState::setManualPathText(const QString &value) {
  if (manualPathText_ == value) {
    return;
  }
  manualPathText_ = value;
  pendingFiles_.clear();
  emit changed();
}

QStringList ImportState::importFiles() const {
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

int ImportState::queuedCount() const noexcept {
  return importWorkflow_ ? importWorkflow_->queuedCount() : 0;
}

QObject *ImportState::runModel() const noexcept {
  return importWorkflow_ ? importWorkflow_->runs() : nullptr;
}

QString ImportState::selectedRunLogId() const {
  const StatementDraft *draft = currentDraft();
  return draft ? draft->draftId() : QString();
}

StatementDraft *ImportState::currentDraft() const noexcept {
  return importWorkflow_ ? importWorkflow_->draft() : nullptr;
}

void ImportState::activatePage() {
  setPageActivated(true);
  applyDefaultImportSelection();
}

void ImportState::applyDefaultImportSelection() {
  if (!importWorkflow_ || !settingsViewModel_ || importWorkflow_->isRunning()) {
    return;
  }
  if (importWorkflow_->queuedCount() > 0) {
    return;
  }
  const QString currentFile = importWorkflow_->selectedFile();
  const QString defaultPath = settingsViewModel_->importDefaultPath();
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

void ImportState::browseImportPdf() {
  if (actions_) {
    actions_->browseImportPdf();
  }
}

void ImportState::commitManualImportFiles() {
  QStringList files = pendingFiles_;
  if (files.isEmpty() && !manualPathText_.trimmed().isEmpty()) {
    files.push_back(manualPathText_);
  }
  addImportFiles(files);
  if (!files.isEmpty()) {
    manualPathText_.clear();
    pendingFiles_.clear();
    emit changed();
  }
}

void ImportState::addImportFiles(const QStringList &paths) {
  if (!importWorkflow_) {
    return;
  }
  const QStringList supported = supportedImportFiles(paths);
  if (supported.isEmpty()) {
    return;
  }
  importWorkflow_->addFiles(supported);
  pendingFiles_.clear();
  emit changed();
}

void ImportState::resetStatus() {
  if (importWorkflow_) {
    importWorkflow_->resetStatus();
  }
}

void ImportState::cancelImport() {
  if (importWorkflow_) {
    importWorkflow_->cancelImport();
  }
}

void ImportState::cancelAllImports() {
  if (importWorkflow_) {
    importWorkflow_->cancelAllImports();
  }
}

void ImportState::togglePause() {
  if (importWorkflow_) {
    importWorkflow_->togglePause();
  }
}

void ImportState::startImport() {
  if (importWorkflow_) {
    importWorkflow_->startStatementImport();
  }
}

void ImportState::openPreviousDraft() {
  if (importWorkflow_) {
    importWorkflow_->openPrevDraft();
  }
}

void ImportState::openNextDraft() {
  if (importWorkflow_) {
    importWorkflow_->openNextDraft();
  }
}

void ImportState::refreshFromWorkspace() {
  if (importWorkflow_) {
    importWorkflow_->refreshFromStateSnapshot();
  }
  applyDefaultImportSelection();
  emit changed();
}

void ImportState::activateRun(int index, const QString &logId,
                              bool draftAttached, const QString &statementId,
                              const QString &draftId) {
  if (!importWorkflow_) {
    return;
  }
  if (draftAttached) {
    const QString targetDraftId = !draftId.isEmpty() ? draftId : logId;
    const QString currentDraftId =
        importWorkflow_->draft() ? importWorkflow_->draft()->draftId()
                                 : QString();
    importWorkflow_->activateRunAt(index);
    if (targetDraftId != currentDraftId) {
      importWorkflow_->openPersistedDraft(targetDraftId);
    }
    if (navigation_) {
      navigation_->setSection(NavigationState::Section::Import);
    }
    return;
  }

  if (statementId.isEmpty() || !workspace_) {
    return;
  }
  workspace_->setSelectedStatementId(statementId);
  workspace_->setSelectedTransactionId(QString());
  if (navigation_) {
    navigation_->setSection(NavigationState::Section::Booking);
  }
}

void ImportState::deleteRun(int index, bool draftAttached,
                            const QString &draftId) {
  if (!importWorkflow_ || !importWorkflow_->runs()) {
    return;
  }
  if (draftAttached) {
    importWorkflow_->clearPersistedStatementDraft(draftId);
    importWorkflow_->clearDraft();
  }
  importWorkflow_->removeRunAt(index);
}

void ImportState::bindImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (!importWorkflow_) {
    return;
  }
  connect(importWorkflow_, &ImportWorkflow::stateChanged, this, [this]() {
    updateManualPathFromWorkflow();
    applyDefaultImportSelection();
    emit changed();
  });
  connect(importWorkflow_, &ImportWorkflow::importCanceled, this, [this]() {
    setStatusText(tr("Import canceled"));
  });
  connect(importWorkflow_, &ImportWorkflow::importFinished, this, [this]() {
    setStatusText(tr("Import finished"));
  });
  connect(importWorkflow_, &ImportWorkflow::importFailed, this,
          [this](const QString &error) {
            setStatusText(error.isEmpty() ? tr("Import failed") : error);
          });
}

void ImportState::bindSettings(SettingsViewModel *value) {
  if (settingsViewModel_) {
    disconnect(settingsViewModel_, nullptr, this, nullptr);
  }
  settingsViewModel_ = value;
  if (settingsViewModel_) {
    connect(settingsViewModel_, &SettingsViewModel::importDefaultPathChanged,
            this, [this]() {
              applyDefaultImportSelection();
              emit changed();
            });
  }
}

void ImportState::bindActions(Actions *value) {
  if (actions_) {
    disconnect(actions_, nullptr, this, nullptr);
  }
  actions_ = value;
  if (!actions_) {
    return;
  }
  connect(actions_, &Actions::importFileSelected, this,
          [this](const QString &path) {
            manualPathText_ = path;
            setPendingFiles(QStringList{path});
          });
  connect(actions_, &Actions::importFilesSelected, this,
          [this](const QStringList &paths) {
            manualPathText_ = paths.isEmpty() ? QString{} : paths.front();
            setPendingFiles(paths);
          });
  connect(actions_, &Actions::importFileDropped, this,
          [this](const QString &path) { addImportFiles(QStringList{path}); });
  connect(actions_, &Actions::importFilesDropped, this,
          [this](const QStringList &paths) { addImportFiles(paths); });
}

void ImportState::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (workspace_) {
    connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
            &ImportState::refreshFromWorkspace);
  }
}

void ImportState::updateManualPathFromWorkflow() {
  if (!importWorkflow_ || !pendingFiles_.isEmpty()) {
    return;
  }
  const QString selectedFile = importWorkflow_->selectedFile();
  if (manualPathText_ != selectedFile) {
    manualPathText_ = selectedFile;
  }
}

void ImportState::setPendingFiles(const QStringList &paths) {
  pendingFiles_ = paths;
  emit changed();
}

void ImportState::setStatusText(const QString &text) {
  if (status_) {
    status_->setText(text);
  }
  emit changed();
}

QStringList ImportState::supportedImportFiles(const QStringList &paths) const {
  QStringList supported;
  for (const QString &path : paths) {
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

} // namespace ui
