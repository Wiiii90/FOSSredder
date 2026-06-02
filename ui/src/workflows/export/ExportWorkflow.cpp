/**
 * @file ui/src/workflows/export/ExportWorkflow.cpp
 * @brief Implements export workflow orchestration, logging, and async execution.
 */

#include "ui/workflows/export/ExportWorkflow.h"

#include "ui/adapters/ExportAdapter.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/ports/export/ExportRequest.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/observability/Trace.h"
#include "ui/shared/text/Text.h"
#include "ui/shared/util/StringConversions.h"

#include <QDateTime>
#include <QMetaObject>
#include <QPointer>
#include <QUuid>
#include <QtConcurrent/qtconcurrentrun.h>

#include <algorithm>
#include <exception>
#include <string>

namespace ui {
namespace {

QString buildExportSuccessStatusMessage() {
  return ui::text::exportRuns::successDetail();
}

QString currentTimestamp() {
  return QDateTime::currentDateTime().toString(
      QStringLiteral("dd.MM.yyyy HH:mm:ss"));
}

QString generateLogId() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

core::ports::workspace::ExportLogSnapshot
toExportLogSnapshot(const ExportLogRow &row) {
  core::ports::workspace::ExportLogSnapshot log;
  log.id = row.logId.toStdString();
  log.time = row.time.toStdString();
  log.targetPath = row.file.toStdString();
  log.status = row.status.toStdString();
  log.message = row.message.toStdString();
  log.payload = row.payload.toStdString();
  return log;
}

ExportLogRow
toExportLogRow(const core::ports::workspace::ExportLogSnapshot &log) {
  ExportLogRow row;
  row.logId = QString::fromStdString(log.id);
  row.time = QString::fromStdString(log.time);
  row.file = QString::fromStdString(log.targetPath);
  row.status = QString::fromStdString(log.status);
  row.message = QString::fromStdString(log.message);
  row.payload = QString::fromStdString(log.payload);
  return row;
}

} // namespace

ExportWorkflow::ExportWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter, QObject *parent)
    : QObject(parent),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      exportAdapter_(std::move(exportAdapter)) {
  connect(&exportWatcher_,
          &QFutureWatcher<core::ports::exporting::ExportResult>::finished, this,
          &ExportWorkflow::onExportFinished);

  restoreExportLogsFromSnapshot();
}

void ExportWorkflow::refreshFromStateSnapshot() {
  restoreExportLogsFromSnapshot();
  emit stateChanged();
}

void ExportWorkflow::setExportLogSink(ExportLogSink sink) {
  exportLogSink_ = std::move(sink);
}

int ExportWorkflow::currentMode() const noexcept {
  if (isRunning_)
    return ProgressMode;
  return CreateMode;
}

core::ports::workspace::WorkspaceSnapshot ExportWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

core::ports::exporting::ExportRequest
ExportWorkflow::buildRequest(int format, const QString &path,
                             bool includeFormulas,
                             const QString &locale) const {
  if (!exportAdapter_) {
    return {};
  }
  return exportAdapter_->buildExportRequest(format, path, includeFormulas,
                                            locale, pendingPayload_);
}

ExportLogRow ExportWorkflow::upsertExportLogById(const QString &logId,
                                                 const QString &path,
                                                 const QString &status,
                                                 const QString &message,
                                                 const QString &payload) {
  ExportLogRow row;
  row.logId = logId.isEmpty() ? generateLogId() : logId;
  auto existing =
      std::find_if(exportLogs_.begin(), exportLogs_.end(),
                   [&row](const ExportLogRow &candidate) {
                     return candidate.logId == row.logId;
                   });
  if (existing != exportLogs_.end()) {
    row = *existing;
  }
  row.time = currentTimestamp();
  row.file = path;
  row.status = status;
  row.message = message;
  row.payload = payload;
  if (existing != exportLogs_.end()) {
    *existing = row;
  } else {
    exportLogs_.push_back(row);
  }
  publishExportLog(row);
  return row;
}

