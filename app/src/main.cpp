/**
 * @file app/src/main.cpp
 * @brief Initializes the desktop application and shared runtime infrastructure.
 */

#include "Environment.h"
#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"
#include "archive/ZipArchiveAdapter.h"
#include "core/application/analysis/AnalysisService.h"
#include "core/application/annual/AnnualService.h"
#include "core/application/export/ExportService.h"
#include "core/application/import/IImportStatement.h"
#include "core/application/import/StatementImportRunner.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/analysis/IAnalysisRunner.h"
#include "core/ports/annual/IAnnualRunner.h"
#include "core/ports/export/IExportRunner.h"
#include "core/ports/image-processing/IImageProcessor.h"
#include "core/ports/import/IImportRunner.h"
#include "core/ports/pdf-rendering/IPdfRenderer.h"
#include "core/ports/text-recognition/ITextRecognizer.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include <QApplication>
#include <QByteArray>
#include <QIcon>
#include <QMessageBox>
#include <QObject>
#include <QQuickStyle>

#include "core/constants/app.h"
#include "core/constants/preferences.h"
#include "core/constants/runtime.h"
#include "persistence/Factory.h"
#include "persistence/WorkspaceStateStore.h"

#include "core/application/storage/StorageManager.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "debug/DebugDefaults.h"
#include "debug/ErrorReporter.h"
#include "debug/FileDebugger.h"
#include "debug/IDebugger.h"
#include "ui/shared/config/Defaults.h"
#include "ui/shared/observability/ErrorCodes.h"
#include "xlsx-writer/XlntTableWriterAdapter.h"

#include <QDir>
#include <QStandardPaths>
#include <cstdio>
#include <filesystem>

std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer>
createPdfRendererAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<core::ports::image_processing::IImageProcessor>
createImageProcessorAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<core::ports::text_recognition::ITextRecognizer>
createTextRecognizerAdapter(std::shared_ptr<IDebugger> dbg);

namespace {

void ensureParentDirectoryExists(const std::filesystem::path &path,
                                 const char *origin) {
  try {
    if (path.has_parent_path())
      std::filesystem::create_directories(path.parent_path());
  } catch (...) {
    core::errors::reportException(core::errors::ErrorSeverity::Warning, origin,
                                  std::current_exception());
  }
}
/**
 * @brief Global Qt message handler that redirects Qt logging to stderr with
 * context.
 */
static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context,
                             const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  const char *file = context.file ? context.file : "";
  const char *function = context.function ? context.function : "";
  const std::string text = std::string(localMsg.constData()) + " (" + file +
                           ":" + std::to_string(context.line) + ", " +
                           function + ")";
  const core::errors::ErrorContext ctx = {
      {"file", file},
      {"line", std::to_string(context.line)},
      {"function", function}};
  switch (type) {
  case QtDebugMsg:
    core::errors::report(core::errors::ErrorSeverity::Info,
                         ui::observability::codes::QtDebug,
                         "app::qtMessageHandler", text, ctx);
    break;
  case QtInfoMsg:
    core::errors::report(core::errors::ErrorSeverity::Info,
                         ui::observability::codes::QtInfo,
                         "app::qtMessageHandler", text, ctx);
    break;
  case QtWarningMsg:
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         ui::observability::codes::QtWarning,
                         "app::qtMessageHandler", text, ctx);
    break;
  case QtCriticalMsg:
    core::errors::report(core::errors::ErrorSeverity::Error,
                         ui::observability::codes::QtCritical,
                         "app::qtMessageHandler", text, ctx);
    break;
  case QtFatalMsg:
    core::errors::report(core::errors::ErrorSeverity::Critical,
                         ui::observability::codes::QtFatal,
                         "app::qtMessageHandler", text, ctx);
    abort();
  }
}

} // namespace

#ifdef USE_QML
/**
 * @brief Start the QML application UI.
 *
 * Implemented in `main_qml.cpp`. Only available when built with USE_QML.
 */
extern int startQmlApp(QApplication &app,
                       core::ports::workspace::IWorkspaceReader &workspaceReader,
                       core::ports::workspace::IWorkspaceWriter &workspaceWriter,
                       std::shared_ptr<core::errors::IErrorReporter>
                           errorReporter,
                       std::shared_ptr<core::ports::analysis::IAnalysisRunner>
                           analysisRunner,
                       std::shared_ptr<core::ports::annual::IAnnualRunner>
                           annualRunner,
                       std::shared_ptr<core::ports::exporting::IExportRunner>
                           exportRunner,
                       std::shared_ptr<core::ports::importing::IImportRunner>
                           importRunner);
#endif

