/**
 * @file ui/src/bootstrap/QmlRuntime.cpp
 * @brief Implementation of the UI QmlRuntime component.
 */

#include "ui/shell/QmlRuntime.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QQmlEngine>
#include <qqml.h>

#include "ui/shell/QmlContracts.h"
#include "ui/shared/config/Defaults.h"
#include "ui/state/export/ExportState.h"
#include "ui/state/session/AnalysisState.h"
#include "ui/state/session/AnnualState.h"
#include "ui/state/import/ImportState.h"
#include "ui/state/import/StatementDraftState.h"
#include "ui/state/import/TransactionDraftState.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/state/settings/SettingsState.h"

namespace ui::bootstrap {

void registerTypes()
{
    // Idempotent registration of QML-exposed types and metaobjects.
    static bool registered = false;
    if (registered) return;

    qmlRegisterUncreatableType<ui::NavigationState>(ui::qml::contracts::module::kName,
                                                    ui::qml::contracts::module::kMajorVersion,
                                                    ui::qml::contracts::module::kMinorVersion,
                                                    ui::qml::contracts::module::kNavigationTypeName,
                                                    ui::qml::contracts::module::kNavigationTypeDescription);
    qmlRegisterUncreatableMetaObject(ui::qml::contracts::staticMetaObject,
                                     ui::qml::contracts::module::kName,
                                     ui::qml::contracts::module::kMajorVersion,
                                     ui::qml::contracts::module::kMinorVersion,
                                     ui::qml::contracts::module::kQmlContractsTypeName,
                                     ui::qml::contracts::module::kQmlContractsTypeDescription);
    qmlRegisterType<ui::ImportState>(ui::qml::contracts::module::kName,
                                     ui::qml::contracts::module::kMajorVersion,
                                     ui::qml::contracts::module::kMinorVersion,
                                     "ImportState");
    qmlRegisterType<ui::AnalysisState>(ui::qml::contracts::module::kName,
                                       ui::qml::contracts::module::kMajorVersion,
                                       ui::qml::contracts::module::kMinorVersion,
                                       "AnalysisState");
    qmlRegisterType<ui::AnnualState>(ui::qml::contracts::module::kName,
                                     ui::qml::contracts::module::kMajorVersion,
                                     ui::qml::contracts::module::kMinorVersion,
                                     "AnnualState");
    qmlRegisterType<ui::ExportState>(ui::qml::contracts::module::kName,
                                     ui::qml::contracts::module::kMajorVersion,
                                     ui::qml::contracts::module::kMinorVersion,
                                     "ExportState");
    qmlRegisterType<ui::SettingsState>(ui::qml::contracts::module::kName,
                                       ui::qml::contracts::module::kMajorVersion,
                                       ui::qml::contracts::module::kMinorVersion,
                                       "SettingsState");
    qmlRegisterType<ui::StatementDraftState>(ui::qml::contracts::module::kName,
                                             ui::qml::contracts::module::kMajorVersion,
                                             ui::qml::contracts::module::kMinorVersion,
                                             "StatementDraftState");
    qmlRegisterType<ui::TransactionDraftState>(ui::qml::contracts::module::kName,
                                               ui::qml::contracts::module::kMajorVersion,
                                               ui::qml::contracts::module::kMinorVersion,
                                               "TransactionDraftState");
    registered = true;
}

void configureRuntime(QQmlEngine* engine)
{
    if (!engine) return;

    const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
        engine->addImportPath(qtImports);
    }

    const QString appQmlDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kAppQmlDirName;
    if (QDir(appQmlDir).exists()) {
        engine->addImportPath(appQmlDir);
    }

    const QString imageFormatsDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kImageFormatsDirName;
    if (QDir(imageFormatsDir).exists()) {
        QCoreApplication::addLibraryPath(imageFormatsDir);
    }
}

} // namespace ui::bootstrap
