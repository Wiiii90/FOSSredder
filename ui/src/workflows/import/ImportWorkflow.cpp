/**
 * @file ui/src/workflows/import/ImportWorkflow.cpp
 * @brief Implements asynchronous import runs and draft session restoration.
 */

#include "ui/workflows/import/ImportWorkflow.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <QMetaObject>
#include <QUuid>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/shared/config/Defaults.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/observability/Trace.h"
#include "ui/shared/text/Text.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui::importing {

void ImportWorkflowState::clearDraftState() {
  draft_ = {};
  catalogSnapshot_ = {};
  hasDraft_ = false;
  currentTransactionIndex_ = 0;
}

bool ImportWorkflowState::setSelectedFile(const QString &path) {
  if (selectedFile_ == path) {
    return false;
  }
  selectedFile_ = path;
  return true;
}

bool ImportWorkflowState::addFiles(const QStringList &paths) {
  QStringList cleaned;
  cleaned.reserve(paths.size());
  for (const auto &path : paths) {
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
  phase_ = ui::text::importPhases::finished();
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

bool ImportWorkflowState::takeNextQueuedFile(QString &nextFile) {
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

void ImportWorkflowState::beginImport(const QString &path) {
  selectedFile_ = path;
  currentImportFile_ = path;

  artifactCount_ = 0;
  error_.clear();
  phase_ = ui::text::importPhases::starting();
  progress_ = ui::config::importProgress::kInitial;
  isRunning_ = true;
  resetCancellationState();
}

void ImportWorkflowState::rejectStart(const QString &errorMessage) {
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
  phase_ = ui::text::importPhases::stopping();
}

bool ImportWorkflowState::setPaused(bool paused) {
  if (!isRunning_ || canceled_) {
    return false;
  }
  if (paused_ == paused) {
    return false;
  }
  paused_ = paused;
  phase_ = paused_ ? QStringLiteral("Paused")
                   : QStringLiteral("Running import...");
  return true;
}

void ImportWorkflowState::recordCanceled() {
  error_.clear();
  if (cancelClearsQueue_) {
    queuedFiles_.clear();
  }
  phase_ = ui::text::importPhases::canceled();
  clearTransientImportState();
  resetCancellationState();
  selectedFile_.clear();
  currentImportFile_.clear();
}

void ImportWorkflowState::recordFailed(const QString &errorMessage) {
  error_ = errorMessage;
  queuedFiles_.clear();
  phase_ = ui::text::importPhases::failed();
  clearTransientImportState();
  resetCancellationState();
  currentImportFile_.clear();
}

bool ImportWorkflowState::populateDraft(
    const core::ports::importing::draft::StatementDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const std::map<std::string, std::vector<uint8_t>> &artifacts,
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
    const core::ports::importing::draft::StatementDraft &draft,
    const core::ports::workspace::WorkspaceSnapshot &state,
    const QString &draftId, int currentTransactionIndex) {
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

void ImportWorkflowState::updateProgress(double progress, const QString &phase) {
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
  if (progress < config::importProgress::kMinimum) {
    return config::importProgress::kMinimum;
  }
  if (progress > config::importProgress::kMaximum) {
    return config::importProgress::kMaximum;
  }
  return progress;
}

} // namespace

ImportWorkflow::ImportWorkflow(
    std::shared_ptr<ui::adapters::ImportAdapter> importAdapter,
    std::shared_ptr<core::errors::IErrorReporter> errorReporter,
    WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent),
      importAdapter_(std::move(importAdapter)),
      workspace_(workspace),
      errorReporter_(std::move(errorReporter)) {
  if (!errorReporter_) {
    throw std::invalid_argument("ImportWorkflow requires an error reporter");
  }
}

void ImportWorkflow::setWorkspace(WorkspaceFacade *workspace) {
  workspace_ = workspace;
  emit stateChanged();
}

QString ImportWorkflow::selectedFile() const { return state_.selectedFile(); }

QStringList ImportWorkflow::queuedFiles() const { return state_.queuedFiles(); }

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

void ImportWorkflow::saveRunLog(const QString &logId, const QString &status,
                                const QString &message, bool draftAttached,
                                const QString &draftId,
                                const QString &statementId) {
  if (!workspace_) {
    return;
  }
  workspace_->upsertImportLog(logId, status, message, draftAttached, draftId,
                              statementId, state_.currentImportFile());
}

void ImportWorkflow::addFiles(const QStringList &paths) {
  if (state_.addFiles(paths)) {
    emit stateChanged();
  }
}

void ImportWorkflow::setSelectedFile(const QString &path) {
  if (state_.setSelectedFile(path)) {
    emit stateChanged();
  }
}

void ImportWorkflow::resetStatus() {
  if (state_.resetStatus()) {
    emit stateChanged();
  }
}

void ImportWorkflow::clearDraft() {
  rememberCurrentDraftTransactionIndex();
  const bool shouldStartNext = state_.clearDraft();
  activeDraftId_.clear();
  emit stateChanged();
  if (shouldStartNext) {
    startNextQueuedImport();
  }
}

void ImportWorkflow::rejectImportStart(const QString &errorMessage,
                                       const char *traceMessage) {
  state_.rejectStart(errorMessage);
  observability::reportFlow(core::errors::ErrorSeverity::Warning,
                            observability::codes::FlowImportRejected,
                            observability::origins::workflow::import::kStart,
                            traceMessage);
  emit stateChanged();
  emit importFailed(state_.error());
}

void ImportWorkflow::startNextQueuedImport() {
  QString next;
  if (!state_.takeNextQueuedFile(next)) {
    return;
  }
  emit stateChanged();
  startImportForFile(next);
}

void ImportWorkflow::startStatementImport() {
  if (state_.isRunning() || state_.hasDraft()) {
    return;
  }

  const auto selected = state_.takeSelectedFileForStart();
  if (!selected.isEmpty()) {
    emit stateChanged();
    startImportForFile(selected);
    return;
  }

  startNextQueuedImport();
}

void ImportWorkflow::startImportForFile(const QString &path) {
  if (state_.isRunning()) {
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

  state_.beginImport(path);
  activeImportLogId_ = newLogId();
  activeImportTerminalHandled_ = false;
  hasPendingTerminalEvent_ = false;
  activeImportHandle_ = {};
  hasActiveImportHandle_ = false;
  pendingTerminalState_ =
      core::ports::importing::StatementImportState::Pending;
  pendingTerminalMessage_.clear();
  saveRunLog(activeImportLogId_, text::importLogs::statusRunning(),
             text::importPhases::starting());
  emit stateChanged();

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportStarted,
      observability::origins::workflow::import::kStart, "Import started",
      {{observability::context::kFile, strings::toStdString(path)},
       {observability::context::kQueuedCount,
        std::to_string(state_.queuedFiles().size())}});

  core::ports::importing::StatementImportStartRequest request;
  request.sourcePath = strings::toEncodedPath(path);
  activeImportHandle_ = importAdapter_->startStatementImport(
      request, [this](const core::ports::importing::StatementImportEvent &event) {
        handleImportEvent(event);
      });
  hasActiveImportHandle_ = !activeImportHandle_.importId.empty();

  if (!hasActiveImportHandle_) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: unable to start job");
  }
}

void ImportWorkflow::updateProgress(double progress, const QString &phase) {
  state_.updateProgress(progress, phase);
  emit stateChanged();
}

void ImportWorkflow::reportException(const char *origin,
                                     std::exception_ptr exception) const {
  if (!errorReporter_) {
    return;
  }
  errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin,
                                  exception);
}

void ImportWorkflow::requestImportCancellation(bool clearQueue,
                                               const char *origin,
                                               const char *traceMessage) {
  if (!state_.isRunning()) {
    return;
  }

  const auto queuedBeforeCancel =
      clearQueue ? state_.queuedFiles() : QStringList{};
  state_.beginCancel(clearQueue);

  if (clearQueue && workspace_) {
    for (const auto &queuedPath : queuedBeforeCancel) {
      workspace_->upsertImportLog(
          newLogId(), text::importLogs::statusCanceled(),
          QStringLiteral("Import canceled before start."), false, {}, {},
          queuedPath);
    }
  }

  if (importAdapter_ && hasActiveImportHandle()) {
    importAdapter_->cancel(activeImportHandle_);
  }
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled, origin, traceMessage,
      {{observability::context::kFile,
        strings::toStdString(state_.currentImportFile())},
       {observability::context::kQueuedCount,
        std::to_string(state_.queuedFiles().size())}});
  emit stateChanged();
}

