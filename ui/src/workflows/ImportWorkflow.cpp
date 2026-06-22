/**
 * @file ui/src/workflows/ImportWorkflow.cpp
 * @brief Implements asynchronous import runs and draft session restoration.
 */

#include "ui/workflows/ImportWorkflow.h"

#include "ui/adapters/ImportAdapter.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/i18n/Text.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/shell/Defaults.h"
#include "ui/util/StringConversions.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include <QMetaObject>
#include <QUuid>

namespace ui::importing {

class ImportWorkflowState {
public:
  ImportWorkflowState() = default;

  bool isRunning() const noexcept {
    return isRunning_;
  }
  bool isPaused() const noexcept {
    return paused_;
  }
  double progress() const noexcept {
    return progress_;
  }
  const QString& phase() const noexcept {
    return phase_;
  }
  const QString& error() const noexcept {
    return error_;
  }
  const QString& selectedFile() const noexcept {
    return selectedFile_;
  }
  const QStringList& queuedFiles() const noexcept {
    return queuedFiles_;
  }
  bool hasDraft() const noexcept {
    return hasDraft_;
  }
  core::ports::importing::draft::StatementDraft* draft() noexcept {
    return hasDraft_ ? &draft_ : nullptr;
  }
  const core::ports::importing::draft::StatementDraft* draft() const noexcept {
    return hasDraft_ ? &draft_ : nullptr;
  }
  const core::ports::workspace::WorkspaceSnapshot&
  catalogSnapshot() const noexcept {
    return catalogSnapshot_;
  }
  int currentTransactionIndex() const noexcept {
    return currentTransactionIndex_;
  }
  void setCurrentTransactionIndex(int index);
  int artifactCount() const noexcept {
    return artifactCount_;
  }
  bool cancelRequested() const noexcept {
    return canceled_;
  }

  bool setSelectedFile(const QString& path);
  bool addFiles(const QStringList& paths);
  bool resetStatus();
  bool clearDraft();

  QString currentImportFile() const;
  QString takeSelectedFileForStart();
  bool takeNextQueuedFile(QString& nextFile);

  void beginImport(const QString& path);
  void rejectStart(const QString& errorMessage);
  void beginCancel(bool clearQueue);
  bool setPaused(bool paused);
  void recordCanceled();
  void recordFailed(const QString& errorMessage);
  void recordFinished();
  bool
  populateDraft(const core::ports::importing::draft::StatementDraft& draft,
                const core::ports::workspace::WorkspaceSnapshot& state,
                const std::map<std::string, std::vector<uint8_t>>& artifacts,
                int currentTransactionIndex);
  bool restoreDraft(const core::ports::importing::draft::StatementDraft& draft,
                    const core::ports::workspace::WorkspaceSnapshot& state,
                    const QString& draftId, int currentTransactionIndex);
  void updateProgress(double progress, const QString& phase);

private:
  void clearDraftState();
  void clearTransientImportState();
  void resetCancellationState();

  bool isRunning_ = false;
  double progress_ = 0.0;
  QString phase_;
  QString error_;
  QString selectedFile_;
  QStringList queuedFiles_;
  core::ports::importing::draft::StatementDraft draft_;
  bool hasDraft_ = false;
  int currentTransactionIndex_ = 0;
  core::ports::workspace::WorkspaceSnapshot catalogSnapshot_;
  int artifactCount_ = 0;
  bool canceled_ = false;
  bool paused_ = false;
  bool cancelClearsQueue_ = false;
  QString currentImportFile_;
};

void ImportWorkflowState::clearDraftState() {
  draft_ = {};
  catalogSnapshot_ = {};
  hasDraft_ = false;
  currentTransactionIndex_ = 0;
}

bool ImportWorkflowState::setSelectedFile(const QString& path) {
  if (selectedFile_ == path) {
    return false;
  }
  selectedFile_ = path;
  return true;
}

bool ImportWorkflowState::addFiles(const QStringList& paths) {
  QStringList cleaned;
  cleaned.reserve(paths.size());
  for (const auto& path : paths) {
    const auto trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }
    cleaned.push_back(trimmed);
  }
  if (cleaned.isEmpty()) {
    return false;
  }

  bool changed = false;
  const bool hasSelection = !selectedFile_.trimmed().isEmpty();
  int startIndex = 0;
  if (!hasSelection) {
    changed = setSelectedFile(cleaned.front()) || changed;
    startIndex = 1;
  }

  for (int i = startIndex; i < cleaned.size(); ++i) {
    if (cleaned[i] == selectedFile_ || queuedFiles_.contains(cleaned[i])) {
      continue;
    }
    queuedFiles_.push_back(cleaned[i]);
    changed = true;
  }

  return changed;
}

bool ImportWorkflowState::resetStatus() {
  if (isRunning_) {
    return false;
  }

  phase_.clear();
  error_.clear();
  progress_ = ui::config::importProgress::kMinimum;
  canceled_ = false;
  cancelClearsQueue_ = false;
  paused_ = false;
  selectedFile_.clear();
  currentImportFile_.clear();
  queuedFiles_.clear();
  artifactCount_ = 0;
  return true;
}

bool ImportWorkflowState::clearDraft() {
  clearDraftState();

  artifactCount_ = 0;
  if (!isRunning_ && queuedFiles_.isEmpty()) {
    selectedFile_.clear();
    currentImportFile_.clear();
  }
  return !isRunning_ && !queuedFiles_.isEmpty();
}

