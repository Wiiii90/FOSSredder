/**
 * @file app/src/main_qml.cpp
 * @brief Boots the QML application shell and wires UI-facing services.
 */

#ifdef USE_QML
#include "MainWindow.h"
#include "core/errors/IErrorReporter.h"
#include "core/ports/analysis/IAnalysisRunner.h"
#include "core/ports/annual/IAnnualRunner.h"
#include "core/ports/export/IExportRunner.h"
#include "core/ports/import/IImportRunner.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

#include "ui/shared/observability/ErrorCodes.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shell/Composition.h"
#include "ui/workspace/WorkspaceFacade.h"
#include <ui/shared/observability/Trace.h>
#include <QApplication>
#include <QList>
#include <QQmlEngine>
#include <QQmlError>

#include <memory>
#include <utility>

namespace {

void wireFileSignals(MainWindow &w, ui::WorkspaceFacade *workspace) {
  if (!workspace)
    return;

  QObject::connect(
      &w, &MainWindow::newFileRequested, workspace,
      [workspace](const QString &path) { workspace->newFile(path); });
  QObject::connect(
      &w, &MainWindow::openFileRequested, workspace,
      [workspace](const QString &path) { workspace->openFile(path); });
  QObject::connect(&w, &MainWindow::saveFileRequested, workspace,
                   [workspace]() { workspace->saveFile(); });
  QObject::connect(
      &w, &MainWindow::saveFileAsRequested, workspace,
      [workspace](const QString &path) { workspace->saveFileAs(path); });
  QObject::connect(workspace, &ui::WorkspaceFacade::operationSucceeded, &w,
                   &MainWindow::handleStorageOperationSucceeded);
  QObject::connect(workspace, &ui::WorkspaceFacade::operationFailed, &w,
                   &MainWindow::handleStorageOperationFailed);
}

void wireQmlWarnings(
    MainWindow &w,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  auto *engine = w.qmlEngine();
  if (!engine || !errorReporter)
    return;

  QObject::connect(
      engine, &QQmlEngine::warnings, &w,
      [errorReporter](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
          core::errors::ErrorEvent event;
          event.severity = core::errors::ErrorSeverity::Warning;
          event.code = ui::observability::codes::QmlWarning;
          event.origin = ui::observability::origins::app::kQmlWarnings;
          event.message = warning.description().toStdString();
          event.context.emplace_back(ui::observability::context::kUrl,
                                     warning.url().toString().toStdString());
          event.context.emplace_back(ui::observability::context::kLine,
                                     std::to_string(warning.line()));
          event.context.emplace_back(ui::observability::context::kColumn,
                                     std::to_string(warning.column()));
          errorReporter->report(event);
        }
      });
}

} // namespace

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication &app,
                core::ports::workspace::IWorkspaceReader &workspaceReader,
                core::ports::workspace::IWorkspaceWriter &workspaceWriter,
                std::shared_ptr<core::errors::IErrorReporter> errorReporter,
                std::shared_ptr<core::ports::analysis::IAnalysisRunner>
                    analysisRunner,
                std::shared_ptr<core::ports::annual::IAnnualRunner>
                    annualRunner,
                std::shared_ptr<core::ports::exporting::IExportRunner>
                    exportRunner,
                std::shared_ptr<core::ports::importing::IImportRunner>
                    importRunner) {
  MainWindow w;

  workspaceWriter.setErrorReporter(errorReporter);

  const ui::shell::Composition composition =
      ui::shell::createComposition(
          app, w, workspaceReader, workspaceWriter, errorReporter,
          std::move(analysisRunner), std::move(annualRunner),
          std::move(exportRunner), std::move(importRunner));

  ui::shell::wireAppStateToSession(w, composition, workspaceWriter,
                                   errorReporter);
  ui::shell::refreshComposition(composition);

  wireFileSignals(w, w.workspace());

  wireQmlWarnings(w, errorReporter);

  w.loadQml();

  ui::shell::refreshComposition(composition);

  w.show();
  const int exitCode = app.exec();

  if (w.workspace())
    w.workspace()->setWorkspacePorts(nullptr, nullptr);
  workspaceWriter.setSnapshotChangedCallback({});
  workspaceWriter.setDeletionImpactCallback({});
  workspaceWriter.setErrorReporter({});

  return exitCode;
}

#endif
