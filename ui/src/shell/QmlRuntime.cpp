/**
 * @file ui/src/shell/QmlRuntime.cpp
 * @brief Implementation of the UI QmlRuntime component.
 */

#include "ui/shell/QmlRuntime.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QQmlEngine>
#include <qqml.h>

#include "ui/platform/FileSystemBrowser.h"
#include "ui/platform/LanguageService.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/Defaults.h"
#include "ui/shell/Navigation.h"
#include "ui/shell/QmlContracts.h"
#include "ui/shell/Status.h"
#include "ui/viewmodels/ActorViewModel.h"
#include "ui/viewmodels/AnalysisViewModel.h"
#include "ui/viewmodels/AnnualViewModel.h"
#include "ui/viewmodels/BookingViewModel.h"
#include "ui/viewmodels/ContractViewModel.h"
#include "ui/viewmodels/ExportViewModel.h"
#include "ui/viewmodels/ImportViewModel.h"
#include "ui/viewmodels/PropertyViewModel.h"
#include "ui/viewmodels/SettingsViewModel.h"

namespace ui::bootstrap {

namespace {

template<typename T>
void registerContextObjectType(const char* name) {
  qmlRegisterUncreatableType<T>(ui::qml::contracts::module::kName,
                                ui::qml::contracts::module::kMajorVersion,
                                ui::qml::contracts::module::kMinorVersion, name,
                                "This type is provided by AppContext");
}

} // namespace

void registerTypes() {
  // Idempotent registration of QML-exposed types and metaobjects.
  static bool registered = false;
  if (registered)
    return;

  qmlRegisterUncreatableType<ui::Navigation>(
      ui::qml::contracts::module::kName,
      ui::qml::contracts::module::kMajorVersion,
      ui::qml::contracts::module::kMinorVersion,
      ui::qml::contracts::module::kNavigationTypeName,
      ui::qml::contracts::module::kNavigationTypeDescription);
  registerContextObjectType<ui::Actions>("Actions");
  registerContextObjectType<ui::FileSystemBrowser>("FileSystemBrowser");
  registerContextObjectType<ui::LanguageService>("LanguageService");
  registerContextObjectType<ui::Status>("Status");
  registerContextObjectType<ui::ActorViewModel>("ActorViewModel");
  registerContextObjectType<ui::BookingViewModel>("BookingViewModel");
  registerContextObjectType<ui::ContractViewModel>("ContractViewModel");
  registerContextObjectType<ui::PropertyViewModel>("PropertyViewModel");
  registerContextObjectType<ui::AnalysisViewModel>("AnalysisViewModel");
  registerContextObjectType<ui::AnnualViewModel>("AnnualViewModel");
  registerContextObjectType<ui::ExportViewModel>("ExportViewModel");
  registerContextObjectType<ui::ImportViewModel>("ImportViewModel");
  registerContextObjectType<ui::SettingsViewModel>("SettingsViewModel");
  qmlRegisterUncreatableMetaObject(
      ui::qml::contracts::staticMetaObject, ui::qml::contracts::module::kName,
      ui::qml::contracts::module::kMajorVersion,
      ui::qml::contracts::module::kMinorVersion,
      ui::qml::contracts::module::kQmlContractsTypeName,
      ui::qml::contracts::module::kQmlContractsTypeDescription);
  registered = true;
}

void configureRuntime(QQmlEngine* engine) {
  if (!engine)
    return;

  const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
  if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
    engine->addImportPath(qtImports);
  }

  const QString appQmlDir = QCoreApplication::applicationDirPath() +
                            QLatin1Char('/') + ui::config::kAppQmlDirName;
  if (QDir(appQmlDir).exists()) {
    engine->addImportPath(appQmlDir);
  }

  const QString appDir = QCoreApplication::applicationDirPath();
  if (QDir(appDir).exists()) {
    engine->addImportPath(appDir);
  }

  const QString imageFormatsDir = QCoreApplication::applicationDirPath() +
                                  QLatin1Char('/') +
                                  ui::config::kImageFormatsDirName;
  if (QDir(imageFormatsDir).exists()) {
    QCoreApplication::addLibraryPath(imageFormatsDir);
  }
}

} // namespace ui::bootstrap