void ImportWorkflowState::setCurrentTransactionIndex(int index) {
  if (!hasDraft_ || draft_.transactions.empty()) {
    currentTransactionIndex_ = 0;
    return;
  }
  const int maxIndex = static_cast<int>(draft_.transactions.size()) - 1;
  currentTransactionIndex_ = std::clamp(index, 0, maxIndex);
}

void ImportWorkflowState::recordFinished() {
  error_.clear();
  phase_ = ui::text::importing::phaseFinished();
  clearTransientImportState();
  resetCancellationState();
  selectedFile_.clear();
  currentImportFile_.clear();
}

QString ImportWorkflowState::currentImportFile() const {
  return currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_;
}

QString ImportWorkflowState::takeSelectedFileForStart() {
  const QString trimmed = selectedFile_.trimmed();
  if (trimmed.isEmpty()) {
    return {};
  }
  selectedFile_.clear();
  return trimmed;
}

bool ImportWorkflowState::takeNextQueuedFile(QString& nextFile) {
  if (isRunning_ || queuedFiles_.isEmpty()) {
    return false;
  }
  nextFile = queuedFiles_.takeFirst();
  return true;
}

void ImportWorkflowState::clearTransientImportState() {
  isRunning_ = false;
  progress_ = ui::config::importProgress::kMinimum;
}

void ImportWorkflowState::resetCancellationState() {
  canceled_ = false;
  cancelClearsQueue_ = false;
  paused_ = false;
}

void ImportWorkflowState::beginImport(const QString& path) {
  selectedFile_ = path;
  currentImportFile_ = path;

  artifactCount_ = 0;
  error_.clear();
  phase_ = ui::text::importing::phaseStarting();
  progress_ = ui::config::importProgress::kInitial;
  isRunning_ = true;
  resetCancellationState();
}

void ImportWorkflowState::rejectStart(const QString& errorMessage) {
  error_ = errorMessage;
  queuedFiles_.clear();
  currentImportFile_.clear();
  phase_.clear();
  clearTransientImportState();
  resetCancellationState();
}

void ImportWorkflowState::beginCancel(bool clearQueue) {
  if (!isRunning_) {
    return;
  }
  canceled_ = true;
  paused_ = false;
  cancelClearsQueue_ = clearQueue;
  if (clearQueue) {
    queuedFiles_.clear();
  }
  phase_ = ui::text::importing::phaseStopping();
}

bool ImportWorkflowState::setPaused(bool paused) {
  if (!isRunning_ || canceled_) {
    return false;
  }
  if (paused_ == paused) {
    return false;
  }
  paused_ = paused;
  phase_ = paused_ ? ui::text::importing::phasePaused()
                   : ui::text::importing::phaseRunning();
  return true;
}

void ImportWorkflowState::recordCanceled() {
  error_.clear();
  if (cancelClearsQueue_) {
    queuedFiles_.clear();
  }
  phase_ = ui::text::importing::phaseCanceled();
  clearTransientImportState();
  resetCancellationState();
  selectedFile_.clear();
  currentImportFile_.clear();
}

void ImportWorkflowState::recordFailed(const QString& errorMessage) {
  error_ = errorMessage;
  queuedFiles_.clear();
  phase_ = ui::text::importing::phaseFailed();
  clearTransientImportState();
  resetCancellationState();
  currentImportFile_.clear();
}

bool ImportWorkflowState::populateDraft(
    const core::ports::importing::draft::StatementDraft& draft,
    const core::ports::workspace::WorkspaceSnapshot& state,
    const std::map<std::string, std::vector<uint8_t>>& artifacts,
    int currentTransactionIndex) {
  if (draft.id.empty() && draft.name.empty()) {
    return false;
  }

  artifactCount_ = static_cast<int>(artifacts.size());

  clearDraftState();
  draft_ = draft;
  catalogSnapshot_ = state;
  hasDraft_ = true;
  setCurrentTransactionIndex(currentTransactionIndex);
  recordFinished();
  return true;
}

bool ImportWorkflowState::restoreDraft(
    const core::ports::importing::draft::StatementDraft& draft,
    const core::ports::workspace::WorkspaceSnapshot& state,
    const QString& draftId, int currentTransactionIndex) {
  if (draft.id.empty() && draft.name.empty()) {
    return false;
  }

  clearDraftState();
  draft_ = draft;
  if (!draftId.isEmpty()) {
    draft_.id = ui::strings::toStdString(draftId);
  }
  catalogSnapshot_ = state;
  hasDraft_ = true;
  setCurrentTransactionIndex(currentTransactionIndex);
  return hasDraft_;
}

void ImportWorkflowState::updateProgress(double progress,
                                         const QString& phase) {
  if (!isRunning_ || canceled_ || paused_) {
    return;
  }

  progress_ = progress;
  if (phase.isEmpty()) {
    return;
  }
  phase_ = phase;
}

} // namespace ui::importing

