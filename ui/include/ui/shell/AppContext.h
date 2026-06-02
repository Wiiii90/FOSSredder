#pragma once

#include <QObject>

#include "ui/platform/FileSystemBrowser.h"
#include "ui/platform/LanguageService.h"
#include "ui/shell/AppActions.h"
#include "ui/viewmodels/ActorViewModel.h"
#include "ui/viewmodels/BookingViewModel.h"
#include "ui/viewmodels/ContractViewModel.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/shell/NavigationState.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/viewmodels/AnnualViewModel.h"
#include "ui/viewmodels/PropertyViewModel.h"
#include "ui/viewmodels/SettingsViewModel.h"
#include "ui/shell/StatusState.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui::bootstrap {

class AppContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      ui::Actions *actions READ actions WRITE setActions NOTIFY actionsChanged)
  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY navigationChanged)
  Q_PROPERTY(ui::WorkspaceFacade *workspaceFacade READ workspaceFacade WRITE
                 setWorkspaceFacade NOTIFY workspaceFacadeChanged)
  Q_PROPERTY(ui::FileSystemBrowser *fileSystemBrowser READ fileSystemBrowser
                 WRITE setFileSystemBrowser NOTIFY fileSystemBrowserChanged)
  Q_PROPERTY(
      ui::StatusState *status READ status WRITE setStatus NOTIFY statusChanged)
  Q_PROPERTY(ui::ActorViewModel *actorViewModel READ actorViewModel WRITE
                 setActorViewModel NOTIFY actorViewModelChanged)
  Q_PROPERTY(ui::BookingViewModel *bookingViewModel READ bookingViewModel WRITE
                 setBookingViewModel NOTIFY bookingViewModelChanged)
  Q_PROPERTY(ui::ContractViewModel *contractViewModel READ contractViewModel
                 WRITE setContractViewModel NOTIFY contractViewModelChanged)
  Q_PROPERTY(ui::PropertyViewModel *propertyViewModel READ propertyViewModel
                 WRITE setPropertyViewModel NOTIFY propertyViewModelChanged)
  Q_PROPERTY(ui::AnalysisViewModel *analysisViewModel READ analysisViewModel WRITE
                 setAnalysisViewModel NOTIFY analysisViewModelChanged)
  Q_PROPERTY(ui::AnnualViewModel *annualViewModel READ annualViewModel WRITE
                 setAnnualViewModel NOTIFY annualViewModelChanged)
  Q_PROPERTY(ui::ExportViewModel *exportViewModel READ exportViewModel WRITE
                 setExportViewModel NOTIFY exportViewModelChanged)
  Q_PROPERTY(ui::ImportViewModel *importViewModel READ importViewModel WRITE setImportViewModel
                 NOTIFY importViewModelChanged)
  Q_PROPERTY(ui::SettingsViewModel *settingsViewModel READ settingsViewModel WRITE
                 setSettingsViewModel NOTIFY settingsViewModelChanged)
  Q_PROPERTY(ui::LanguageService *languageService READ languageService WRITE
                 setLanguageService NOTIFY languageServiceChanged)
  Q_PROPERTY(bool isDebugBuild READ isDebugBuild WRITE setIsDebugBuild NOTIFY
                 isDebugBuildChanged)

public:
  explicit AppContext(QObject *parent = nullptr);

  ui::Actions *actions() const noexcept { return actions_; }
  ui::NavigationState *navigation() const noexcept { return navigation_; }
  ui::WorkspaceFacade *workspaceFacade() const noexcept {
    return workspaceFacade_;
  }
  ui::FileSystemBrowser *fileSystemBrowser() const noexcept {
    return fileSystemBrowser_;
  }
  ui::StatusState *status() const noexcept { return status_; }
  ui::ActorViewModel *actorViewModel() const noexcept {
    return actorViewModel_;
  }
  ui::BookingViewModel *bookingViewModel() const noexcept {
    return bookingViewModel_;
  }
  ui::ContractViewModel *contractViewModel() const noexcept {
    return contractViewModel_;
  }
  ui::PropertyViewModel *propertyViewModel() const noexcept {
    return propertyViewModel_;
  }
  ui::AnalysisViewModel *analysisViewModel() const noexcept {
    return analysisViewModel_;
  }
  ui::AnnualViewModel *annualViewModel() const noexcept {
    return annualViewModel_;
  }
  ui::ExportViewModel *exportViewModel() const noexcept {
    return exportViewModel_;
  }
  ui::ImportViewModel *importViewModel() const noexcept {
    return importViewModel_;
  }
  ui::SettingsViewModel *settingsViewModel() const noexcept {
    return settingsViewModel_;
  }
  ui::LanguageService *languageService() const noexcept {
    return languageService_;
  }
  bool isDebugBuild() const noexcept { return isDebugBuild_; }

  void setActions(ui::Actions *value);
  void setNavigation(ui::NavigationState *value);
  void setWorkspaceFacade(ui::WorkspaceFacade *value);
  void setFileSystemBrowser(ui::FileSystemBrowser *value);
  void setStatus(ui::StatusState *value);
  void setActorViewModel(ui::ActorViewModel *value);
  void setBookingViewModel(ui::BookingViewModel *value);
  void setContractViewModel(ui::ContractViewModel *value);
  void setPropertyViewModel(ui::PropertyViewModel *value);
  void setAnalysisViewModel(ui::AnalysisViewModel *value);
  void setAnnualViewModel(ui::AnnualViewModel *value);
  void setExportViewModel(ui::ExportViewModel *value);
  void setImportViewModel(ui::ImportViewModel *value);
  void setSettingsViewModel(ui::SettingsViewModel *value);
  void setLanguageService(ui::LanguageService *value);
  void setIsDebugBuild(bool value);

signals:
  void actionsChanged();
  void navigationChanged();
  void workspaceFacadeChanged();
  void fileSystemBrowserChanged();
  void statusChanged();
  void actorViewModelChanged();
  void bookingViewModelChanged();
  void contractViewModelChanged();
  void propertyViewModelChanged();
  void analysisViewModelChanged();
  void annualViewModelChanged();
  void exportViewModelChanged();
  void importViewModelChanged();
  void settingsViewModelChanged();
  void languageServiceChanged();
  void isDebugBuildChanged();

private:
  ui::Actions *actions_ = nullptr;
  ui::NavigationState *navigation_ = nullptr;
  ui::WorkspaceFacade *workspaceFacade_ = nullptr;
  ui::FileSystemBrowser *fileSystemBrowser_ = nullptr;
  ui::StatusState *status_ = nullptr;
  ui::ActorViewModel *actorViewModel_ = nullptr;
  ui::BookingViewModel *bookingViewModel_ = nullptr;
  ui::ContractViewModel *contractViewModel_ = nullptr;
  ui::PropertyViewModel *propertyViewModel_ = nullptr;
  ui::AnalysisViewModel *analysisViewModel_ = nullptr;
  ui::AnnualViewModel *annualViewModel_ = nullptr;
  ui::ExportViewModel *exportViewModel_ = nullptr;
  ui::ImportViewModel *importViewModel_ = nullptr;
  ui::SettingsViewModel *settingsViewModel_ = nullptr;
  ui::LanguageService *languageService_ = nullptr;
  bool isDebugBuild_ = false;
};

} // namespace ui::bootstrap
