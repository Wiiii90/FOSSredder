/**
 * @file ui/include/ui/shell/Composition.h
 * @brief Wires QML shell workflows and view models.
 */

#pragma once

#include <memory>

class QApplication;
class MainWindow;

namespace core::errors {
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
}

namespace ui::shell {

class Composition {
public:
  Composition();

private:
  struct State;

  explicit Composition(std::shared_ptr<State> state);

  std::shared_ptr<State> state_;

  friend Composition createComposition(
      QApplication &app, MainWindow &w,
      core::ports::workspace::IWorkspaceReader &workspaceReader,
      core::ports::workspace::IWorkspaceWriter &workspaceWriter,
      const std::shared_ptr<core::errors::IErrorReporter> &errorReporter,
      std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
      std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
      std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
      std::shared_ptr<core::ports::importing::IImportRunner> importRunner);
  friend void refreshComposition(const Composition &composition);
};

Composition createComposition(
    QApplication &app, MainWindow &w,
    core::ports::workspace::IWorkspaceReader &workspaceReader,
    core::ports::workspace::IWorkspaceWriter &workspaceWriter,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner);

void wireAppStateToSession(
    MainWindow &w, const Composition &composition,
    core::ports::workspace::IWorkspaceWriter &workspaceWriter,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter);

void refreshComposition(const Composition &composition);

} // namespace ui::shell