namespace ui {

namespace {

QString newLogId() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

double clampedProgress(double progress) {
  if (progress < ui::config::importProgress::kMinimum) {
    return ui::config::importProgress::kMinimum;
  }
  if (progress > ui::config::importProgress::kMaximum) {
    return ui::config::importProgress::kMaximum;
  }
  return progress;
}

} // namespace

ImportWorkflow::ImportWorkflow(
    std::shared_ptr<ui::adapters::ImportAdapter> importAdapter,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    StateSnapshotProvider stateSnapshotProvider, WorkspaceCommands* commands,
    WorkspaceSelectors* selectors, QObject* parent)
    : QObject(parent),
      state_(std::make_unique<importing::ImportWorkflowState>()),
      importAdapter_(std::move(importAdapter)),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      commands_(commands), selectors_(selectors),
      errorReporter_(std::move(errorReporter)) {
  if (!errorReporter_) {
    throw std::invalid_argument("ImportWorkflow requires an error reporter");
  }
}

ImportWorkflow::~ImportWorkflow() = default;

void ImportWorkflow::setWorkspaceRoles(WorkspaceCommands* commands,
                                       WorkspaceSelectors* selectors) {
  commands_ = commands;
  selectors_ = selectors;
  emit stateChanged();
}

bool ImportWorkflow::isRunning() const noexcept {
  return state_->isRunning();
}

bool ImportWorkflow::isPaused() const noexcept {
  return state_->isPaused();
}

double ImportWorkflow::progress() const noexcept {
  return state_->progress();
}

QString ImportWorkflow::phase() const {
  return state_->phase();
}

QString ImportWorkflow::error() const {
  return state_->error();
}

QString ImportWorkflow::selectedFile() const {
  return state_->selectedFile();
}

int ImportWorkflow::queuedCount() const noexcept {
  return static_cast<int>(state_->queuedFiles().size());
}

QStringList ImportWorkflow::queuedFiles() const {
  return state_->queuedFiles();
}

bool ImportWorkflow::hasDraft() const noexcept {
  return state_->hasDraft();
}

int ImportWorkflow::currentTransactionIndex() const noexcept {
  return state_->currentTransactionIndex();
}

bool ImportWorkflow::hasActiveImportHandle() const noexcept {
  return hasActiveImportHandle_ && !activeImportHandle_.importId.empty();
}

void ImportWorkflow::clearActiveImportSubscription() {
  if (importAdapter_ && hasActiveImportHandle()) {
    importAdapter_->unsubscribe(activeImportHandle_);
  }
  activeImportHandle_ = {};
  hasActiveImportHandle_ = false;
}

void ImportWorkflow::saveImportLog(const QString& logId, const QString& status,
                                   const QString& message, bool draftAttached,
                                   const QString& draftId,
                                   const QString& statementId,
                                   const QString& importFile) {
  if (!commands_) {
    return;
  }
  const QString sourceFile =
      importFile.isEmpty() ? state_->currentImportFile() : importFile;
  commands_->upsertImportLog(logId, status, message, draftAttached, draftId,
                             statementId, sourceFile);
}

void ImportWorkflow::clearStoredDraft(const QString& draftId) {
  if (!commands_) {
    return;
  }
  try {
    commands_->clearStatementDraft(draftId);
  } catch (...) {
    reportException(observability::origins::workflow::import::kFinalize,
                    std::current_exception());
  }
}

void ImportWorkflow::pauseActiveDraft() {
  if (!commands_ || !state_->hasDraft()) {
    return;
  }
  const QString draftId = currentDraftId();
  flushActiveDraftToWorkspace();
  saveImportLog(draftId, text::importing::statusDraft(),
                text::importing::messageDraftPaused(), true, draftId);
  clearDraft();
}

void ImportWorkflow::discardActiveDraft() {
  if (!commands_ || !state_->hasDraft()) {
    return;
  }
  const QString draftId = currentDraftId();
  flushActiveDraftToWorkspace();
  clearStoredDraft(draftId);
  saveImportLog(draftId, text::importing::statusDraftDiscarded(),
                text::importing::messageDraftDiscarded(), false, {}, {},
                state_->selectedFile());
  clearDraft();
}

void ImportWorkflow::finalizeActiveDraft() {
  if (!commands_ || !state_->hasDraft()) {
    return;
  }
  const QString draftId = currentDraftId();
  observability::traceWorkflow(
      observability::origins::workflow::import::kFinalize,
      "Statement draft finalize submitted",
      {{"draftId", draftId.toStdString()}});

  QString statementId;
  try {
    statementId = finalizeCurrentStatementDraft();
  } catch (...) {
    reportException(observability::origins::workflow::import::kFinalize,
                    std::current_exception());
  }

  if (statementId.isEmpty()) {
    saveImportLog(draftId, text::importing::statusFinalizeFailed(),
                  text::importing::messageFinalizeFailed(), true, draftId);
    return;
  }

  clearStoredDraft(draftId);
  saveImportLog(draftId, text::importing::statusFinalized(),
                text::importing::messageFinalized(), false, {}, statementId);
  clearDraft();
}

void ImportWorkflow::removeAttachedImportLog(const QString& logId,
                                             bool draftAttached,
                                             const QString& draftId) {
  if (draftAttached) {
    clearStoredDraft(draftId);
    clearDraft();
  }
  if (commands_ && !logId.isEmpty()) {
    commands_->deleteImportLog(logId);
  }
}

void ImportWorkflow::addFiles(const QStringList& paths) {
  if (state_->addFiles(paths)) {
    emit stateChanged();
  }
}

void ImportWorkflow::setSelectedFile(const QString& path) {
  if (state_->setSelectedFile(path)) {
    emit stateChanged();
  }
}

void ImportWorkflow::resetStatus() {
  if (state_->resetStatus()) {
    emit stateChanged();
  }
}

void ImportWorkflow::clearDraft() {
  rememberCurrentDraftTransactionIndex();
  const bool shouldStartNext = state_->clearDraft();
  activeDraftId_.clear();
  emit stateChanged();
  if (shouldStartNext) {
    startNextQueuedImport();
  }
}

void ImportWorkflow::rejectImportStart(const QString& errorMessage,
                                       const char* traceMessage) {
  state_->rejectStart(errorMessage);
  observability::reportFlow(core::errors::ErrorSeverity::Warning,
                            observability::codes::FlowImportRejected,
                            observability::origins::workflow::import::kStart,
                            traceMessage);
  emit stateChanged();
  emit importFailed(state_->error());
}

void ImportWorkflow::startNextQueuedImport() {
  QString next;
  if (!state_->takeNextQueuedFile(next)) {
    return;
  }
  emit stateChanged();
  startImportForFile(next);
}

void ImportWorkflow::startStatementImport() {
  if (state_->isRunning() || state_->hasDraft()) {
    return;
  }

  const auto selected = state_->takeSelectedFileForStart();
  if (!selected.isEmpty()) {
    emit stateChanged();
    startImportForFile(selected);
    return;
  }

  startNextQueuedImport();
}

void ImportWorkflow::startImportForFile(const QString& path) {
  if (state_->isRunning()) {
    return;
  }
  if (!importAdapter_) {
    rejectImportStart(text::workflowErrors::importWorkflowUnavailable(),
                      "Import start rejected: workflow unavailable");
    return;
  }
  if (path.trimmed().isEmpty()) {
    rejectImportStart(text::workflowErrors::noFileSelected(),
                      "Import start rejected: no file selected");
    return;
  }

  state_->beginImport(path);
  activeImportLogId_ = newLogId();
  activeImportTerminalHandled_ = false;
  hasPendingTerminalEvent_ = false;
  activeImportHandle_ = {};
  hasActiveImportHandle_ = false;
  pendingTerminalState_ = core::ports::importing::StatementImportState::Pending;
  pendingTerminalMessage_.clear();
  saveImportLog(activeImportLogId_, text::importing::statusRunning(),
                text::importing::phaseStarting());
  emit stateChanged();

  observability::traceWorkflow(
      observability::origins::workflow::import::kStart, "Import submitted",
      {{observability::context::kFile, strings::toStdString(path)},
       {observability::context::kQueuedCount,
        std::to_string(state_->queuedFiles().size())}});
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportStarted,
      observability::origins::workflow::import::kStart, "Import started",
      {{observability::context::kFile, strings::toStdString(path)},
       {observability::context::kQueuedCount,
        std::to_string(state_->queuedFiles().size())}});

