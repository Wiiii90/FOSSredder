/**
 * @file ui/src/workflows/ExportWorkflow.cpp
 * @brief Implements export workflow orchestration, logging, and async
 * execution.
 */

#include "ui/workflows/ExportWorkflow.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporting.h"
#include "core/ports/usecases/export/ExportRequest.h"
#include "ui/adapters/ExportAdapter.h"
#include "ui/i18n/Text.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"

#include <QDateTime>
#include <QMetaObject>
#include <QPointer>
#include <QUuid>
#include <QtConcurrent/qtconcurrentrun.h>

#include <algorithm>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

namespace ui {
namespace {

QString buildExportSuccessStatusMessage() {
  return ui::text::exporting::successDetail();
}

QString currentTimestamp() {
  return QDateTime::currentDateTime().toString(
      QStringLiteral("dd.MM.yyyy HH:mm:ss"));
}

QString generateLogId() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace

struct ExportWorkflow::ExportControlState {
  mutable std::mutex mutex;
  std::condition_variable changed;
  bool paused = false;
  bool canceled = false;

  bool shouldCancel() const {
    std::lock_guard lock(mutex);
    return canceled;
  }

  void waitIfPaused() {
    std::unique_lock lock(mutex);
    changed.wait(lock, [this]() {
      return !paused || canceled;
    });
  }

  void setPaused(bool value) {
    {
      std::lock_guard lock(mutex);
      paused = value;
    }
    changed.notify_all();
  }

  void cancel() {
    {
      std::lock_guard lock(mutex);
      canceled = true;
      paused = false;
    }
    changed.notify_all();
  }
};

ExportWorkflow::ExportWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    QObject* parent)
    : QObject(parent), stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      exportAdapter_(std::move(exportAdapter)),
      errorReporter_(std::move(errorReporter)) {
  if (!errorReporter_) {
    throw std::invalid_argument("ExportWorkflow requires an error reporter");
  }

  connect(&exportWatcher_,
          &QFutureWatcher<core::ports::exporting::ExportResult>::finished, this,
          &ExportWorkflow::onExportFinished);

  exportControl_ = std::make_shared<ExportControlState>();
}

void ExportWorkflow::setExportLogSink(ExportLogSink sink) {
  exportLogSink_ = std::move(sink);
}

int ExportWorkflow::currentMode() const noexcept {
  if (isRunning_)
    return ProgressMode;
  return CreateMode;
}

core::ports::workspace::WorkspaceSnapshot
ExportWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : core::ports::workspace::WorkspaceSnapshot{};
}

core::ports::exporting::ExportRequest
ExportWorkflow::buildRequest(int format, const QString& path,
                             bool includeFormulas, const QString& locale,
                             const QVariantMap& payload) const {
  if (!exportAdapter_) {
    return {};
  }
  return exportAdapter_->buildExportRequest(format, path, includeFormulas,
                                            locale, payload);
}

core::ports::workspace::ExportLogSnapshot
ExportWorkflow::publishExportLogById(const QString& logId, const QString& path,
                                     const QString& status,
                                     const QString& message) {
  core::ports::workspace::ExportLogSnapshot row;
  const QString resolvedLogId = logId.isEmpty() ? generateLogId() : logId;
  const auto snapshot = stateSnapshot();
  for (const auto& candidate : snapshot.exportLogs) {
    if (QString::fromStdString(candidate.id) == resolvedLogId) {
      row = candidate;
      break;
    }
  }

  row.id = resolvedLogId.toStdString();
  row.time = currentTimestamp().toStdString();
  row.targetPath = path.toStdString();
  row.status = status.toStdString();
  row.message = message.toStdString();
  publishExportLog(row);
  return row;
}

void ExportWorkflow::publishExportLog(
    const core::ports::workspace::ExportLogSnapshot& row) {
  if (!exportLogSink_) {
    return;
  }
  try {
    exportLogSink_(row);
  } catch (...) {
    core::errors::reportException(
        errorReporter_.get(), core::errors::ErrorSeverity::Error,
        core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kFinish,
        std::current_exception());
  }
}

void ExportWorkflow::clearActiveExportLog() {
  activeExportLogId_.clear();
  emit stateChanged();
}