void ExportWorkflow::restoreExportLogsFromSnapshot() {
  exportLogs_.clear();
  const auto snapshot = stateSnapshot();
  exportLogs_.reserve(snapshot.exportLogs.size());
  for (const auto &log : snapshot.exportLogs) {
    exportLogs_.push_back(toExportLogRow(log));
  }
}

void ExportWorkflow::publishExportLog(const ExportLogRow &row) {
  if (!exportLogSink_) {
    return;
  }
  try {
    exportLogSink_(toExportLogSnapshot(row));
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kFinish,
        std::current_exception());
  }
}

void ExportWorkflow::clearActiveExportLog() {
  activeExportLogId_.clear();
  emit stateChanged();
}

void ExportWorkflow::finishExport(bool success, const QString &outputPath) {
  completedSteps_ = totalSteps_;
  progress_ = 1.0;
  phase_ = success ? QStringLiteral("Finished") : QStringLiteral("Failed");

  const QString status =
      success ? QStringLiteral("Success") : QStringLiteral("Failed");
  const QString path = outputPath.isEmpty() ? activeExportPath_ : outputPath;
  QString message = lastError_;
  if (success && message.isEmpty()) {
    message = buildExportSuccessStatusMessage();
  }
  upsertExportLogById(activeExportLogId_, path, status, message,
                      pendingPayload_);

  isRunning_ = false;
  isPaused_ = false;
  cancelRequested_ = false;
  if (!success) {
    emit exportFailed(lastError_);
  }
  emit stateChanged();
  emit exportFinished(success);
}

void ExportWorkflow::exportData(int format, const QString &path,
                                bool includeFormulas, const QString &locale) {
  exportDataWithPayload(format, path, includeFormulas, locale, QString(), 1);
}

void ExportWorkflow::exportDataWithPayload(int format, const QString &path,
                                           bool includeFormulas,
                                           const QString &locale,
                                           const QString &payload,
                                           int totalSteps) {
  if (isRunning_) {
    observability::reportFlow(
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::workflow::exportFlow::kStart,
        "Export ignored: already running");
    return;
  }

  try {
    lastError_.clear();
    totalSteps_ = std::max(1, totalSteps);
    completedSteps_ = 0;
    progress_ = 0.0;
    phase_ = QStringLiteral("Starting export...");
    pendingPayload_ = payload;
    activeExportPath_ = path;
    activeExportLogId_ = generateLogId();
    upsertExportLogById(activeExportLogId_, activeExportPath_,
                        QStringLiteral("Running"),
                        ui::text::exportRuns::startingDetail(),
                        pendingPayload_);
    emit stateChanged();

    auto request = buildRequest(format, path, includeFormulas, locale);
    QPointer<ExportWorkflow> self(this);
    request.progressCallback = [self](double progressValue,
                                      const std::string &phaseText) {
      if (!self)
        return;
      QMetaObject::invokeMethod(
          self,
          [self, progressValue, phaseText]() {
            if (!self || !self->isRunning_)
              return;
            const double clamped =
                std::max(0.0, std::min(1.0, progressValue));
            self->progress_ = clamped;
            if (!phaseText.empty()) {
              self->phase_ = QString::fromStdString(phaseText);
            }
            emit self->stateChanged();
          },
          Qt::QueuedConnection);
    };

    if (!stateSnapshotProvider_) {
      lastError_ = ui::text::workflowErrors::exportStateUnavailable();
      observability::reportFlow(
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kStart,
          "Export rejected: state snapshot unavailable",
          {{observability::context::kPath, strings::toStdString(path)}});
      upsertExportLogById(activeExportLogId_, activeExportPath_,
                          QStringLiteral("Failed"), lastError_,
                          pendingPayload_);
      emit exportFailed(lastError_);
      emit exportFinished(false);
      emit stateChanged();
      return;
    }

    if (!exportAdapter_) {
      lastError_ = ui::text::exportRunner::runnerUnavailable();
      observability::reportFlow(
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kStart,
          "Export rejected: export runner unavailable",
          {{observability::context::kPath, strings::toStdString(path)}});
      upsertExportLogById(activeExportLogId_, activeExportPath_,
                          QStringLiteral("Failed"), lastError_,
                          pendingPayload_);
      emit exportFailed(lastError_);
      emit exportFinished(false);
      emit stateChanged();
      return;
    }

    isRunning_ = true;
    phase_ = QStringLiteral("Running export...");
    emit stateChanged();

    observability::reportFlow(
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::workflow::exportFlow::kStart, "Export started",
        {{observability::context::kPath, strings::toStdString(path)},
         {observability::context::kFormat, std::to_string(format)},
         {observability::context::kIncludeFormulas,
          includeFormulas ? "true" : "false"},
         {observability::context::kLocale, strings::toStdString(locale)}});

    const auto snapshot = stateSnapshot();
    exportFuture_ = QtConcurrent::run(
        [exportAdapter = exportAdapter_, snapshot,
         request = std::move(request)]() mutable {
          return exportAdapter->runExport(snapshot, std::move(request));
        });
    exportWatcher_.setFuture(exportFuture_);
  } catch (const std::exception &ex) {
    core::errors::report(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionStd,
        observability::origins::workflow::exportFlow::kStart, ex.what());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kStart,
        "Export failed with exception",
        {{observability::context::kException, ex.what()},
         {observability::context::kPath, strings::toStdString(path)}});
    finishExport(false);
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kStart,
        std::current_exception());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kStart,
        "Export failed with non-std exception",
        {{observability::context::kPath, strings::toStdString(path)}});
    finishExport(false);
  }
}