void ImportWorkflow::cancelImport() {
  requestImportCancellation(false,
                            observability::origins::workflow::import::kCancel,
                            "Import cancellation requested");
}

void ImportWorkflow::cancelQueuedImports() {
  requestImportCancellation(
      true, observability::origins::workflow::import::kCancelAll,
      "Cancel-all requested for import queue");
}

void ImportWorkflow::setImportPaused(bool paused) {
  if (state_.isPaused() == paused) {
    return;
  }
  if (!state_.setPaused(paused)) {
    return;
  }
  if (importAdapter_ && hasActiveImportHandle()) {
    if (state_.isPaused()) {
      importAdapter_->pause(activeImportHandle_);
    } else {
      importAdapter_->resume(activeImportHandle_);
    }
  }
  if (!activeImportLogId_.isEmpty()) {
    saveRunLog(activeImportLogId_,
               state_.isPaused() ? text::importLogs::statusPaused()
                                 : text::importLogs::statusRunning(),
               state_.isPaused() ? QStringLiteral("Import paused.")
                                 : QStringLiteral("Import resumed."));
  }
  emit stateChanged();

  if (!state_.isPaused() && hasPendingTerminalEvent_) {
    const auto terminalState = pendingTerminalState_;
    const auto message = pendingTerminalMessage_;
    hasPendingTerminalEvent_ = false;
    pendingTerminalState_ =
        core::ports::importing::StatementImportState::Pending;
    pendingTerminalMessage_.clear();
    onJobTerminal(terminalState, message);
  }
}

