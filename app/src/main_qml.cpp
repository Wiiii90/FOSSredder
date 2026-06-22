/**
 * @file app/src/main_qml.cpp
 * @brief Boots the QML application shell and wires UI-facing services.
 */

#ifdef USE_QML
#include "MainWindow.h"
#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/ports/usecases/analysis/IAnalysisRunner.h"
#include "core/ports/usecases/annual/IAnnualRunner.h"
#include "core/ports/usecases/export/IExportRunner.h"
#include "core/ports/usecases/import/IImportRunner.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

#include "ui/shell/Composition.h"
#include "ui/shell/QmlDiagnostics.h"
#include <QApplication>

#include <memory>
#include <utility>

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param workspaceReader Workspace read port exposed to the UI composition.
 * @param workspaceWriter Workspace write port exposed to the UI composition.
 * @param errorReporter Reporter used by UI and workspace boundaries.
 * @param analysisRunner Analysis use-case runner.
 * @param annualRunner Annual use-case runner.
 * @param exportRunner Export use-case runner.
 * @param importRunner Import use-case runner.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(
    QApplication& app,
    core::ports::workspace::IWorkspaceReader& workspaceReader,
    core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner) {
  MainWindow w(errorReporter);

  ui::shell::createComposition(app, w, workspaceReader, workspaceWriter,
                               errorReporter, std::move(analysisRunner),
                               std::move(annualRunner), std::move(exportRunner),
                               std::move(importRunner));

  ui::shell::wireWorkspaceCallbacks(w, workspaceWriter, errorReporter);
  ui::bootstrap::wireQmlWarnings(w.qmlEngine(), &w, errorReporter);

  w.loadQml();

  w.show();
  return app.exec();
}

#endif
