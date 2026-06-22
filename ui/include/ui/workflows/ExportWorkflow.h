/**
 * @file ui/include/ui/workflows/ExportWorkflow.h
 * @brief Declares the asynchronous export workflow used by ExportViewModel.
 */

#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <functional>
#include <memory>

#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/ports/usecases/export/ExportResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {
class ExportAdapter;
}

namespace ui {

/**
 * @brief Coordinates asynchronous exports from a read-only state snapshot.
 */
class ExportWorkflow : public QObject {
  Q_OBJECT
public:
  enum Mode {
    CreateMode = 0,
    ProgressMode = 1
  };

  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;
  using ExportLogSink =
      std::function<void(const core::ports::workspace::ExportLogSnapshot&)>;

  /**
   * @brief Creates an export workflow backed by a snapshot provider and runner.
   * @param stateSnapshotProvider Provider for the current workspace snapshot.
   * @param exportAdapter Adapter used to invoke the core export runner.
   * @param errorReporter Error reporter used for workflow diagnostics.
   * @param parent Optional Qt parent.
   */
  explicit ExportWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter,
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
      QObject* parent = nullptr);

  /**
   * @brief Sets the sink used to save export logs through workspace commands.
   * @param sink Export log sink callback.
   */
  void setExportLogSink(ExportLogSink sink);

  /** @brief Reports whether an export is running. @return True while running.
   */
  bool isRunning() const noexcept {
    return isRunning_;
  }
  /** @brief Reports whether the export is paused. @return True while paused. */
  bool isPaused() const noexcept {
    return isPaused_;
  }
  /** @brief Returns export progress. @return Progress in the range 0..1. */
  double progress() const noexcept {
    return progress_;
  }
  /** @brief Returns current export phase text. @return Phase text. */
  QString phase() const {
    return phase_;
  }
  /** @brief Returns current export error text. @return Error text or empty. */
  QString error() const {
    return lastError_;
  }
  /**
   * @brief Returns the current export workflow mode.
   * @return CreateMode or ProgressMode.
   */
  int currentMode() const noexcept;

  /**
   * @brief Starts an export using a QML selection payload.
   * @param format QML export format index.
   * @param path Output path.
   * @param includeFormulas Whether formulas should be included.
   * @param locale Export locale.
   * @param payload Selected export objects.
   * @param totalSteps Total UI progress steps.
   */
  void exportDataWithPayload(int format, const QString& path,
                             bool includeFormulas, const QString& locale,
                             const QVariantMap& payload, int totalSteps = 1);
  /**
   * @brief Clears the active export log selection.
   */
  void clearActiveExportLog();
  /**
   * @brief Requests cancellation of the running export.
   */
  void cancelExport();
  /**
   * @brief Pauses the running export.
   */
  void pauseExport();
  /**
   * @brief Resumes the paused export.
   */
  void resumeExport();

signals:
  /**
   * @brief Emitted when export workflow state changed.
   */
  void stateChanged();
  /**
   * @brief Emitted when an export finished or stopped.
   * @param success True when the export completed successfully.
   */
  void exportFinished(bool success);
  /**
   * @brief Emitted when an export failed.
   * @param error Error message.
   */
  void exportFailed(const QString& error);

private slots:
  /** @brief Finalize UI state once the asynchronous export completes. */
  void onExportFinished();

private:
  /**
   * @brief Builds the core export request from UI fields.
   * @param format QML export format index.
   * @param path Output path.
   * @param includeFormulas Whether formulas should be included.
   * @param locale Export locale.
   * @param payload Selected export objects.
   * @return Core export request.
   */
  core::ports::exporting::ExportRequest
  buildRequest(int format, const QString& path, bool includeFormulas,
               const QString& locale, const QVariantMap& payload) const;
  /**
   * @brief Publishes an export log snapshot through the configured sink.
   * @param log Export log snapshot.
   */
  void publishExportLog(const core::ports::workspace::ExportLogSnapshot& log);
  /**
   * @brief Builds and publishes an export log snapshot for a log id.
   * @param logId Existing log id or empty for a new id.
   * @param path Target path.
   * @param status Log status.
   * @param message Log message.
   * @return Published export log snapshot.
   */
  core::ports::workspace::ExportLogSnapshot
  publishExportLogById(const QString& logId, const QString& path,
                       const QString& status, const QString& message);
  /**
   * @brief Finalizes workflow state after an export result.
   * @param success Whether the export succeeded.
   * @param outputPath Resolved output path.
   */
  void finishExport(bool success, const QString& outputPath = QString());
  /**
   * @brief Finalizes workflow state after cancellation.
   */
  void finishCanceled();
  /**
   * @brief Returns the current workspace snapshot.
   * @return Workspace snapshot or an empty snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;
  struct ExportControlState;

  StateSnapshotProvider stateSnapshotProvider_;
  ExportLogSink exportLogSink_;
  std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter_;
  std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
  std::shared_ptr<ExportControlState> exportControl_;
  QFuture<core::ports::exporting::ExportResult> exportFuture_;
  QFutureWatcher<core::ports::exporting::ExportResult> exportWatcher_;
  bool isRunning_ = false;
  bool isPaused_ = false;
  bool cancelRequested_ = false;
  double progress_ = 0.0;
  QString phase_;
  int totalSteps_ = 1;
  int completedSteps_ = 0;
  QString activeExportLogId_;
  QString activeExportPath_;
  QString lastError_;
};

} // namespace ui
