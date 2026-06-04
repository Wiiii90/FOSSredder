/**
 * @file ui/src/shell/Composition.cpp
 * @brief Wires QML shell workflows and view models.
 */

#include "ui/shell/Composition.h"

#include "MainWindow.h"
#include "core/errors/IErrorReporter.h"
#include "core/ports/usecases/analysis/IAnalysisRunner.h"
#include "core/ports/usecases/annual/IAnnualRunner.h"
#include "core/ports/usecases/export/IExportRunner.h"
#include "core/ports/usecases/import/IImportRunner.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/adapters/AnalysisAdapter.h"
#include "ui/adapters/AnnualAdapter.h"
#include "ui/adapters/ExportAdapter.h"
#include "ui/adapters/ImportAdapter.h"
#include "ui/platform/LanguageService.h"
#include "ui/shell/AppContext.h"
#include "ui/shell/Settings.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/viewmodels/AnnualViewModel.h"
#include "ui/viewmodels/ActorViewModel.h"
#include "ui/viewmodels/BookingViewModel.h"
#include "ui/viewmodels/ContractViewModel.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/viewmodels/PropertyViewModel.h"
#include "ui/viewmodels/SettingsViewModel.h"
#include "ui/workflows/AnalysisWorkflow.h"
#include "ui/workflows/AnnualWorkflow.h"
#include "ui/workflows/ExportWorkflow.h"
#include "ui/workflows/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

#include <QApplication>

#include <exception>
#include <utility>

namespace ui::shell {

struct Composition::State {
  ui::ExportWorkflow *exportWorkflow = nullptr;
  ui::ImportWorkflow *importWorkflow = nullptr;
};

Composition::Composition() = default;

Composition::Composition(std::shared_ptr<State> state)
    : state_(std::move(state)) {}

Composition createComposition(
    QApplication &app, MainWindow &w,
    core::ports::workspace::IWorkspaceReader &workspaceReader,
    core::ports::workspace::IWorkspaceWriter &workspaceWriter,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner) {
  auto state = std::make_shared<Composition::State>();

  if (w.workspace())
    w.workspace()->setWorkspacePorts(&workspaceWriter, &workspaceReader);
  if (auto *appContext = w.appContext()) {
    appContext->setWorkspaceFacade(w.workspace());
  }

  const auto workspaceSnapshotProvider = [&workspaceReader]() {
    return workspaceReader.workspaceSnapshot();
  };

  auto analysisAdapter =
      std::make_shared<ui::adapters::AnalysisAdapter>(std::move(analysisRunner));
  auto *analysisWorkflow =
      new ui::AnalysisWorkflow(workspaceSnapshotProvider, analysisAdapter, &w);

  auto annualAdapter =
      std::make_shared<ui::adapters::AnnualAdapter>(std::move(annualRunner));
  auto *annualWorkflow =
      new ui::AnnualWorkflow(workspaceSnapshotProvider, annualAdapter, &w);

  auto exportAdapter =
      std::make_shared<ui::adapters::ExportAdapter>(std::move(exportRunner));
  state->exportWorkflow =
      new ui::ExportWorkflow(workspaceSnapshotProvider, exportAdapter, &w);

  ui::LanguageService *languageService = nullptr;
  if (auto *appContext = w.appContext()) {
    languageService = appContext->languageService();
  }
  if (!languageService) {
    languageService = new ui::LanguageService(&app, w.qmlEngine(), &w);
  }
  if (auto *appContext = w.appContext())
    appContext->setLanguageService(languageService);

  ui::Settings *settings = w.settings();
  if (!settings) {
    settings = new ui::Settings(&w);
  }

  auto importAdapter =
      std::make_shared<ui::adapters::ImportAdapter>(std::move(importRunner));
  state->importWorkflow =
      new ui::ImportWorkflow(importAdapter, errorReporter, w.workspace(), &w);
  if (w.workspace()) {
    w.workspace()->setImportWorkflowForSave(state->importWorkflow);
  }

  if (auto *appContext = w.appContext()) {
    auto *actorViewModel = new ui::ActorViewModel(w.workspace(), &w);
    appContext->setActorViewModel(actorViewModel);

    auto *bookingViewModel = new ui::BookingViewModel(w.workspace(), &w);
    appContext->setBookingViewModel(bookingViewModel);

    auto *contractViewModel = new ui::ContractViewModel(w.workspace(), &w);
    appContext->setContractViewModel(contractViewModel);

    auto *propertyViewModel = new ui::PropertyViewModel(w.workspace(), &w);
    appContext->setPropertyViewModel(propertyViewModel);

    auto *analysisViewModel = new ui::AnalysisViewModel(&w);
    analysisViewModel->setWorkspace(w.workspace());
    analysisViewModel->setAnalysisWorkflow(analysisWorkflow);
    analysisViewModel->setSettings(settings);
    appContext->setAnalysisViewModel(analysisViewModel);

    auto *annualViewModel = new ui::AnnualViewModel(&w);
    annualViewModel->setWorkspace(w.workspace());
    annualViewModel->setAnnualWorkflow(annualWorkflow);
    appContext->setAnnualViewModel(annualViewModel);

    auto *exportViewModel = new ui::ExportViewModel(&w);
    exportViewModel->setWorkspace(w.workspace());
    exportViewModel->setExportWorkflow(state->exportWorkflow);
    exportViewModel->setActions(appContext->actions());
    exportViewModel->setFileSystemBrowser(appContext->fileSystemBrowser());
    exportViewModel->setSettings(settings);
    appContext->setExportViewModel(exportViewModel);

    auto *importViewModel = new ui::ImportViewModel(&w);
    importViewModel->setImportWorkflow(state->importWorkflow);
    importViewModel->setNavigation(appContext->navigation());
    importViewModel->setWorkspace(w.workspace());
    importViewModel->setSettings(settings);
    importViewModel->setActions(appContext->actions());
    importViewModel->setStatus(appContext->status());
    appContext->setImportViewModel(importViewModel);

    auto *settingsViewModel = new ui::SettingsViewModel(&w);
    settingsViewModel->setNavigation(appContext->navigation());
    settingsViewModel->setSettings(settings);
    settingsViewModel->setActions(appContext->actions());
    settingsViewModel->setLanguageService(languageService);
    appContext->setSettingsViewModel(settingsViewModel);
  }

  return Composition(std::move(state));
}

void wireAppStateToSession(
    MainWindow &w, const Composition &composition,
    core::ports::workspace::IWorkspaceWriter &workspaceWriter,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  workspaceWriter.setSnapshotChangedCallback(
      [&w, composition](
          const core::ports::workspace::WorkspaceSnapshot &snapshot) {
        if (w.workspace()) {
          w.workspace()->loadFromState(snapshot);
        }
        refreshComposition(composition);
      });

  workspaceWriter.setDeletionImpactCallback(
      [&w, errorReporter](const core::ports::workspace::DeletionImpact &impact) {
        try {
          if (w.workspace())
            w.workspace()->applyDeletionImpact(impact);
        } catch (...) {
          if (errorReporter)
            errorReporter->reportException(
                core::errors::ErrorSeverity::Error,
                "ui::shell::wireAppStateToSession::applyDeletionImpact",
                std::current_exception());
        }
      });
}

void refreshComposition(const Composition &composition) {
  if (!composition.state_) {
    return;
  }
  if (composition.state_->exportWorkflow) {
    composition.state_->exportWorkflow->refreshFromStateSnapshot();
  }
}

} // namespace ui::shell