void ImportWorkflow::pauseImport() { setImportPaused(true); }

void ImportWorkflow::resumeImport() { setImportPaused(false); }

void ImportWorkflow::handleImportEvent(
    const core::ports::importing::StatementImportEvent &event) {
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
  state_.recordCanceled();
  if (!activeImportTerminalHandled_) {
    saveRunLog(activeImportLogId_, text::importLogs::statusCanceled(),
               text::importPhases::canceled());
    activeImportTerminalHandled_ = true;
  }
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled,
      observability::origins::workflow::import::kTerminal, "Import canceled",
      {{observability::context::kStatus,
        strings::toStdString(text::importLogs::statusCanceled())}});
  emit stateChanged();
  emit importCanceled();
}

void ImportWorkflow::handleImportFailed(const QString &errorMessage,
                                        const char *traceMessage) {
  const QString traceDetail =
      traceMessage ? QString::fromUtf8(traceMessage) : QString();
  const QString visibleError =
      errorMessage == text::workflowErrors::importFailed() &&
              traceDetail.startsWith(QStringLiteral("Import failed:"))
          ? traceDetail
          : errorMessage;
  state_.recordFailed(visibleError);
  if (!activeImportTerminalHandled_) {
    saveRunLog(activeImportLogId_, text::importLogs::statusFailed(),
               errorMessage);
    activeImportTerminalHandled_ = true;
  }
  observability::reportFlow(
      core::errors::ErrorSeverity::Warning,
      observability::codes::FlowImportFailed,
      observability::origins::workflow::import::kTerminal, traceMessage,
      {{observability::context::kError, strings::toStdString(state_.error())}});
  emit stateChanged();
  emit importFailed(state_.error());
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

  const auto snapshot =
      workspace_ ? workspace_->workspaceSnapshot()
                 : core::ports::workspace::WorkspaceSnapshot{};
  const QString draftId = activeImportLogId_;
  const bool hadVisibleDraft = state_.hasDraft();

  const auto draftSnapshot = importAdapter_->buildImportedStatementDraftSnapshot(
      strings::toStdString(state_.currentImportFile()),
      strings::toStdString(draftId), imported.statement, imported.transactions);
  if (!workspace_ || draftSnapshot.id.empty()) {
    handleImportFailed(text::workflowErrors::importFailed(),
                       "Import failed: unable to create draft state");
    return false;
  }
  workspace_->saveStatementDraft(draftSnapshot);

  if (hadVisibleDraft) {
    state_.recordFinished();
  } else {
    const auto draft = importAdapter_->restoreStatementDraft(draftSnapshot);
    if (!state_.populateDraft(draft, snapshot, imported.artifacts, 0)) {
      handleImportFailed(text::workflowErrors::importFailed(),
                         "Import failed: unable to create statement draft");
      return false;
    }
  }

  saveRunLog(activeImportLogId_, text::importLogs::statusDraft(),
             QStringLiteral("Draft ready for manual review."), true, draftId);
  if (!hadVisibleDraft) {
    activeDraftId_ = draftId;
  }
  activeImportTerminalHandled_ = true;

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportFinished,
      observability::origins::workflow::import::kTerminal, "Import finished",
      {{observability::context::kStatus,
        strings::toStdString(text::importLogs::statusSuccess())},
       {observability::context::kArtifactCount,
        std::to_string(state_.artifactCount())}});
  emit stateChanged();
  emit importFinished();
  return true;
}