  core::ports::importing::ImportRequest request;
  request.sourcePath = strings::toEncodedPath(path);
  activeImportHandle_ = importAdapter_->startStatementImport(
      request,
      [this](const core::ports::importing::StatementImportEvent& event) {
        handleImportEvent(event);
      });
  hasActiveImportHandle_ = !activeImportHandle_.importId.empty();

  if (!hasActiveImportHandle_) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: unable to start job");
  }
}

void ImportWorkflow::updateProgress(double progress, const QString& phase) {
  state_->updateProgress(progress, phase);
  emit stateChanged();
}

void ImportWorkflow::reportException(const char* origin,
                                     std::exception_ptr exception) const {
  if (!errorReporter_) {
    return;
  }
  errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin,
                                  exception);
}

void ImportWorkflow::requestImportCancellation(bool clearQueue,
                                               const char* origin,
                                               const char* traceMessage) {
  if (!state_->isRunning()) {
    return;
  }

  const auto queuedBeforeCancel =
      clearQueue ? state_->queuedFiles() : QStringList{};
  state_->beginCancel(clearQueue);

  if (clearQueue && commands_) {
    for (const auto& queuedPath : queuedBeforeCancel) {
      commands_->upsertImportLog(newLogId(), text::importing::statusCanceled(),
                                 text::importing::messageCanceledBeforeStart(),
                                 false, {}, {}, queuedPath);
    }
  }

  if (importAdapter_ && hasActiveImportHandle()) {
    importAdapter_->cancel(activeImportHandle_);
  }
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled, origin, traceMessage,
      {{observability::context::kFile,
        strings::toStdString(state_->currentImportFile())},
       {observability::context::kQueuedCount,
        std::to_string(state_->queuedFiles().size())}});
  emit stateChanged();
}

void ImportWorkflow::cancelImport() {
  requestImportCancellation(false,
                            observability::origins::workflow::import::kCancel,
                            "Import cancellation requested");
}

void ImportWorkflow::cancelQueuedImports() {
  requestImportCancellation(
      true, observability::origins::workflow::import::kCancelQueued,
      "Cancel-all requested for import queue");
}

