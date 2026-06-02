/**
 * @file ui/src/workspace/WorkspaceFacade.cpp
 * @brief Implements the core wiring of the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>
#include <cstddef>
#include <exception>

#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {

namespace {

QString displayFileName(const QString &path) {
  const QFileInfo info(path);
  const QString name = info.fileName();
  return name.isEmpty() ? path : name;
}

} // namespace

WorkspaceFacade::WorkspaceFacade(QObject *parent)
    : QObject(parent), cache_(std::make_unique<WorkspaceCache>(this)),
      selection_(std::make_unique<WorkspaceSelection>(cache_->models(), this)) {
  connect(selection_.get(), &WorkspaceSelection::selectedActorIdChanged, this,
          &WorkspaceFacade::selectedActorIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedPropertyIdChanged, this,
          &WorkspaceFacade::selectedPropertyIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedContractIdChanged, this,
          &WorkspaceFacade::selectedContractIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedStatementIdChanged, this,
          &WorkspaceFacade::selectedStatementIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedTransactionIdChanged,
          this, &WorkspaceFacade::selectedTransactionIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedAnalysisIdChanged, this,
          &WorkspaceFacade::selectedAnalysisIdChanged);
  connect(selection_.get(), &WorkspaceSelection::selectedAnnualIdChanged, this,
          &WorkspaceFacade::selectedAnnualIdChanged);
  connect(selection_.get(), &WorkspaceSelection::lastAnalysisResultChanged, this,
          &WorkspaceFacade::lastAnalysisResultChanged);
}

WorkspaceFacade::WorkspaceFacade(
    core::ports::workspace::IWorkspaceWriter *workspaceWriter,
    core::ports::workspace::IWorkspaceReader *workspaceReader, QObject *parent)
    : WorkspaceFacade(parent) {
  setWorkspacePorts(workspaceWriter, workspaceReader);
}

WorkspaceCache *WorkspaceFacade::cache() noexcept { return cache_.get(); }

WorkspaceSelection *WorkspaceFacade::selection() noexcept {
  return selection_.get();
}

void WorkspaceFacade::bumpDataRevision() {
  ++dataRevision_;
  emit dataRevisionChanged();
}

void WorkspaceFacade::runStorageOperation(
    const QString &operation, const std::function<void()> &action) {
  if (!workspaceWriter_) {
    return;
  }
  try {
    action();
    emit operationSucceeded(operation);
  } catch (const std::exception &ex) {
    emit operationFailed(operation, QString::fromUtf8(ex.what()));
  }
}

void WorkspaceFacade::setWorkspacePorts(
    core::ports::workspace::IWorkspaceWriter *workspaceWriter,
    core::ports::workspace::IWorkspaceReader *workspaceReader) {
  workspaceWriter_ = workspaceWriter;
  workspaceReader_ = workspaceReader;
  if (workspaceReader_) {
    loadFromState(workspaceReader_->workspaceSnapshot());
  }
}

void WorkspaceFacade::loadFromState(
    const core::ports::workspace::WorkspaceSnapshot &state) {
  cache_->loadFromState(state);
  selection_->loadFromState();
  bumpDataRevision();
}

QString WorkspaceFacade::currentPath() const {
  return workspaceReader_
             ? QString::fromStdString(workspaceReader_->currentPath())
             : QString();
}

core::ports::workspace::WorkspaceSnapshot WorkspaceFacade::workspaceSnapshot()
    const {
  return workspaceReader_ ? workspaceReader_->workspaceSnapshot()
                          : core::ports::workspace::WorkspaceSnapshot{};
}

std::optional<core::ports::workspace::StatementDraftSnapshot>
WorkspaceFacade::statementDraftSnapshot(const QString &draftId) const {
  return workspaceReader_
             ? workspaceReader_->statementDraftSnapshot(
                   strings::toStdString(draftId))
             : std::nullopt;
}

QString WorkspaceFacade::finalizeStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot &draft) {
  if (!workspaceWriter_) {
    return {};
  }
  core::ports::workspace::FinalizeStatementDraftCommand command;
  command.draft = draft;
  const QString id =
      QString::fromStdString(workspaceWriter_->finalizeStatementDraft(command));
  if (workspaceReader_) {
    loadFromState(workspaceReader_->workspaceSnapshot());
  }
  return id;
}

void WorkspaceFacade::saveStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot &draft) {
  if (!workspaceWriter_) {
    return;
  }
  core::ports::workspace::StatementDraftCommand command;
  command.draft = draft;
  workspaceWriter_->saveStatementDraft(command);
  if (workspaceReader_) {
    loadFromState(workspaceReader_->workspaceSnapshot());
  }
}

void WorkspaceFacade::clearStatementDraft(const QString &draftId) {
  if (!workspaceWriter_) {
    return;
  }
  workspaceWriter_->clearStatementDraft(strings::toStdString(draftId));
  if (workspaceReader_) {
    loadFromState(workspaceReader_->workspaceSnapshot());
  }
}

QString WorkspaceFacade::selectedActorId() const {
  return selection_ ? selection_->selectedActorId() : QString();
}

QString WorkspaceFacade::selectedPropertyId() const {
  return selection_ ? selection_->selectedPropertyId() : QString();
}

QString WorkspaceFacade::selectedContractId() const {
  return selection_ ? selection_->selectedContractId() : QString();
}

QString WorkspaceFacade::selectedStatementId() const {
  return selection_ ? selection_->selectedStatementId() : QString();
}

QString WorkspaceFacade::selectedTransactionId() const {
  return selection_ ? selection_->selectedTransactionId() : QString();
}

QString WorkspaceFacade::selectedAnalysisId() const {
  return selection_ ? selection_->selectedAnalysisId() : QString();
}

QString WorkspaceFacade::selectedAnnualId() const {
  return selection_ ? selection_->selectedAnnualId() : QString();
}

QVariant WorkspaceFacade::lastAnalysisResult() const {
  return selection_ ? selection_->lastAnalysisResult() : QVariant();
}

void WorkspaceFacade::selectActor(const QString &id) {
  if (selection_) {
    selection_->setSelectedActorId(id.trimmed());
  }
}

void WorkspaceFacade::selectProperty(const QString &id) {
  if (selection_) {
    selection_->setSelectedPropertyId(id.trimmed());
  }
}

void WorkspaceFacade::selectContract(const QString &id) {
  if (selection_) {
    selection_->setSelectedContractId(id.trimmed());
  }
}

void WorkspaceFacade::selectStatement(const QString &id) {
  if (selection_) {
    selection_->setSelectedStatementId(id.trimmed());
  }
}

void WorkspaceFacade::selectTransaction(const QString &statementId,
                                        const QString &id) {
  if (!selection_) {
    return;
  }
  selection_->setSelectedStatementId(statementId.trimmed());
  selection_->setSelectedTransactionId(id.trimmed());
}

void WorkspaceFacade::selectAnalysis(const QString &id) {
  if (selection_) {
    selection_->setSelectedAnalysisId(id.trimmed());
  }
}

void WorkspaceFacade::selectAnnual(const QString &id) {
  if (selection_) {
    selection_->setSelectedAnnualId(id.trimmed());
  }
}

void WorkspaceFacade::setLastAnalysisResult(const QVariant &value) {
  if (selection_) {
    selection_->setLastAnalysisResult(value);
  }
}

void WorkspaceFacade::saveImportLog(
    const core::ports::workspace::ImportLogSnapshot &log) {
  if (!workspaceWriter_) {
    return;
  }
  core::ports::workspace::ImportLogCommand command;
  command.log = log;
  workspaceWriter_->saveImportLog(command);
}

void WorkspaceFacade::upsertImportLog(const QString &logId,
                                      const QString &status,
                                      const QString &message,
                                      bool draftAttached,
                                      const QString &draftId,
                                      const QString &statementId,
                                      const QString &importFile) {
  if (!workspaceWriter_) {
    return;
  }

  core::ports::workspace::ImportLogSnapshot log;
  const auto snapshot = workspaceSnapshot();
  const QString resolvedLogId =
      logId.isEmpty()
          ? QUuid::createUuid().toString(QUuid::WithoutBraces)
          : logId;
  for (const auto &existing : snapshot.importLogs) {
    if (QString::fromStdString(existing.id) == resolvedLogId) {
      log = existing;
      break;
    }
  }

  log.id = resolvedLogId.toStdString();
  log.time = QDateTime::currentDateTime()
                 .toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))
                 .toStdString();
  log.status = status.toStdString();
  log.message = message.toStdString();
  log.draftAttached = draftAttached;
  log.statementId = statementId.toStdString();

  if (log.type.empty()) {
    log.type = "statement";
  }
  if (!importFile.isEmpty()) {
    log.file = importFile.toStdString();
  }

  const QString resolvedDraftId =
      draftAttached ? (draftId.isEmpty() ? resolvedLogId : draftId) : QString();
  log.draftId = resolvedDraftId.toStdString();
  log.statementDraftIds.clear();
  if (draftAttached && !resolvedDraftId.isEmpty()) {
    log.statementDraftIds.push_back(resolvedDraftId.toStdString());
  }

  saveImportLog(log);
}

QStringList WorkspaceFacade::attachedImportDraftIds() const {
  QStringList ids;
  const auto snapshot = workspaceSnapshot();
  ids.reserve(static_cast<int>(snapshot.importLogs.size()));
  for (const auto &entry : snapshot.importLogs) {
    if (!entry.draftAttached) {
      continue;
    }
    const QString draftId = QString::fromStdString(entry.draftId);
    const QString logId = QString::fromStdString(entry.id);
    const QString id = !draftId.isEmpty() ? draftId : logId;
    if (id.isEmpty() || ids.contains(id)) {
      continue;
    }
    ids.push_back(id);
  }
  return ids;
}

void WorkspaceFacade::deleteImportLog(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteImportLog(strings::toStdString(id));
  }
}

void WorkspaceFacade::clearImportLogs() {
  if (workspaceWriter_) {
    workspaceWriter_->clearImportLogs();
  }
}

QVariantList WorkspaceFacade::importLogRows() const {
  QVariantList rows;
  if (!workspaceReader_) {
    return rows;
  }

  const auto snapshot = workspaceReader_->workspaceSnapshot();
  rows.reserve(static_cast<int>(snapshot.importLogs.size()));
  for (const auto &log : snapshot.importLogs) {
    const QString file = QString::fromStdString(log.file);
    const QString message = QString::fromStdString(log.message);
    QVariantMap row;
    row[payload::keys::importRun::kLogId] = QString::fromStdString(log.id);
    row[payload::keys::importRun::kTime] = QString::fromStdString(log.time);
    row[payload::keys::common::kType] = QString::fromStdString(log.type);
    row[payload::keys::importRun::kFile] = file;
    row[payload::keys::common::kStatus] = QString::fromStdString(log.status);
    row[payload::keys::importRun::kMessage] = message;
    row[payload::keys::importRun::kDraftAttached] = log.draftAttached;
    row[payload::keys::importRun::kDraftId] =
        QString::fromStdString(log.draftId);
    row[payload::keys::importRun::kStatementId] =
        QString::fromStdString(log.statementId);
    row[payload::keys::common::kDisplayTime] = QString::fromStdString(log.time);
    row[payload::keys::common::kDisplayTitle] =
        displayFileName(file);
    row[payload::keys::common::kDisplayStatusDetail] = message;
    rows.push_back(row);
  }
  return rows;
}

void WorkspaceFacade::saveExportLog(
    const core::ports::workspace::ExportLogSnapshot &log) {
  if (!workspaceWriter_) {
    return;
  }
  core::ports::workspace::ExportLogCommand command;
  command.log = log;
  workspaceWriter_->saveExportLog(command);
}

void WorkspaceFacade::deleteExportLog(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteExportLog(strings::toStdString(id));
  }
}

void WorkspaceFacade::deleteExportLogAt(int index) {
  if (!workspaceWriter_ || !workspaceReader_ || index < 0) {
    return;
  }
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  if (index >= static_cast<int>(snapshot.exportLogs.size())) {
    return;
  }
  workspaceWriter_->deleteExportLog(
      snapshot.exportLogs[static_cast<std::size_t>(index)].id);
}

void WorkspaceFacade::clearExportLogs() {
  if (workspaceWriter_) {
    workspaceWriter_->clearExportLogs();
  }
}

QVariantList WorkspaceFacade::exportLogRows() const {
  QVariantList rows;
  if (!workspaceReader_) {
    return rows;
  }

  const auto snapshot = workspaceReader_->workspaceSnapshot();
  rows.reserve(static_cast<int>(snapshot.exportLogs.size()));
  for (const auto &log : snapshot.exportLogs) {
    const QString targetPath = QString::fromStdString(log.targetPath);
    const QString message = QString::fromStdString(log.message);
    QVariantMap row;
    row[payload::keys::exportRun::kLogId] = QString::fromStdString(log.id);
    row[payload::keys::exportRun::kTime] = QString::fromStdString(log.time);
    row[payload::keys::exportRun::kFile] = targetPath;
    row[payload::keys::common::kStatus] = QString::fromStdString(log.status);
    row[payload::keys::exportRun::kMessage] = message;
    row[payload::keys::exportRun::kPayload] =
        QString::fromStdString(log.payload);
    row[payload::keys::importRun::kDraftAttached] = false;
    row[payload::keys::importRun::kDraftId] = QString();
    row[payload::keys::importRun::kStatementId] = QString();
    row[payload::keys::common::kDisplayTime] = QString::fromStdString(log.time);
    row[payload::keys::common::kDisplayTitle] =
        displayFileName(targetPath);
    row[payload::keys::common::kDisplayStatusDetail] = message;
    rows.push_back(row);
  }
  return rows;
}

QString WorkspaceFacade::exportLogTargetPath(const QString &id,
                                             int fallbackIndex) const {
  if (!workspaceReader_) {
    return {};
  }
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  if (!id.isEmpty()) {
    const auto idStd = strings::toStdString(id);
    const auto it =
        std::find_if(snapshot.exportLogs.begin(), snapshot.exportLogs.end(),
                     [&](const auto &log) { return log.id == idStd; });
    if (it != snapshot.exportLogs.end()) {
      return QString::fromStdString(it->targetPath);
    }
  }
  if (fallbackIndex >= 0 &&
      fallbackIndex < static_cast<int>(snapshot.exportLogs.size())) {
    return QString::fromStdString(
        snapshot.exportLogs[static_cast<std::size_t>(fallbackIndex)]
            .targetPath);
  }
  return {};
}

void WorkspaceFacade::applyDeletionImpact(
    const core::ports::workspace::DeletionImpact &impact) {
  cache_->applyDeletionImpact(impact);
  bumpDataRevision();
}

} // namespace ui
