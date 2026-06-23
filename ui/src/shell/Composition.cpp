/**
 * @file ui/src/shell/Composition.cpp
 * @brief Wires QML shell workflows and view models.
 */

#include "ui/shell/Composition.h"

#include "MainWindow.h"
#include "core/ports/diagnostics/IErrorReporter.h"
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
#include "ui/viewmodels/ActorViewModel.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/viewmodels/AnnualViewModel.h"
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
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

#include <QApplication>
#include <QPointer>

#include <exception>
#include <utility>

namespace ui::shell {

void createComposition(
    QApplication& app, MainWindow& w,
    core::ports::workspace::IWorkspaceReader& workspaceReader,
    core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner) {
  auto* workspaceStore = w.workspaceStore();
  auto* workspaceCommands = w.workspaceCommands();
  auto* workspaceSelection = w.workspaceSelection();
  auto* workspaceSelectors = w.workspaceSelectors();

  if (workspaceStore) {
    workspaceStore->setWorkspacePorts(&workspaceWriter, &workspaceReader);
  }
  if (workspaceCommands) {
    workspaceCommands->setBeforeStorageSaveCallback({});
    QObject::connect(&w, &MainWindow::newFileRequested, workspaceCommands,
                     &ui::WorkspaceCommands::newFile);
    QObject::connect(&w, &MainWindow::openFileRequested, workspaceCommands,
                     &ui::WorkspaceCommands::openFile);
    QObject::connect(&w, &MainWindow::saveFileRequested, workspaceCommands,
                     &ui::WorkspaceCommands::saveFile);
    QObject::connect(&w, &MainWindow::saveFileAsRequested, workspaceCommands,
                     &ui::WorkspaceCommands::saveFileAs);
    QObject::connect(workspaceCommands,
                     &ui::WorkspaceCommands::operationSucceeded, &w,
                     &MainWindow::handleStorageOperationSucceeded);
    QObject::connect(workspaceCommands, &ui::WorkspaceCommands::operationFailed,
                     &w, &MainWindow::handleStorageOperationFailed);
  }

  const auto workspaceSnapshotProvider = [&workspaceReader]() {
    return workspaceReader.workspaceSnapshot();
  };

  auto analysisAdapter = std::make_shared<ui::adapters::AnalysisAdapter>(
      std::move(analysisRunner));
  auto* analysisWorkflow =
      new ui::AnalysisWorkflow(workspaceSnapshotProvider, analysisAdapter,
                               errorReporter, &w);

  auto annualAdapter =
      std::make_shared<ui::adapters::AnnualAdapter>(std::move(annualRunner));
  auto* annualWorkflow =
      new ui::AnnualWorkflow(workspaceSnapshotProvider, annualAdapter, &w);

  auto exportAdapter =
      std::make_shared<ui::adapters::ExportAdapter>(std::move(exportRunner));
  auto* exportWorkflow =
      new ui::ExportWorkflow(workspaceSnapshotProvider, exportAdapter,
                             errorReporter, &w);

  ui::LanguageService* languageService = nullptr;
  if (auto* appContext = w.appContext()) {
    languageService = appContext->languageService();
  }
  if (!languageService) {
    languageService = new ui::LanguageService(&app, w.qmlEngine(), &w);
  }
  if (auto* appContext = w.appContext())
    appContext->setLanguageService(languageService);

  ui::Settings* settings = w.settings();
  if (!settings) {
    settings = new ui::Settings(&w);
  }

  auto importAdapter =
      std::make_shared<ui::adapters::ImportAdapter>(std::move(importRunner));
  auto* importWorkflow = new ui::ImportWorkflow(
      importAdapter, errorReporter,
      [workspaceStore]() {
        return workspaceStore ? workspaceStore->snapshot()
                              : core::ports::workspace::WorkspaceSnapshot{};
      },
      workspaceCommands, workspaceSelectors, &w);
  if (workspaceCommands) {
    QPointer<ui::ImportWorkflow> activeImportWorkflow(importWorkflow);
    workspaceCommands->setBeforeStorageSaveCallback([activeImportWorkflow]() {
      if (activeImportWorkflow) {
        activeImportWorkflow->flushActiveDraftToWorkspace();
      }
    });
  }

  if (auto* appContext = w.appContext()) {
    auto* actorViewModel =
        new ui::ActorViewModel(workspaceStore, workspaceCommands,
                               workspaceSelection, workspaceSelectors, &w);
    appContext->setActorViewModel(actorViewModel);

    auto* bookingViewModel =
        new ui::BookingViewModel(workspaceStore, workspaceCommands,
                                 workspaceSelection, workspaceSelectors, &w);
    appContext->setBookingViewModel(bookingViewModel);

    auto* contractViewModel =
        new ui::ContractViewModel(workspaceStore, workspaceCommands,
                                  workspaceSelection, workspaceSelectors, &w);
    appContext->setContractViewModel(contractViewModel);

    auto* propertyViewModel =
        new ui::PropertyViewModel(workspaceStore, workspaceCommands,
                                  workspaceSelection, workspaceSelectors, &w);
    appContext->setPropertyViewModel(propertyViewModel);

    auto* analysisViewModel = new ui::AnalysisViewModel(&w);
    analysisViewModel->setWorkspaceRoles(workspaceStore, workspaceCommands,
                                         workspaceSelection,
                                         workspaceSelectors);
    analysisViewModel->setAnalysisWorkflow(analysisWorkflow);
    analysisViewModel->setSettings(settings);
    appContext->setAnalysisViewModel(analysisViewModel);

    auto* annualViewModel = new ui::AnnualViewModel(&w);
    annualViewModel->setWorkspaceRoles(workspaceStore, workspaceCommands,
                                       workspaceSelection, workspaceSelectors);
    annualViewModel->setAnnualWorkflow(annualWorkflow);
    appContext->setAnnualViewModel(annualViewModel);

    auto* exportViewModel = new ui::ExportViewModel(&w);
    exportViewModel->setWorkspaceRoles(workspaceStore, workspaceCommands,
                                       workspaceSelectors);
    exportViewModel->setExportWorkflow(exportWorkflow);
    exportViewModel->setActions(appContext->actions());
    exportViewModel->setFileSystemBrowser(appContext->fileSystemBrowser());
    exportViewModel->setSettings(settings);
    appContext->setExportViewModel(exportViewModel);

    auto* importViewModel = new ui::ImportViewModel(&w);
    importViewModel->setImportWorkflow(importWorkflow);
    importViewModel->setNavigation(appContext->navigation());
    importViewModel->setWorkspaceRoles(workspaceStore, workspaceCommands,
                                       workspaceSelection, workspaceSelectors);
    importViewModel->setSettings(settings);
    importViewModel->setActions(appContext->actions());
    importViewModel->setStatus(appContext->status());
    appContext->setImportViewModel(importViewModel);

    auto* settingsViewModel = new ui::SettingsViewModel(&w);
    settingsViewModel->setNavigation(appContext->navigation());
    settingsViewModel->setSettings(settings);
    settingsViewModel->setActions(appContext->actions());
    settingsViewModel->setLanguageService(languageService);
    appContext->setSettingsViewModel(settingsViewModel);
  }
}

void wireWorkspaceCallbacks(
    MainWindow& w, core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& errorReporter) {
  workspaceWriter.setSnapshotChangedCallback(
      [&w](const core::ports::workspace::WorkspaceSnapshot& snapshot) {
        if (w.workspaceStore()) {
          w.workspaceStore()->loadFromState(snapshot);
        }
      });

  workspaceWriter.setDeletionImpactCallback(
      [&w,
       errorReporter](const core::ports::workspace::DeletionImpact& impact) {
        try {
          if (w.workspaceStore())
            w.workspaceStore()->applyDeletionImpact(impact);
        } catch (...) {
          if (errorReporter)
            errorReporter->reportException(
                core::errors::ErrorSeverity::Error,
                "ui::shell::wireWorkspaceCallbacks::applyDeletionImpact",
                std::current_exception());
        }
      });
}

} // namespace ui::shell