int main(int argc, char *argv[]) {
  auto errorReporter = debug::createDefaultErrorReporter();
  core::errors::setGlobalErrorReporter(errorReporter);

#if defined(_DEBUG)
  if (qEnvironmentVariableIsEmpty("QT_ACCESSIBILITY")) {
    qputenv("QT_ACCESSIBILITY", QByteArrayLiteral("0"));
  }
#endif

  // Install global Qt message handler early so startup logs are captured
  const auto previousQtMessageHandler =
      qInstallMessageHandler(qtMessageHandler);

  // Load runtime environment from .env if present
  app::runtime::loadDotEnv(".env", false);

  // Ensure Qt Quick Controls uses a non-native style that supports
  // customization Call before creating the QApplication/QGuiApplication
  QQuickStyle::setStyle(core::constants::runtime::kQtStyle.data());

  // Create the Qt application (manages event loop and GUI resources)
  QApplication app(argc, argv);
  app.setStyle(core::constants::runtime::kQtStyle.data());
  app.setOrganizationName(QString::fromLatin1(
      core::constants::preferences::kOrganizationName.data()));
  app.setApplicationName(QString::fromLatin1(
      core::constants::preferences::kApplicationName.data()));
  app.setWindowIcon(QIcon(ui::config::kAppIconResource));

  // Setup storage manager and controller (manages application state files)
  const QString appDataLocation =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  const std::filesystem::path appDataRoot =
      appDataLocation.isEmpty()
          ? std::filesystem::path(QDir::homePath().toStdString()) /
                std::string(core::constants::runtime::kAppDataDirectoryName)
          : std::filesystem::path(appDataLocation.toStdString());

  const std::filesystem::path defaultDbPath =
      appDataRoot / std::string(core::constants::runtime::kDatabaseFileName);
  const std::filesystem::path registryDbPath =
      appDataRoot / std::string(core::constants::runtime::kRegistryFileName);
  ensureParentDirectoryExists(defaultDbPath,
                              "app::main::createConfigDirectory");
  ensureParentDirectoryExists(registryDbPath,
                              "app::main::createRegistryDirectory");

  std::shared_ptr<core::ports::storage::IRegistry> registry;
  try {
    registry = createSqliteRegistry(registryDbPath.string());
  } catch (const std::exception &ex) {
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::ConfigDbOpenFailed,
                         "app::main::openRegistryDb",
                         std::string("failed to open registry DB '") +
                             registryDbPath.string() + "': " + ex.what(),
                         {{"path", registryDbPath.string()}});
  }

  core::storage::StorageManager sm(registry);

  auto smPtr = std::make_unique<core::storage::StorageManager>(std::move(sm));
  core::application::WorkspaceFacade appStateFacade(std::move(smPtr));

  appStateFacade.setErrorReporter(errorReporter);
  appStateFacade.setAtomicStoreLoad([](const std::string &dbPath) {
    auto db = createSqliteDb(dbPath);
    WorkspaceStateStore store(db);
    return store.load();
  });
  appStateFacade.setAtomicStoreSave(
      [](const std::string &dbPath,
         const core::application::workspace::WorkspaceSessionState &document) {
        auto db = createSqliteDb(dbPath);
        WorkspaceStateStore store(db);
        return store.save(document);
      });

  try {
    appStateFacade.openLatest();
  } catch (const std::exception &ex) {
    core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                  "app::main::openLatest",
                                  std::current_exception());
    // continue with empty state
  }

  // Only create a new file if no path was found AND the loaded state is empty.
  // This avoids accidentally overwriting a valid loaded state due to
  // registry or ordering issues at startup.
  if (appStateFacade.currentPath().empty() && appStateFacade.state().empty()) {
    appStateFacade.newFile(defaultDbPath.string());
  }

  // Ensure Qt finds deployed plugins and QML modules next to the executable
  QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
  // Delegate to QML-specific startup
  try {
    auto analysisRunner =
        std::make_shared<core::application::analysis::AnalysisService>(
            std::make_shared<infra::analysis_image_renderer::
                                 OpenCvAnalysisImageRendererAdapter>());
    auto annualRunner =
        std::make_shared<core::application::annual::AnnualService>();
    auto exportRunner =
        std::make_shared<core::application::exporting::ExportService>(
            std::make_shared<infra::archive::ZipArchiveAdapter>(),
            std::make_shared<infra::xlsx_writer::XlntTableWriterAdapter>(),
            std::make_shared<infra::analysis_image_renderer::
                                 OpenCvAnalysisImageRendererAdapter>());

    auto importDebugger = std::make_shared<FileDebugger>(
        "", std::string(debug::defaults::kImportProcessName));
    auto importService = core::application::importing::createImportStatement(
        createPdfRendererAdapter(importDebugger),
        createImageProcessorAdapter(importDebugger),
        createTextRecognizerAdapter(importDebugger), errorReporter);
    const auto importRunBasePath =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            .toStdString();
    auto importRunner =
        std::make_shared<core::application::importing::StatementImportRunner>(
            importService, importRunBasePath, errorReporter);

    const int exitCode =
        startQmlApp(app, appStateFacade, appStateFacade,
                    errorReporter,
                    std::move(analysisRunner), std::move(annualRunner),
                    std::move(exportRunner), std::move(importRunner));
    qInstallMessageHandler(previousQtMessageHandler);
    appStateFacade.setErrorReporter({});
    core::errors::setGlobalErrorReporter({});
    return exitCode;
  } catch (const std::exception &ex) {
    qInstallMessageHandler(previousQtMessageHandler);
    core::errors::reportException(core::errors::ErrorSeverity::Critical,
                                  "app::main::startQmlApp",
                                  std::current_exception());
    QMessageBox::critical(
        nullptr, QObject::tr("Fatal error"),
        QObject::tr("Startup failed: %1").arg(QString::fromUtf8(ex.what())));
    appStateFacade.setErrorReporter({});
    core::errors::setGlobalErrorReporter({});
    return -1;
  } catch (...) {
    qInstallMessageHandler(previousQtMessageHandler);
    core::errors::reportException(core::errors::ErrorSeverity::Critical,
                                  "app::main::startQmlAppUnknown",
                                  std::current_exception());
    QMessageBox::critical(nullptr, QObject::tr("Fatal error"),
                          QObject::tr("Startup failed: unknown exception"));
    appStateFacade.setErrorReporter({});
    core::errors::setGlobalErrorReporter({});
    return -2;
  }
#else
  // No UI available in this build configuration
  qInstallMessageHandler(previousQtMessageHandler);
  return 0;
#endif
}