void ImportWorkflow::setImportPaused(bool paused) {
  if (state_->isPaused() == paused) {
    return;
  }
  if (!state_->setPaused(paused)) {
    return;
  }
  if (importAdapter_ && hasActiveImportHandle()) {
    if (state_->isPaused()) {
      importAdapter_->pause(activeImportHandle_);
    } else {
      importAdapter_->resume(activeImportHandle_);
    }
  }
  if (!activeImportLogId_.isEmpty()) {
    saveImportLog(activeImportLogId_,
                  state_->isPaused() ? text::importing::statusPaused()
                                     : text::importing::statusRunning(),
                  state_->isPaused() ? text::importing::messageImportPaused()
                                     : text::importing::messageImportResumed());
  }
  emit stateChanged();

  if (!state_->isPaused() && hasPendingTerminalEvent_) {
    const auto terminalState = pendingTerminalState_;
    const auto message = pendingTerminalMessage_;
    hasPendingTerminalEvent_ = false;
    pendingTerminalState_ =
        core::ports::importing::StatementImportState::Pending;
    pendingTerminalMessage_.clear();
    onJobTerminal(terminalState, message);
  }
}

void ImportWorkflow::pauseImport() {
  setImportPaused(true);
}

void ImportWorkflow::resumeImport() {
  setImportPaused(false);
}

void ImportWorkflow::handleImportEvent(
    const core::ports::importing::StatementImportEvent& event) {
  const double progress = clampedProgress(event.progress);
  const QString phase = QString::fromStdString(event.message);
  const auto eventState = event.state;
  const QString message = QString::fromStdString(event.message);

  QMetaObject::invokeMethod(
      this,
      [this, progress, phase, eventState, message]() {
        if (eventState ==
                core::ports::importing::StatementImportState::Finished ||
            eventState ==
                core::ports::importing::StatementImportState::Failed ||
            eventState ==
                core::ports::importing::StatementImportState::Canceled) {
          onJobTerminal(eventState, message);
          return;
        }
        updateProgress(progress, phase);
      },
      Qt::QueuedConnection);
}

void ImportWorkflow::handleImportCanceled() {
  state_->recordCanceled();
  if (!activeImportTerminalHandled_) {
    saveImportLog(activeImportLogId_, text::importing::statusCanceled(),
                  text::importing::phaseCanceled());
    activeImportTerminalHandled_ = true;
  }
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled,
      observability::origins::workflow::import::kTerminal, "Import canceled",
      {{observability::context::kStatus,
        strings::toStdString(text::importing::statusCanceled())}});
  emit stateChanged();
  emit importCanceled();
}

void ImportWorkflow::handleImportFailed(const QString& errorMessage,
                                        const char* traceMessage) {
  const QString traceDetail =
      traceMessage ? QString::fromUtf8(traceMessage) : QString();
  const QString visibleError =
      errorMessage == text::workflowErrors::importFailed() &&
              traceDetail.startsWith(QStringLiteral("Import failed:"))
          ? traceDetail
          : errorMessage;
  state_->recordFailed(visibleError);
  if (!activeImportTerminalHandled_) {
    saveImportLog(activeImportLogId_, text::importing::statusFailed(),
                  errorMessage);
    activeImportTerminalHandled_ = true;
  }
  observability::reportFlow(core::errors::ErrorSeverity::Warning,
                            observability::codes::FlowImportFailed,
                            observability::origins::workflow::import::kTerminal,
                            traceMessage,
                            {{observability::context::kError,
                              strings::toStdString(state_->error())}});
  emit stateChanged();
  emit importFailed(state_->error());
}

bool ImportWorkflow::populateDraftFromResult() {
  if (!importAdapter_ || !hasActiveImportHandle()) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: import runner unavailable");
    return false;
  }

  const auto imported = importAdapter_->importResult(activeImportHandle_);
  if (!imported.hasStatement) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: missing statement result");
    return false;
  }

  const auto snapshot = stateSnapshot();
  const QString draftId = activeImportLogId_;
  const bool hadVisibleDraft = state_->hasDraft();

  const auto preparedDraft = importAdapter_->buildStatementDraft(
      strings::toStdString(state_->currentImportFile()), imported.statement,
      snapshot, imported.transactions, strings::toStdString(draftId));
  if (preparedDraft.id.empty() && preparedDraft.transactions.empty()) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: unable to create draft state");
    return false;
  }

  const auto draftSnapshot =
      importAdapter_->buildStatementDraftSnapshot(preparedDraft, snapshot);
  if (!commands_ || draftSnapshot.id.empty()) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: unable to save draft state");
    return false;
  }
  commands_->saveStatementDraft(draftSnapshot);

  if (hadVisibleDraft) {
    state_->recordFinished();
  } else {
    if (!state_->populateDraft(preparedDraft, snapshot, imported.artifacts,
                               0)) {
      handleImportFailed(text::workflowErrors::importFailed(),
                         "Import failed: unable to create statement draft");
      return false;
    }
  }

  saveImportLog(activeImportLogId_, text::importing::statusDraft(),
                text::importing::messageDraftReady(), true, draftId);
  if (!hadVisibleDraft) {
    activeDraftId_ = draftId;
  }
  activeImportTerminalHandled_ = true;

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportFinished,
      observability::origins::workflow::import::kTerminal, "Import finished",
      {{observability::context::kStatus,
        strings::toStdString(text::importing::statusSuccess())},
       {observability::context::kArtifactCount,
        std::to_string(state_->artifactCount())}});
  emit stateChanged();
  emit importFinished();
  return true;
}

