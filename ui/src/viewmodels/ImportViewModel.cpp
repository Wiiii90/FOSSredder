/**
 * @file ui/src/viewmodels/ImportViewModel.cpp
 * @brief Implements the import overview UI state adapter core.
 */

#include "ui/viewmodels/ImportViewModel.h"

#include <QFileInfo>

#include "ui/shell/AppActions.h"
#include "ui/shell/NavigationState.h"
#include "ui/shell/Settings.h"
#include "ui/shell/StatusState.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

ImportViewModel::ImportViewModel(QObject *parent) : QObject(parent) {}

void ImportViewModel::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  bindImportWorkflow(value);
  updateManualPathFromWorkflow();
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::setSettings(Settings *value) {
  if (settings_ == value) {
    return;
  }
  bindSettings(value);
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::setActions(Actions *value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emit changed();
}

void ImportViewModel::setNavigation(NavigationState *value) {
  if (navigation_ == value) {
    return;
  }
  navigation_ = value;
  emit changed();
}

void ImportViewModel::setStatus(StatusState *value) {
  if (status_ == value) {
    return;
  }
  status_ = value;
  emit changed();
}

void ImportViewModel::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  if (importWorkflow_) {
    importWorkflow_->setWorkspace(workspace_);
  }
  applyDefaultImportSelection();
  emit changed();
}

void ImportViewModel::bindImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (!importWorkflow_) {
    return;
  }
  connect(importWorkflow_, &ImportWorkflow::stateChanged, this, [this]() {
    updateManualPathFromWorkflow();
    emit changed();
  });
  connect(importWorkflow_, &ImportWorkflow::importCanceled, this,
          [this]() { setStatusText(tr("Import canceled")); });
  connect(importWorkflow_, &ImportWorkflow::importFinished, this,
          [this]() { setStatusText(tr("Import finished")); });
  connect(importWorkflow_, &ImportWorkflow::importFailed, this,
          [this](const QString &error) {
            setStatusText(error.isEmpty() ? tr("Import failed") : error);
          });
}

void ImportViewModel::bindSettings(Settings *value) {
  if (settings_) {
    disconnect(settings_, nullptr, this, nullptr);
  }
  settings_ = value;
  if (settings_) {
    connect(settings_, &Settings::importDefaultPathChanged,
            this, [this]() {
              applyDefaultImportSelection();
              emit changed();
            });
  }
}

void ImportViewModel::bindActions(Actions *value) {
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
          [this](const QString &path) { queueImportFiles(QStringList{path}); });
  connect(actions_, &Actions::importFilesDropped, this,
          [this](const QStringList &paths) { queueImportFiles(paths); });
}

void ImportViewModel::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (workspace_) {
    connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this, [this]() {
      applyDefaultImportSelection();
      emit changed();
    });
  }
}

void ImportViewModel::setManualPathText(const QString &value) {
  if (manualPathText_ == value) {
    return;
  }
  manualPathText_ = value;
  pendingFiles_.clear();
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
  for (const QString &file : files) {
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
  QStringList files = pendingFiles_;
  if (files.isEmpty() && !manualPathText_.trimmed().isEmpty()) {
    files.push_back(manualPathText_);
  }
  queueImportFiles(files);
  if (!files.isEmpty()) {
    manualPathText_.clear();
    pendingFiles_.clear();
    emit changed();
  }
}

void ImportViewModel::queueImportFiles(const QStringList &paths) {
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

void ImportViewModel::updateManualPathFromWorkflow() {
  if (!importWorkflow_ || !pendingFiles_.isEmpty()) {
    return;
  }
  const QString selectedFile = importWorkflow_->selectedFile();
  if (manualPathText_ != selectedFile) {
    manualPathText_ = selectedFile;
  }
}

void ImportViewModel::setPendingFiles(const QStringList &paths) {
  pendingFiles_ = paths;
  emit changed();
}

QStringList ImportViewModel::supportedImportFiles(
    const QStringList &paths) const {
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

int ImportViewModel::contentIndex() const noexcept { return hasDraft() ? 1 : 0; }

bool ImportViewModel::hasImportWorkflow() const noexcept {
  return importWorkflow_ != nullptr;
}

bool ImportViewModel::hasDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->hasDraft();
}

bool ImportViewModel::hasDraftNavigation() const noexcept {
  return workspace_ && !workspace_->attachedImportDraftIds().isEmpty();
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
  return workspace_ ? workspace_->importLogRows() : QVariantList{};
}

QString ImportViewModel::selectedDraftId() const {
  return importWorkflow_ ? importWorkflow_->currentDraftId() : QString();
}

QStringList ImportViewModel::importSourceLabels() const { return {tr("PDF")}; }

QStringList ImportViewModel::statementStrategyLabels() const {
  return {tr("Commerzbank26")};
}

void ImportViewModel::clearImport() {
  manualPathText_.clear();
  pendingFiles_.clear();
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

void ImportViewModel::cancelAllImports() {
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
  if (!importWorkflow_ || !workspace_) {
    return -1;
  }
  const auto ids = workspace_->attachedImportDraftIds();
  if (ids.isEmpty() || !importWorkflow_->hasDraft() ||
      importWorkflow_->currentDraftId().isEmpty()) {
    return -1;
  }
  return ids.indexOf(importWorkflow_->currentDraftId());
}

bool ImportViewModel::openDraftAtStackIndex(int index) {
  if (!importWorkflow_ || !workspace_) {
    return false;
  }
  const auto ids = workspace_->attachedImportDraftIds();
  if (index < 0 || index >= ids.size()) {
    return false;
  }
  return importWorkflow_->openPersistedDraft(ids.at(index));
}

void ImportViewModel::selectPreviousDraft() {
  if (!importWorkflow_ || !workspace_) {
    return;
  }
  importWorkflow_->rememberCurrentDraftTransactionIndex();
  const auto ids = workspace_->attachedImportDraftIds();
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
  if (!importWorkflow_ || !workspace_) {
    return;
  }
  importWorkflow_->rememberCurrentDraftTransactionIndex();
  const auto ids = workspace_->attachedImportDraftIds();
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

void ImportViewModel::openImportLog(const QString &logId, bool draftAttached,
                                    const QString &statementId,
                                    const QString &draftId) {
  if (!importWorkflow_) {
    return;
  }
  if (draftAttached) {
    const QString targetDraftId = !draftId.isEmpty() ? draftId : logId;
    const QString currentDraftId = importWorkflow_->currentDraftId();
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
  workspace_->selectTransaction(statementId, {});
  if (navigation_) {
    navigation_->setSection(NavigationState::Section::Booking);
  }
}

void ImportViewModel::deleteImportLog(const QString &logId, bool draftAttached,
                                      const QString &draftId) {
  if (draftAttached) {
    if (workspace_) {
      workspace_->clearStatementDraft(draftId);
    }
    if (importWorkflow_) {
      importWorkflow_->clearDraft();
    }
  }
  if (workspace_ && !logId.isEmpty()) {
    workspace_->deleteImportLog(logId);
  }
}

void ImportViewModel::setStatusText(const QString &text) {
  if (status_) {
    status_->setText(text);
  }
  emit changed();
}

} // namespace ui