void ExportWorkflow::finishExport(bool success, const QString& outputPath) {
  completedSteps_ = totalSteps_;
  progress_ = 1.0;
  phase_ = success ? ui::text::exporting::phaseFinished()
                   : ui::text::exporting::phaseFailed();

  const QString status = success ? ui::text::exporting::statusSuccess()
                                 : ui::text::exporting::statusFailed();
  const QString path = outputPath.isEmpty() ? activeExportPath_ : outputPath;
  QString message = lastError_;
  if (success && message.isEmpty()) {
    message = buildExportSuccessStatusMessage();
  }
  publishExportLogById(activeExportLogId_, path, status, message);

  isRunning_ = false;
  isPaused_ = false;
  cancelRequested_ = false;
  if (!success) {
    emit exportFailed(lastError_);
  }
  emit stateChanged();
  emit exportFinished(success);
}

void ExportWorkflow::finishCanceled() {
  completedSteps_ = totalSteps_;
  progress_ = 0.0;
  phase_ = ui::text::exporting::phaseCanceled();
  lastError_.clear();
  publishExportLogById(activeExportLogId_, activeExportPath_,
                       ui::text::exporting::statusCanceled(),
                       ui::text::exporting::phaseCanceled());

  isRunning_ = false;
  isPaused_ = false;
  cancelRequested_ = false;
  if (exportControl_) {
    exportControl_->cancel();
  }
  emit stateChanged();
  emit exportFinished(false);
}