void ImportWorkflow::onJobTerminal(
    core::ports::importing::StatementImportState state,
    const QString& message) {
  if ((!state_->isRunning() && !state_->cancelRequested()) ||
      activeImportTerminalHandled_) {
    return;
  }

  if (state_->isPaused() &&
      state != core::ports::importing::StatementImportState::Canceled) {
    hasPendingTerminalEvent_ = true;
    pendingTerminalState_ = state;
    pendingTerminalMessage_ = message;
    return;
  }

  if (state == core::ports::importing::StatementImportState::Canceled ||
      state_->cancelRequested()) {
    clearActiveImportSubscription();
    handleImportCanceled();
    return;
  }

  if (state == core::ports::importing::StatementImportState::Failed) {
    clearActiveImportSubscription();
    handleImportFailed(message.isEmpty() ? text::workflowErrors::importFailed()
                                         : message,
                       "Import failed");
    return;
  }

  const bool populated = populateDraftFromResult();
  clearActiveImportSubscription();
  if (!populated) {
    return;
  }

  startNextQueuedImport();
}

QString ImportWorkflow::resolveDraftContextId() const {
  if (!activeDraftId_.isEmpty()) {
    return activeDraftId_;
  }
  return currentDraftId();
}

void ImportWorkflow::rememberCurrentDraftTransactionIndex() {
  const QString draftId = currentDraftId();
  if (draftId.isEmpty()) {
    return;
  }
  draftTransactionIndexByDraftId_.insert(draftId,
                                         state_->currentTransactionIndex());
}

int ImportWorkflow::rememberedDraftTransactionIndex(
    const QString& draftId) const {
  return draftTransactionIndexByDraftId_.value(draftId, 0);
}

bool ImportWorkflow::openStoredDraft(const QString& draftId) {
  if (!stateSnapshotProvider_ || !selectors_) {
    return false;
  }
  rememberCurrentDraftTransactionIndex();
  const QString currentDraftId = this->currentDraftId();

  QString requestedDraftId = !draftId.isEmpty() ? draftId : activeDraftId_;
  if (requestedDraftId.isEmpty()) {
    for (const auto& id : selectors_->attachedImportDraftIds()) {
      requestedDraftId = id;
      break;
    }
  }
  if (!requestedDraftId.isEmpty()) {
    if (!currentDraftId.isEmpty() && requestedDraftId != currentDraftId) {
      flushActiveDraftToWorkspace();
    }
    activeDraftId_ = requestedDraftId;
  }

  const bool restored = restoreDraftFromState(stateSnapshot());
  if (restored) {
    emit stateChanged();
  }
  return restored;
}

bool ImportWorkflow::restoreDraftFromState(
    const core::ports::workspace::WorkspaceSnapshot& snapshot) {
  if (snapshot.statementDrafts.empty() || !importAdapter_) {
    return false;
  }

  const core::ports::workspace::StatementDraftSnapshot* storedDraft = nullptr;
  const QString requestedDraftId = resolveDraftContextId();
  if (!requestedDraftId.isEmpty()) {
    for (const auto& draft : snapshot.statementDrafts) {
      if (QString::fromStdString(draft.id) == requestedDraftId) {
        storedDraft = &draft;
        break;
      }
    }
  }
  if (!storedDraft) {
    storedDraft = &snapshot.statementDrafts.front();
  }
  if (!storedDraft) {
    return false;
  }

  const QString restoredDraftId = QString::fromStdString(storedDraft->id);
  if (!restoredDraftId.isEmpty()) {
    activeDraftId_ = restoredDraftId;
  }

  auto draft = importAdapter_->restoreStatementDraft(*storedDraft);
  return state_->restoreDraft(draft, snapshot, restoredDraftId,
                              rememberedDraftTransactionIndex(restoredDraftId));
}

core::ports::importing::draft::TransactionDraft*
ImportWorkflow::currentTransactionDraft() {
  auto* draft = state_->draft();
  if (!draft) {
    return nullptr;
  }
  const int index = state_->currentTransactionIndex();
  if (index < 0 ||
      static_cast<std::size_t>(index) >= draft->transactions.size()) {
    return nullptr;
  }
  return &draft->transactions[static_cast<std::size_t>(index)];
}

const core::ports::importing::draft::TransactionDraft*
ImportWorkflow::currentTransactionDraft() const {
  return const_cast<ImportWorkflow*>(this)->currentTransactionDraft();
}

int ImportWorkflow::transactionCount() const noexcept {
  const auto* draft = state_->draft();
  return draft ? static_cast<int>(draft->transactions.size()) : 0;
}

core::ports::workspace::WorkspaceSnapshot
ImportWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

QString ImportWorkflow::currentDraftId() const {
  const auto* draft = state_->draft();
  return draft ? QString::fromStdString(draft->id) : QString();
}

QString ImportWorkflow::currentStatementName() const {
  const auto* draft = state_->draft();
  return draft ? QString::fromStdString(draft->name) : QString();
}

core::ports::importing::draft::StatementDraft*
ImportWorkflow::statementDraft() noexcept {
  return state_->draft();
}

const core::ports::importing::draft::StatementDraft*
ImportWorkflow::statementDraft() const noexcept {
  return state_->draft();
}

