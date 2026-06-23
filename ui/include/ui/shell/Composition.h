/**
 * @file ui/include/ui/shell/Composition.h
 * @brief Wires QML shell workflows and view models.
 */

#pragma once

#include <memory>

class QApplication;
class MainWindow;

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace core::ports::analysis {
class IAnalysisRunner;
}

namespace core::ports::annual {
class IAnnualRunner;
}

namespace core::ports::exporting {
class IExportRunner;
}

namespace core::ports::importing {
class IImportRunner;
}

namespace core::ports::workspace {
class IWorkspaceReader;
class IWorkspaceWriter;
} // namespace core::ports::workspace

namespace ui::shell {

/**
 * @brief Creates and wires QML-facing workflows and view models.
 * @param app Qt application used for platform services.
 * @param w Main window owning the created QObject graph.
 * @param workspaceReader Core workspace read port.
 * @param workspaceWriter Core workspace write port.
 * @param errorReporter Error reporter for workflow and adapter boundaries.
 * @param analysisRunner Core analysis use-case runner.
 * @param annualRunner Core annual use-case runner.
 * @param exportRunner Core export use-case runner.
 * @param importRunner Core import use-case runner.
 */
void createComposition(
    QApplication& app, MainWindow& w,
    core::ports::workspace::IWorkspaceReader& workspaceReader,
    core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner);

/**
 * @brief Wires workspace writer callbacks back into the UI workspace store.
 * @param w Main window owning the workspace store.
 * @param workspaceWriter Core workspace write port.
 * @param errorReporter Error reporter for callback failures.
 */
void wireWorkspaceCallbacks(
    MainWindow& w, core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& errorReporter);

} // namespace ui::shell
