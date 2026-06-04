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
  Q_ENUM(Mode)

  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;
  using ExportLogSink =
      std::function<void(const core::ports::workspace::ExportLogSnapshot&)>;

  /** @brief Create an export workflow backed by a snapshot provider and export
   * runner. */
  explicit ExportWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter,
      QObject* parent = nullptr);

  void refreshFromStateSnapshot();
  void setExportLogSink(ExportLogSink sink);

  bool isRunning() const noexcept {
    return isRunning_;
  }
  bool isPaused() const noexcept {
    return isPaused_;
  }
  double progress() const noexcept {
    return progress_;
  }
  QString phase() const {
    return phase_;
  }
  QString error() const {
    return lastError_;
  }
  int currentMode() const noexcept;

  /** @brief Start an asynchronous export with the selected UI options.
   *  @param format Export format enum value
   *  @param path Output path
   *  @param includeFormulas Whether to include formulas
   *  @param locale Locale identifier
   */
  void exportData(int format, const QString& path, bool includeFormulas = true,
                  const QString& locale = QString());
  void exportDataWithPayload(int format, const QString& path,
                             bool includeFormulas, const QString& locale,
                             const QVariantMap& payload, int totalSteps = 1);
  void clearActiveExportLog();
  void cancelExport();
  void pauseExport();
  void resumeExport();

signals:
  void stateChanged();
  void exportFinished(bool success);
  void exportFailed(const QString& error);

private slots:
  /** @brief Finalize UI state once the asynchronous export completes. */
  void onExportFinished();

private:
  core::ports::exporting::ExportRequest
  buildRequest(int format, const QString& path, bool includeFormulas,
               const QString& locale) const;
  void publishExportLog(const core::ports::workspace::ExportLogSnapshot& log);
  core::ports::workspace::ExportLogSnapshot
  upsertExportLogById(const QString& logId, const QString& path,
                      const QString& status, const QString& message,
                      const QString& payload);
  void finishExport(bool success, const QString& outputPath = QString());
  void finishCanceled();
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;
  struct ExportControlState;

  StateSnapshotProvider stateSnapshotProvider_;
  ExportLogSink exportLogSink_;
  std::shared_ptr<ui::adapters::ExportAdapter> exportAdapter_;
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
  QString pendingPayload_;
  QVariantMap pendingPayloadMap_;
  QString activeExportLogId_;
  QString activeExportPath_;
  QString lastError_;
};

} // namespace ui