void ImportWorkflow::setCurrentTransactionIndex(int index) {
  if (!state_->hasDraft()) {
    return;
  }
  const int previous = state_->currentTransactionIndex();
  state_->setCurrentTransactionIndex(index);
  if (state_->currentTransactionIndex() != previous) {
    emit stateChanged();
  }
}

void ImportWorkflow::notifyDraftChanged() {
  emit stateChanged();
}

bool ImportWorkflow::renameCurrentStatementDraft(const QString& name) {
  auto* draft = statementDraft();
  if (!draft || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::StatementDraftEdit edit;
  edit.kind = core::ports::importing::draft::StatementDraftEditKind::Rename;
  edit.text = strings::toStdString(name);
  const auto result = importAdapter_->updateStatementDraft(*draft, edit);
  if (result.changed) {
    notifyDraftChanged();
  }
  return result.changed;
}

int ImportWorkflow::insertTransactionAfterCurrent() {
  auto* draft = statementDraft();
  if (!draft || !importAdapter_) {
    return -1;
  }
  core::ports::importing::draft::StatementDraftEdit edit;
  edit.kind = core::ports::importing::draft::StatementDraftEditKind::
      InsertTransactionAfter;
  edit.index = currentTransactionIndex();
  const int newIndex = importAdapter_->updateStatementDraft(*draft, edit)
                           .selectedTransactionIndex;
  if (newIndex >= 0) {
    setCurrentTransactionIndex(newIndex);
  }
  return newIndex;
}

int ImportWorkflow::removeCurrentTransaction() {
  auto* draft = statementDraft();
  if (!draft || !importAdapter_) {
    return -1;
  }
  core::ports::importing::draft::StatementDraftEdit edit;
  edit.kind = core::ports::importing::draft::StatementDraftEditKind::
      RemoveTransactionAt;
  edit.index = currentTransactionIndex();
  const int newIndex = importAdapter_->updateStatementDraft(*draft, edit)
                           .selectedTransactionIndex;
  if (newIndex >= 0) {
    setCurrentTransactionIndex(newIndex);
  }
  return newIndex;
}

bool ImportWorkflow::applyCurrentTransactionPatch(
    const core::ports::importing::draft::TransactionDraftPatch& patch) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind = core::ports::importing::draft::TransactionDraftEditKind::Patch;
  edit.patch = patch;
  if (!importAdapter_->updateTransactionDraft(*transaction, {}, edit)) {
    return false;
  }
  notifyDraftChanged();
  return true;
}

bool ImportWorkflow::commitCurrentTransactionName(const QString& name) {
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasName = true;
  patch.name = strings::toStdString(name);
  return applyCurrentTransactionPatch(patch);
}

bool ImportWorkflow::commitCurrentTransactionBookingDate(
    const QString& bookingDate) {
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasBookingDate = true;
  patch.bookingDate = strings::toStdString(bookingDate);
  return applyCurrentTransactionPatch(patch);
}

bool ImportWorkflow::commitCurrentTransactionValuta(const QString& valuta) {
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasValuta = true;
  patch.valuta = strings::toStdString(valuta);
  return applyCurrentTransactionPatch(patch);
}

bool ImportWorkflow::setCurrentTransactionStatus(int status) {
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasStatus = true;
  patch.status = status;
  return applyCurrentTransactionPatch(patch);
}

bool ImportWorkflow::setCurrentTransactionAllocatable(bool allocatable) {
  core::ports::importing::draft::TransactionDraftPatch patch;
  patch.hasAllocatable = true;
  patch.allocatable = allocatable;
  patch.allocatableSelected = true;
  return applyCurrentTransactionPatch(patch);
}

bool ImportWorkflow::applyCurrentTransactionAmountText(
    const QString& amountText) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind =
      core::ports::importing::draft::TransactionDraftEditKind::ApplyAmountText;
  edit.text = strings::toStdString(amountText);
  if (!importAdapter_->updateTransactionDraft(*transaction, {}, edit)) {
    return false;
  }
  notifyDraftChanged();
  return true;
}

bool ImportWorkflow::selectCurrentTransactionActor(const QString& actorId) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind =
      core::ports::importing::draft::TransactionDraftEditKind::SelectActor;
  edit.id = strings::toStdString(actorId);
  const bool changed =
      importAdapter_->updateTransactionDraft(*transaction, {}, edit);
  if (changed) {
    notifyDraftChanged();
  }
  return changed;
}

bool ImportWorkflow::clearCurrentTransactionActor() {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind =
      core::ports::importing::draft::TransactionDraftEditKind::ClearActor;
  const bool changed =
      importAdapter_->updateTransactionDraft(*transaction, {}, edit);
  if (changed) {
    notifyDraftChanged();
  }
  return changed;
}

bool ImportWorkflow::setCurrentTransactionPropertySelected(
    const QString& propertyId, bool selected) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind = core::ports::importing::draft::TransactionDraftEditKind::
      SetPropertySelected;
  edit.id = strings::toStdString(propertyId);
  edit.selected = selected;
  const bool changed =
      importAdapter_->updateTransactionDraft(*transaction, {}, edit);
  if (changed) {
    notifyDraftChanged();
  }
  return changed;
}

bool ImportWorkflow::selectCurrentTransactionContract(
    const QString& contractId) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind = core::ports::importing::draft::TransactionDraftEditKind::
      SelectContractFromCatalog;
  edit.id = strings::toStdString(contractId);
  const bool changed = importAdapter_->updateTransactionDraft(
      *transaction, catalogSnapshotForDraft(), edit);
  if (changed) {
    notifyDraftChanged();
  }
  return changed;
}

