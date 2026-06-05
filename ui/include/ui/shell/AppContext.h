/**
 * @file ui/include/ui/shell/AppContext.h
 * @brief Declares the QML application context object.
 */

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

namespace ui::bootstrap {

class AppContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      ui::Actions *actions READ actions WRITE setActions NOTIFY actionsChanged)
  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY navigationChanged)
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
  /**
   * @brief Creates the QML application context.
   * @param parent Optional Qt parent.
   */
  explicit AppContext(QObject *parent = nullptr);

  ui::Actions *actions() const noexcept { return actions_; }
  ui::NavigationState *navigation() const noexcept { return navigation_; }
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

  /**
   * @brief Sets shell actions exposed to QML.
   * @param value Actions object or nullptr.
   */
  void setActions(ui::Actions *value);
  /**
   * @brief Sets navigation state exposed to QML.
   * @param value Navigation object or nullptr.
   */
  void setNavigation(ui::NavigationState *value);
  /**
   * @brief Sets filesystem browsing service exposed to QML.
   * @param value File system browser or nullptr.
   */
  void setFileSystemBrowser(ui::FileSystemBrowser *value);
  /**
   * @brief Sets status state exposed to QML.
   * @param value Status state or nullptr.
   */
  void setStatus(ui::StatusState *value);
  /**
   * @brief Sets actor view model exposed to QML.
   * @param value Actor view model or nullptr.
   */
  void setActorViewModel(ui::ActorViewModel *value);
  /**
   * @brief Sets booking view model exposed to QML.
   * @param value Booking view model or nullptr.
   */
  void setBookingViewModel(ui::BookingViewModel *value);
  /**
   * @brief Sets contract view model exposed to QML.
   * @param value Contract view model or nullptr.
   */
  void setContractViewModel(ui::ContractViewModel *value);
  /**
   * @brief Sets property view model exposed to QML.
   * @param value Property view model or nullptr.
   */
  void setPropertyViewModel(ui::PropertyViewModel *value);
  /**
   * @brief Sets analysis view model exposed to QML.
   * @param value Analysis view model or nullptr.
   */
  void setAnalysisViewModel(ui::AnalysisViewModel *value);
  /**
   * @brief Sets annual view model exposed to QML.
   * @param value Annual view model or nullptr.
   */
  void setAnnualViewModel(ui::AnnualViewModel *value);
  /**
   * @brief Sets export view model exposed to QML.
   * @param value Export view model or nullptr.
   */
  void setExportViewModel(ui::ExportViewModel *value);
  /**
   * @brief Sets import view model exposed to QML.
   * @param value Import view model or nullptr.
   */
  void setImportViewModel(ui::ImportViewModel *value);
  /**
   * @brief Sets settings view model exposed to QML.
   * @param value Settings view model or nullptr.
   */
  void setSettingsViewModel(ui::SettingsViewModel *value);
  /**
   * @brief Sets language service exposed to QML.
   * @param value Language service or nullptr.
   */
  void setLanguageService(ui::LanguageService *value);
  /**
   * @brief Sets whether the running binary is a debug build.
   * @param value True for debug builds.
   */
  void setIsDebugBuild(bool value);

signals:
  /**
   * @brief Emitted when actions changed.
   */
  void actionsChanged();
  /**
   * @brief Emitted when navigation changed.
   */
  void navigationChanged();
  /**
   * @brief Emitted when filesystem browser changed.
   */
  void fileSystemBrowserChanged();
  /**
   * @brief Emitted when status state changed.
   */
  void statusChanged();
  /**
   * @brief Emitted when actor view model changed.
   */
  void actorViewModelChanged();
  /**
   * @brief Emitted when booking view model changed.
   */
  void bookingViewModelChanged();
  /**
   * @brief Emitted when contract view model changed.
   */
  void contractViewModelChanged();
  /**
   * @brief Emitted when property view model changed.
   */
  void propertyViewModelChanged();
  /**
   * @brief Emitted when analysis view model changed.
   */
  void analysisViewModelChanged();
  /**
   * @brief Emitted when annual view model changed.
   */
  void annualViewModelChanged();
  /**
   * @brief Emitted when export view model changed.
   */
  void exportViewModelChanged();
  /**
   * @brief Emitted when import view model changed.
   */
  void importViewModelChanged();
  /**
   * @brief Emitted when settings view model changed.
   */
  void settingsViewModelChanged();
  /**
   * @brief Emitted when language service changed.
   */
  void languageServiceChanged();
  /**
   * @brief Emitted when debug build flag changed.
   */
  void isDebugBuildChanged();

private:
  ui::Actions *actions_ = nullptr;
  ui::NavigationState *navigation_ = nullptr;
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
