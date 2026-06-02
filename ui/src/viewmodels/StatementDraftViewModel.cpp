/**
 * @file ui/src/viewmodels/StatementDraftViewModel.cpp
 * @brief Implements statement draft review state.
 */

#include "ui/viewmodels/StatementDraftViewModel.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

StatementDraftViewModel::StatementDraftViewModel(QObject *parent)
    : QObject(parent) {}

void StatementDraftViewModel::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (importWorkflow_) {
    connect(importWorkflow_, &ImportWorkflow::stateChanged, this,
            &StatementDraftViewModel::changed);
  }
  emit changed();
}

void StatementDraftViewModel::setWorkspace(WorkspaceFacade *value) {
  workspace_ = value;
}

bool StatementDraftViewModel::hasDraft() const noexcept {
  return importWorkflow_ && importWorkflow_->hasDraft();
}

QString StatementDraftViewModel::statementName() const {
  const auto *draft = importWorkflow_ ? importWorkflow_->statementDraft()
                                      : nullptr;
  return draft ? QString::fromStdString(draft->name) : QString();
}

void StatementDraftViewModel::setStatementName(const QString &value) {
  if (!importWorkflow_ || statementName() == value) {
    return;
  }
  auto *draft = importWorkflow_->statementDraft();
  const auto adapter = importWorkflow_->importAdapter();
  if (!draft || !adapter) {
    return;
  }
  if (adapter->renameStatementDraft(*draft, strings::toStdString(value))) {
    importWorkflow_->notifyDraftChanged();
    emit changed();
  }
}

int StatementDraftViewModel::currentTransactionNumber() const noexcept {
  if (!hasDraft()) {
    return 0;
  }
  return importWorkflow_->currentTransactionIndex() + 1;
}

int StatementDraftViewModel::transactionCount() const noexcept {
  return importWorkflow_ ? importWorkflow_->transactionCount() : 0;
}

bool StatementDraftViewModel::canDeleteTransaction() const noexcept {
  return importWorkflow_ && importWorkflow_->transactionCount() > 1;
}

bool StatementDraftViewModel::canSelectPreviousTransactionDraft()
    const noexcept {
  return importWorkflow_ && importWorkflow_->currentTransactionIndex() > 0;
}

bool StatementDraftViewModel::canSelectNextTransactionDraft() const noexcept {
  return importWorkflow_ &&
         importWorkflow_->currentTransactionIndex() <
             importWorkflow_->transactionCount() - 1;
}

void StatementDraftViewModel::returnToImport() {
  if (!importWorkflow_ || !workspace_) {
    return;
  }
  const QString draftId = importWorkflow_->currentDraftId();
  workspace_->upsertImportLog(
      draftId, tr("Draft"), tr("Draft paused. Click log entry to continue."),
      true, draftId);
  importWorkflow_->clearDraft();
  emit changed();
}

void StatementDraftViewModel::discard() {
  if (!importWorkflow_ || !workspace_) {
    return;
  }
  const QString draftId = importWorkflow_->currentDraftId();
  const char *origin = observability::origins::workflow::import::kFinalize;
  try {
    workspace_->clearStatementDraft(draftId);
  } catch (...) {
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError, origin,
                                  std::current_exception());
  }
  workspace_->upsertImportLog(draftId, tr("Draft discarded"),
                              tr("Statement draft was discarded."), false,
                              {}, {}, importWorkflow_->selectedFile());
  importWorkflow_->clearDraft();
  emit changed();
}

void StatementDraftViewModel::addTransactionAfterCurrent() {
  if (!hasDraft() || !importWorkflow_) {
    return;
  }
  auto *draft = importWorkflow_->statementDraft();
  const auto adapter = importWorkflow_->importAdapter();
  if (!draft || !adapter) {
    return;
  }
  const int newIndex = adapter->insertTransactionAfter(
      *draft, importWorkflow_->currentTransactionIndex());
  if (newIndex < 0) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(newIndex);
  emit changed();
}

void StatementDraftViewModel::deleteCurrentTransaction() {
  if (!canDeleteTransaction() || !importWorkflow_) {
    return;
  }
  auto *draft = importWorkflow_->statementDraft();
  const auto adapter = importWorkflow_->importAdapter();
  if (!draft || !adapter) {
    return;
  }
  const int newIndex = adapter->removeTransactionAt(
      *draft, importWorkflow_->currentTransactionIndex());
  if (newIndex < 0) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(newIndex);
  emit changed();
}

void StatementDraftViewModel::finalize() {
  if (!hasDraft() || !importWorkflow_ || !workspace_) {
    return;
  }

  const QString draftId = importWorkflow_->currentDraftId();
  const char *origin = observability::origins::workflow::import::kFinalize;
  QString statementId;
  try {
    const auto *draft = importWorkflow_->statementDraft();
    const auto adapter = importWorkflow_->importAdapter();
    if (!draft || !adapter) {
      return;
    }
    const auto input = adapter->buildStatementDraftSnapshot(
        *draft, importWorkflow_->catalogSnapshotForDraft());
    if (!input.transactions.empty()) {
      statementId = workspace_->finalizeStatementDraft(input);
    }
  } catch (...) {
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError, origin,
                                  std::current_exception());
  }

  if (statementId.isEmpty()) {
    workspace_->upsertImportLog(draftId, tr("Finalize failed"),
                                tr("Draft could not be finalized."), true,
                                draftId);
    emit changed();
    return;
  }

  try {
    workspace_->clearStatementDraft(draftId);
  } catch (...) {
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError, origin,
                                  std::current_exception());
  }
  workspace_->upsertImportLog(draftId, tr("Finalized"),
                              tr("Draft was finalized into a statement."),
                              false, {}, statementId);
  importWorkflow_->clearDraft();
  emit changed();
}

void StatementDraftViewModel::selectPreviousTransactionDraft() {
  if (!canSelectPreviousTransactionDraft()) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(
      importWorkflow_->currentTransactionIndex() - 1);
}

void StatementDraftViewModel::selectNextTransactionDraft() {
  if (!canSelectNextTransactionDraft()) {
    return;
  }
  importWorkflow_->setCurrentTransactionIndex(
      importWorkflow_->currentTransactionIndex() + 1);
}

} // namespace ui