void ExportWorkflow::exportDataWithPayload(int format, const QString& path,
                                           bool includeFormulas,
                                           const QString& locale,
                                           const QVariantMap& payload,
                                           int totalSteps) {
  if (isRunning_) {
    observability::reportFlow(
        errorReporter_.get(),
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::workflow::exportFlow::kStart,
        "Export ignored: already running");
    return;
  }

  try {
    lastError_.clear();
    exportControl_ = std::make_shared<ExportControlState>();
    totalSteps_ = std::max(1, totalSteps);
    completedSteps_ = 0;
    progress_ = 0.0;
    phase_ = ui::text::exporting::phaseStarting();
    activeExportPath_ = path;
    activeExportLogId_ = generateLogId();
    publishExportLogById(activeExportLogId_, activeExportPath_,
                         ui::text::exporting::statusRunning(),
                         ui::text::exporting::startingDetail());
    emit stateChanged();

    auto request = buildRequest(format, path, includeFormulas, locale, payload);
    auto control = exportControl_;
    request.shouldCancel = [control]() {
      return control && control->shouldCancel();
    };
    request.waitIfPaused = [control]() {
      if (control) {
        control->waitIfPaused();
      }
    };
    QPointer<ExportWorkflow> self(this);
    request.progressCallback = [self](double progressValue,
                                      const std::string& phaseText) {
      if (!self)
        return;
      QMetaObject::invokeMethod(
          self,
          [self, progressValue, phaseText]() {
            if (!self || !self->isRunning_)
              return;
            const double clamped = std::max(0.0, std::min(1.0, progressValue));
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
          errorReporter_.get(),
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kStart,
          "Export rejected: state snapshot unavailable",
          {{observability::context::kPath, strings::toStdString(path)}});
      publishExportLogById(activeExportLogId_, activeExportPath_,
                           ui::text::exporting::statusFailed(), lastError_);
      emit exportFailed(lastError_);
      emit exportFinished(false);
      emit stateChanged();
      return;
    }

    if (!exportAdapter_) {
      lastError_ = ui::text::exporting::runnerUnavailable();
      observability::reportFlow(
          errorReporter_.get(),
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kStart,
          "Export rejected: export runner unavailable",
          {{observability::context::kPath, strings::toStdString(path)}});
      publishExportLogById(activeExportLogId_, activeExportPath_,
                           ui::text::exporting::statusFailed(), lastError_);
      emit exportFailed(lastError_);
      emit exportFinished(false);
      emit stateChanged();
      return;
    }

    isRunning_ = true;
    phase_ = ui::text::exporting::phaseRunning();
    emit stateChanged();

    observability::traceWorkflow(
        observability::origins::workflow::exportFlow::kStart,
        "Export submitted",
        {{observability::context::kPath, strings::toStdString(path)},
         {observability::context::kFormat, std::to_string(format)},
         {observability::context::kIncludeFormulas,
          includeFormulas ? "true" : "false"},
         {observability::context::kLocale, strings::toStdString(locale)}});
    observability::reportFlow(
        errorReporter_.get(),
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::workflow::exportFlow::kStart, "Export started",
        {{observability::context::kPath, strings::toStdString(path)},
         {observability::context::kFormat, std::to_string(format)},
         {observability::context::kIncludeFormulas,
          includeFormulas ? "true" : "false"},
         {observability::context::kLocale, strings::toStdString(locale)}});

    const auto snapshot = stateSnapshot();
    exportFuture_ = QtConcurrent::run([exportAdapter = exportAdapter_, snapshot,
                                       request = std::move(request)]() mutable {
      return exportAdapter->runExport(snapshot, std::move(request));
    });
    exportWatcher_.setFuture(exportFuture_);
  } catch (const std::exception& ex) {
    core::errors::report(
        errorReporter_.get(), core::errors::ErrorSeverity::Error,
        core::errors::codes::ExceptionStd,
        observability::origins::workflow::exportFlow::kStart, ex.what());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        errorReporter_.get(),
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kStart,
        "Export failed with exception",
        {{observability::context::kException, ex.what()},
         {observability::context::kPath, strings::toStdString(path)}});
    finishExport(false);
  } catch (...) {
    core::errors::reportException(
        errorReporter_.get(), core::errors::ErrorSeverity::Error,
        core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kStart,
        std::current_exception());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        errorReporter_.get(),
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
  if (exportControl_) {
    exportControl_->cancel();
  }
  phase_ = ui::text::exporting::phaseCancelRequested();
  emit stateChanged();
}

void ExportWorkflow::pauseExport() {
  if (!isRunning_ || isPaused_)
    return;
  isPaused_ = true;
  if (exportControl_) {
    exportControl_->setPaused(isPaused_);
  }
  phase_ = ui::text::exporting::phasePaused();
  emit stateChanged();
}

void ExportWorkflow::resumeExport() {
  if (!isRunning_ || !isPaused_)
    return;
  isPaused_ = false;
  if (exportControl_) {
    exportControl_->setPaused(isPaused_);
  }
  phase_ = ui::text::exporting::phaseRunning();
  emit stateChanged();
}

void ExportWorkflow::onExportFinished() {
  if (cancelRequested_) {
    finishCanceled();
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
    if (result.status == core::ports::exporting::ExportStatus::Canceled) {
      finishCanceled();
      return;
    }
    if (!success) {
      lastError_ = result.message.empty()
                       ? ui::text::workflowErrors::exportFailed()
                       : QString::fromStdString(result.message);
      core::errors::report(
          errorReporter_.get(), core::errors::ErrorSeverity::Warning,
          result.errorCode.empty() ? core::errors::codes::GenericError
                                   : result.errorCode.c_str(),
          observability::origins::workflow::exportFlow::kFinish,
          strings::toStdString(lastError_));
      observability::reportFlow(
          errorReporter_.get(),
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::workflow::exportFlow::kFinish,
          "Export finished with failure",
          {{observability::context::kError, strings::toStdString(lastError_)}});
    } else {
      lastError_.clear();
      observability::reportFlow(
          errorReporter_.get(),
          core::errors::ErrorSeverity::Info,
          observability::codes::FlowExportFinished,
          observability::origins::workflow::exportFlow::kFinish,
          "Export finished successfully");
    }
  } catch (const std::exception& ex) {
    core::errors::report(
        errorReporter_.get(), core::errors::ErrorSeverity::Error,
        core::errors::codes::ExceptionStd,
        observability::origins::workflow::exportFlow::kFinish, ex.what());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        errorReporter_.get(),
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kFinish,
        "Export finished with std exception",
        {{observability::context::kException, ex.what()}});
    success = false;
  } catch (...) {
    core::errors::reportException(
        errorReporter_.get(), core::errors::ErrorSeverity::Error,
        core::errors::codes::ExceptionError,
        observability::origins::workflow::exportFlow::kFinish,
        std::current_exception());
    lastError_ = ui::text::workflowErrors::exportFailed();
    observability::reportFlow(
        errorReporter_.get(),
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::workflow::exportFlow::kFinish,
        "Export finished with non-std exception");
    success = false;
  }

  finishExport(success, resolvedOutputPath);
}

} // namespace ui
