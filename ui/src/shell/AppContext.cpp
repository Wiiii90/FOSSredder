/**
 * @file ui/src/shell/AppContext.cpp
 * @brief Implements the QML application context for the UI boundary.
 */

#include "ui/shell/AppContext.h"

namespace ui::bootstrap {

AppContext::AppContext(QObject *parent) : QObject(parent) {}

namespace {

template <typename TSignal, typename TValue>
void assignIfChanged(TValue *&target, TValue *value, AppContext *self,
                     TSignal signal) {
  if (target == value)
    return;
  target = value;
  emit(self->*signal)();
}

} // namespace

void AppContext::setActions(ui::Actions *value) {
  assignIfChanged(actions_, value, this, &AppContext::actionsChanged);
}

void AppContext::setNavigation(ui::NavigationState *value) {
  assignIfChanged(navigation_, value, this, &AppContext::navigationChanged);
}

void AppContext::setWorkspaceFacade(ui::WorkspaceFacade *value) {
  assignIfChanged(workspaceFacade_, value, this,
                  &AppContext::workspaceFacadeChanged);
}

void AppContext::setFileSystemBrowser(ui::FileSystemBrowser *value) {
  assignIfChanged(fileSystemBrowser_, value, this,
                  &AppContext::fileSystemBrowserChanged);
}

void AppContext::setStatus(ui::StatusState *value) {
  assignIfChanged(status_, value, this, &AppContext::statusChanged);
}

void AppContext::setActorViewModel(ui::ActorViewModel *value) {
  assignIfChanged(actorViewModel_, value, this,
                  &AppContext::actorViewModelChanged);
}

void AppContext::setBookingViewModel(ui::BookingViewModel *value) {
  assignIfChanged(bookingViewModel_, value, this,
                  &AppContext::bookingViewModelChanged);
}

void AppContext::setContractViewModel(ui::ContractViewModel *value) {
  assignIfChanged(contractViewModel_, value, this,
                  &AppContext::contractViewModelChanged);
}

void AppContext::setPropertyViewModel(ui::PropertyViewModel *value) {
  assignIfChanged(propertyViewModel_, value, this,
                  &AppContext::propertyViewModelChanged);
}

void AppContext::setAnalysisViewModel(ui::AnalysisViewModel *value) {
  assignIfChanged(analysisViewModel_, value, this,
                  &AppContext::analysisViewModelChanged);
}

void AppContext::setAnnualViewModel(ui::AnnualViewModel *value) {
  assignIfChanged(annualViewModel_, value, this, &AppContext::annualViewModelChanged);
}

void AppContext::setExportViewModel(ui::ExportViewModel *value) {
  assignIfChanged(exportViewModel_, value, this, &AppContext::exportViewModelChanged);
}

void AppContext::setImportViewModel(ui::ImportViewModel *value) {
  assignIfChanged(importViewModel_, value, this, &AppContext::importViewModelChanged);
}

void AppContext::setSettingsViewModel(ui::SettingsViewModel *value) {
  assignIfChanged(settingsViewModel_, value, this,
                  &AppContext::settingsViewModelChanged);
}

void AppContext::setLanguageService(ui::LanguageService *value) {
  assignIfChanged(languageService_, value, this,
                  &AppContext::languageServiceChanged);
}

void AppContext::setIsDebugBuild(bool value) {
  if (isDebugBuild_ == value)
    return;
  isDebugBuild_ = value;
  emit isDebugBuildChanged();
}

} // namespace ui::bootstrap