bool ImportWorkflow::clearCurrentTransactionContract() {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return false;
  }
  core::ports::importing::draft::TransactionDraftEdit edit;
  edit.kind =
      core::ports::importing::draft::TransactionDraftEditKind::ClearContract;
  const bool changed =
      importAdapter_->updateTransactionDraft(*transaction, {}, edit);
  if (changed) {
    notifyDraftChanged();
  }
  return changed;
}

bool ImportWorkflow::createActorForCurrentTransaction(
    const QString& actorName) {
  if (!commands_ || !selectors_) {
    return false;
  }
  const QString trimmedName = actorName.trimmed();
  if (trimmedName.isEmpty()) {
    return false;
  }
  QString actorId = selectors_->actorIdByName(trimmedName);
  if (actorId.isEmpty()) {
    actorId = commands_->saveActor({}, trimmedName);
  }
  return !actorId.isEmpty() && selectCurrentTransactionActor(actorId);
}

bool ImportWorkflow::createPropertyForCurrentTransaction(
    const QString& propertyName) {
  if (!commands_ || !selectors_) {
    return false;
  }
  const QString trimmedName = propertyName.trimmed();
  if (trimmedName.isEmpty()) {
    return false;
  }
  QString propertyId = selectors_->propertyIdByName(trimmedName);
  if (propertyId.isEmpty()) {
    propertyId = commands_->saveProperty({}, trimmedName);
  }
  return !propertyId.isEmpty() &&
         setCurrentTransactionPropertySelected(propertyId, true);
}

bool ImportWorkflow::createOrSelectContractForCurrentTransaction(
    const QString& contractName, const QString& contractType,
    const QString& allocatableMode) {
  auto* transaction = currentTransactionDraft();
  if (!transaction || !commands_ || !selectors_) {
    return false;
  }
  const QString trimmedType = contractType.trimmed();
  if (trimmedType.isEmpty()) {
    return false;
  }
  const QString effectiveName = contractName.trimmed().isEmpty()
                                    ? selectors_->nextContractName()
                                    : contractName.trimmed();
  QStringList actorIds;
  const QString actorId =
      QString::fromStdString(transaction->actorId).trimmed();
  if (!actorId.isEmpty()) {
    actorIds.push_back(actorId);
  }
  QStringList propertyIds;
  propertyIds.reserve(static_cast<int>(transaction->propertyIds.size()));
  for (const auto& propertyId : transaction->propertyIds) {
    propertyIds.push_back(QString::fromStdString(propertyId));
  }

  QString contractId = selectors_->contractIdBySignature(
      effectiveName, trimmedType, actorIds, propertyIds);
  if (contractId.isEmpty()) {
    QString mode = allocatableMode.trimmed().toLower();
    if (mode.isEmpty()) {
      mode = QStringLiteral("mixed");
    }
    contractId = commands_->saveContract({}, effectiveName, trimmedType,
                                         actorIds, propertyIds, {}, mode);
  }
  return !contractId.isEmpty() && selectCurrentTransactionContract(contractId);
}

TransactionDraftView ImportWorkflow::currentTransactionView() const {
  const auto* transaction = currentTransactionDraft();
  if (!transaction || !importAdapter_) {
    return {};
  }
  try {
    return importAdapter_->transactionDraftView(*transaction,
                                                catalogSnapshotForDraft());
  } catch (...) {
    reportException(observability::origins::workflow::import::kFinalize,
                    std::current_exception());
    return {};
  }
}

core::ports::workspace::StatementDraftSnapshot
ImportWorkflow::currentStatementDraftSnapshot() const {
  const auto* draft = statementDraft();
  if (!draft || !importAdapter_) {
    return {};
  }
  return importAdapter_->buildStatementDraftSnapshot(*draft,
                                                     catalogSnapshotForDraft());
}

QString ImportWorkflow::finalizeCurrentStatementDraft() {
  if (!commands_ || !importAdapter_ || !state_->hasDraft()) {
    return {};
  }
  flushActiveDraftToWorkspace();
  const auto snapshot = currentStatementDraftSnapshot();
  if (snapshot.id.empty() || snapshot.transactions.empty()) {
    return {};
  }
  return commands_->finalizeStatementDraft(snapshot);
}

void ImportWorkflow::flushActiveDraftToWorkspace() {
  if (!commands_ || !importAdapter_ || !state_->hasDraft()) {
    return;
  }

  const auto* draft = state_->draft();
  if (!draft || draft->id.empty()) {
    return;
  }

  const auto snapshot = importAdapter_->buildStatementDraftSnapshot(
      *draft, catalogSnapshotForDraft());
  if (snapshot.id.empty()) {
    return;
  }

  commands_->saveStatementDraft(snapshot);
}

core::ports::workspace::WorkspaceSnapshot
ImportWorkflow::catalogSnapshotForDraft() const {
  const auto liveSnapshot = stateSnapshot();
  if (!state_->hasDraft() || !importAdapter_) {
    return liveSnapshot;
  }

  return importAdapter_->mergeWorkspaceState(state_->catalogSnapshot(),
                                             liveSnapshot);
}

} // namespace ui