void ExportWorkflow::cancelExport() {
  if (!isRunning_)
    return;
  cancelRequested_ = true;
  phase_ = QStringLiteral("Cancel requested...");
  emit stateChanged();
}

void ExportWorkflow::togglePause() {
  if (!isRunning_)
    return;
  isPaused_ = !isPaused_;
  phase_ = isPaused_ ? QStringLiteral("Paused")
                     : QStringLiteral("Running export...");
  emit stateChanged();
}

void ExportWorkflow::onExportFinished() {
  if (cancelRequested_) {
    lastError_.clear();
    phase_ = QStringLiteral("Canceled");
    finishExport(false, QString());
    return;
  }

  bool success = false;
  QString resolvedOutputPath;
  try {
    const auto result = exportFuture_.result();
    success = result.success;
    if (success) {
      resolvedOutputPath = QString::fromStdString(result.resolvedOutputPath);
    }
    if (!success) {
      lastError_ = result.message.empty()
                       ? ui::text::workflowErrors::exportFailed()
                       : QString::fromStdString(result.message);
      core::errors::report(
          core::errors::ErrorSeverity::Warning,
          result.errorCode.empty() ? core::errors::codes::GenericError
                                   : result.errorCode.c_str(),
          observability::origins::workflow::exportFlow::kFinish,
          strings::toStdString(lastError_));
      observability::reportFlow(
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kFinish,
          "Export finished with failure",
          {{observability::context::kError, strings::toStdString(lastError_)}});
    } else {
      lastError_.clear();
      observability::reportFlow(
          core::errors::ErrorSeverity::Info,
          observability::codes::FlowExportFinished,
          observability::origins::workflow::exportFlow::kFinish,
          "Export finished successfully");
    }
  } catch (const std::exception &ex) {
    core::errors::report(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionStd,
        observability::origins::workflow::exportFlow::kFinish, ex.what());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kFinish,
        "Export finished with std exception",
        {{observability::context::kException, ex.what()}});
    success = false;
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kFinish,
        std::current_exception());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kFinish,
        "Export finished with non-std exception");
    success = false;
  }

  finishExport(success, resolvedOutputPath);
}

} // namespace ui