void ImportWorkflow::onJobTerminal(
    core::ports::importing::StatementImportState state,
    const QString &message) {
  if ((!state_.isRunning() && !state_.cancelRequested()) ||
      activeImportTerminalHandled_) {
    return;
  }

  if (state_.isPaused() &&
      state != core::ports::importing::StatementImportState::Canceled) {
    hasPendingTerminalEvent_ = true;
    pendingTerminalState_ = state;
    pendingTerminalMessage_ = message;
    return;
  }

  if (state == core::ports::importing::StatementImportState::Canceled ||
      state_.cancelRequested()) {
    clearActiveImportSubscription();
    handleImportCanceled();
    return;
  }

  if (state == core::ports::importing::StatementImportState::Failed) {
    clearActiveImportSubscription();
    handleImportFailed(
        message.isEmpty() ? text::workflowErrors::importFailed() : message,
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
                                         state_.currentTransactionIndex());
}

int ImportWorkflow::rememberedDraftTransactionIndex(
    const QString &draftId) const {
  return draftTransactionIndexByDraftId_.value(draftId, 0);
}

bool ImportWorkflow::openPersistedDraft(const QString &draftId) {
  if (!workspace_) {
    return false;
  }
  rememberCurrentDraftTransactionIndex();

  QString requestedDraftId = !draftId.isEmpty() ? draftId : activeDraftId_;
  if (requestedDraftId.isEmpty()) {
    for (const auto &id : workspace_->attachedImportDraftIds()) {
      requestedDraftId = id;
      break;
    }
  }
  if (!requestedDraftId.isEmpty()) {
    activeDraftId_ = requestedDraftId;
  }

  const bool restored =
      restoreDraftFromState(workspace_->workspaceSnapshot());
  if (restored) {
    emit stateChanged();
  }
  return restored;
}

bool ImportWorkflow::restoreDraftFromState(
    const core::ports::workspace::WorkspaceSnapshot &snapshot) {
  if (snapshot.statementDrafts.empty() || !importAdapter_) {
    return false;
  }

  const core::ports::workspace::StatementDraftSnapshot *persisted = nullptr;
  const QString requestedDraftId = resolveDraftContextId();
  if (!requestedDraftId.isEmpty()) {
    for (const auto &draft : snapshot.statementDrafts) {
      if (QString::fromStdString(draft.id) == requestedDraftId) {
        persisted = &draft;
        break;
      }
    }
  }
  if (!persisted) {
    persisted = &snapshot.statementDrafts.front();
  }
  if (!persisted) {
    return false;
  }

  const QString restoredDraftId = QString::fromStdString(persisted->id);
  if (!restoredDraftId.isEmpty()) {
    activeDraftId_ = restoredDraftId;
  }

  const auto draft = importAdapter_->restoreStatementDraft(*persisted);
  return state_.restoreDraft(draft, snapshot, restoredDraftId,
                             rememberedDraftTransactionIndex(restoredDraftId));
}

core::ports::importing::draft::TransactionDraft *
ImportWorkflow::currentTransactionDraft() {
  auto *draft = state_.draft();
  if (!draft) {
    return nullptr;
  }
  const int index = state_.currentTransactionIndex();
  if (index < 0 ||
      static_cast<std::size_t>(index) >= draft->transactions.size()) {
    return nullptr;
  }
  return &draft->transactions[static_cast<std::size_t>(index)];
}

const core::ports::importing::draft::TransactionDraft *
ImportWorkflow::currentTransactionDraft() const {
  return const_cast<ImportWorkflow *>(this)->currentTransactionDraft();
}

int ImportWorkflow::transactionCount() const noexcept {
  const auto *draft = state_.draft();
  return draft ? static_cast<int>(draft->transactions.size()) : 0;
}

QString ImportWorkflow::currentDraftId() const {
  const auto *draft = state_.draft();
  return draft ? QString::fromStdString(draft->id) : QString();
}

void ImportWorkflow::setCurrentTransactionIndex(int index) {
  if (!state_.hasDraft()) {
    return;
  }
  const int previous = state_.currentTransactionIndex();
  state_.setCurrentTransactionIndex(index);
  if (state_.currentTransactionIndex() != previous) {
    emit stateChanged();
  }
}

void ImportWorkflow::notifyDraftChanged() { emit stateChanged(); }

core::ports::workspace::WorkspaceSnapshot
ImportWorkflow::catalogSnapshotForDraft() const {
  const auto liveSnapshot =
      workspace_ ? workspace_->workspaceSnapshot()
                 : core::ports::workspace::WorkspaceSnapshot{};
  if (!state_.hasDraft() || !importAdapter_) {
    return liveSnapshot;
  }

  return importAdapter_->mergeWorkspaceState(state_.catalogSnapshot(),
                                             liveSnapshot);
}

} // namespace ui
