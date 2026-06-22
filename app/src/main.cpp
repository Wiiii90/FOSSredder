/**
 * @file app/src/main.cpp
 * @brief Initializes the desktop application and shared runtime infrastructure.
 */

#include "analysis-rendering/OpenCvAnalysisRendererAdapter.h"
#include "archive/ZipArchiveAdapter.h"
#include "core/application/analysis/AnalysisService.h"
#include "core/application/annual/AnnualService.h"
#include "core/application/export/ExportService.h"
#include "core/application/import/IImportStatement.h"
#include "core/application/import/StatementImportRunner.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/ports/infra/document-image-processing/IDocumentImageProcessor.h"
#include "core/ports/infra/pdf-rendering/IPdfRenderer.h"
#include "core/ports/infra/text-recognition/ITextRecognizer.h"
#include "core/ports/usecases/analysis/IAnalysisRunner.h"
#include "core/ports/usecases/annual/IAnnualRunner.h"
#include "core/ports/usecases/export/IExportRunner.h"
#include "core/ports/usecases/import/IImportRunner.h"
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
#include "core/errors/ErrorReporting.h"
#include "core/ports/diagnostics/IDiagnostics.h"
#include "diagnostics/DiagnosticsDefaults.h"
#include "diagnostics/ErrorReporter.h"
#include "diagnostics/FileDiagnostics.h"
#include "ui/shell/Defaults.h"
#include "xlsx-writer/XlntTableWriterAdapter.h"

#include <QDir>
#include <QStandardPaths>
#include <QtGlobal>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string_view>

std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer>
createPdfRendererAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg);
std::shared_ptr<core::ports::document_image_processing::IDocumentImageProcessor>
createDocumentImageProcessorAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg);
std::shared_ptr<core::ports::text_recognition::ITextRecognizer>
createTextRecognizerAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg);

namespace {

std::weak_ptr<core::ports::diagnostics::IErrorReporter> g_qtMessageReporter;
std::mutex g_qtMessageReporterMutex;

void setQtMessageReporter(
    const std::shared_ptr<core::ports::diagnostics::IErrorReporter>& reporter) {
  std::lock_guard<std::mutex> lock(g_qtMessageReporterMutex);
  g_qtMessageReporter = reporter;
}

std::shared_ptr<core::ports::diagnostics::IErrorReporter> qtMessageReporter() {
  std::lock_guard<std::mutex> lock(g_qtMessageReporterMutex);
  return g_qtMessageReporter.lock();
}

void ensureParentDirectoryExists(const std::filesystem::path& path,
                                 const std::shared_ptr<
                                     core::ports::diagnostics::IErrorReporter>&
                                     errorReporter,
                                 const char* origin) {
  try {
    if (path.has_parent_path())
      std::filesystem::create_directories(path.parent_path());
  } catch (...) {
    core::errors::reportException(errorReporter.get(),
                                  core::errors::ErrorSeverity::Warning, origin,
                                  std::current_exception());
  }
}

bool startBannerEnabled() {
  const QByteArray value =
      qgetenv("FOSSREDDER_START_BANNER").trimmed().toLower();
  return value != "0" && value != "false" && value != "off";
}

bool consoleColorEnabled() {
  if (!qgetenv("NO_COLOR").isEmpty()) {
    return false;
  }
  const QByteArray value =
      qgetenv("FOSSREDDER_CONSOLE_COLOR").trimmed().toLower();
  return value != "0" && value != "false" && value != "off";
}

const char* buildType() {
#if defined(NDEBUG)
  return "Release";
#else
  return "Debug";
#endif
}

const char* appVersion() {
#if defined(FOSSREDDER_VERSION)
  return FOSSREDDER_VERSION;
#else
  return "dev";
#endif
}

void printStartBanner(const QApplication& app,
                      const std::filesystem::path& workspacePath,
                      const std::filesystem::path& registryPath) {
  if (!startBannerEnabled()) {
    return;
  }

  const bool color = consoleColorEnabled();
  const std::string_view cyan = color ? "\033[36m" : "";
  const std::string_view dim = color ? "\033[2m" : "";
  const std::string_view gold = color ? "\033[33m" : "";
  const std::string_view reset = color ? "\033[0m" : "";

  std::cout << '\n'
            << cyan << "        .----------------.        " << reset << '\n'
            << cyan << "      .'                  '.      " << reset << '\n'
            << cyan << "     /      ________        \\     " << reset << '\n'
            << cyan << "    |      /  ____/_,  o     |    " << reset << '\n'
            << cyan << "    |     /  /___   o--'     |    " << reset << '\n'
            << cyan << "    |    /  ____/-----" << gold << "o" << cyan
            << "      |    " << reset << '\n'
            << cyan << "    |   /__/                 |    " << reset << '\n'
            << cyan << "     \\                      /     " << reset << '\n'
            << cyan << "      '.                  .'      " << reset << '\n'
            << cyan << "        '----------------'        " << reset << '\n'
            << '\n'
            << "  FossRedder" << dim << "  |  desktop workspace runtime"
            << reset << '\n'
            << dim << "  version: " << appVersion()
            << "  |  build: " << buildType() << "  |  Qt: " << QT_VERSION_STR
            << "  |  app: " << app.applicationName().toStdString() << reset
            << '\n'
            << dim << "  workspace: " << workspacePath.string() << reset << '\n'
            << dim << "  registry:  " << registryPath.string() << reset << '\n'
            << '\n';
  std::cout.flush();
}

void printShutdownMessage(int exitCode) {
  if (!startBannerEnabled()) {
    return;
  }

  const bool color = consoleColorEnabled();
  const std::string_view green = color ? "\033[32m" : "";
  const std::string_view dim = color ? "\033[2m" : "";
  const std::string_view reset = color ? "\033[0m" : "";

  std::cout << green << "[shutdown] FossRedder stopped cleanly." << reset << dim
            << " exitCode=" << exitCode << reset << '\n';
  std::cout.flush();
}

struct UseCaseRunners {
  std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner;
  std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner;
  std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner;
  std::shared_ptr<core::ports::importing::IImportRunner> importRunner;
};

class AppComposition {
public:
  explicit AppComposition(
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> reporter)
      : errorReporter_(std::move(reporter)) {}

  UseCaseRunners createUseCaseRunners() const {
    return {
        createAnalysisRunner(),
        createAnnualRunner(),
        createExportRunner(),
        createImportRunner(),
    };
  }

private:
  std::shared_ptr<core::ports::analysis::IAnalysisRunner>
  createAnalysisRunner() const {
    return std::make_shared<core::application::analysis::AnalysisService>(
        createAnalysisRenderer());
  }

  std::shared_ptr<core::ports::annual::IAnnualRunner>
  createAnnualRunner() const {
    return std::make_shared<core::application::annual::AnnualService>();
  }

  std::shared_ptr<core::ports::exporting::IExportRunner>
  createExportRunner() const {
    return std::make_shared<core::application::exporting::ExportService>(
        createArchiveAdapter(), createTableWriterAdapter(),
        createAnalysisRenderer());
  }

  std::shared_ptr<core::ports::importing::IImportRunner>
  createImportRunner() const {
    const auto importDiagnostics = createImportDiagnostics();
    auto importService = core::application::importing::createImportStatement(
        createPdfRendererAdapter(importDiagnostics),
        createDocumentImageProcessorAdapter(importDiagnostics),
        createTextRecognizerAdapter(importDiagnostics), errorReporter_);
    const auto importRunBasePath =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
            .toStdString();
    return std::make_shared<core::application::importing::StatementImportRunner>(
        importService, importRunBasePath, errorReporter_);
  }

  std::shared_ptr<infra::analysis_rendering::
                      OpenCvAnalysisRendererAdapter>
  createAnalysisRenderer() const {
    return std::make_shared<
        infra::analysis_rendering::OpenCvAnalysisRendererAdapter>(
        errorReporter_);
  }

  std::shared_ptr<infra::archive::ZipArchiveAdapter>
  createArchiveAdapter() const {
    return std::make_shared<infra::archive::ZipArchiveAdapter>(errorReporter_);
  }

  std::shared_ptr<infra::xlsx_writer::XlntTableWriterAdapter>
  createTableWriterAdapter() const {
    return std::make_shared<infra::xlsx_writer::XlntTableWriterAdapter>(
        errorReporter_);
  }

  std::shared_ptr<core::ports::diagnostics::IDiagnostics>
  createImportDiagnostics() const {
    return std::make_shared<diagnostics::FileDiagnostics>(
        "", std::string(diagnostics::defaults::kImportProcessName));
  }

  std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

UseCaseRunners createUseCaseRunners(
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter) {
  return AppComposition(std::move(errorReporter)).createUseCaseRunners();
}

/**
 * @brief Global Qt message handler that forwards Qt diagnostics to the
 * reporter.
 */
static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context,
                             const QString& msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  const char* file = context.file ? context.file : "";
  const std::string text = localMsg.constData();
  const core::errors::ErrorContext ctx = {
      {"file", file}, {"line", std::to_string(context.line)}};
  const auto reporter = qtMessageReporter();
  switch (type) {
    case QtDebugMsg:
      core::errors::report(reporter.get(), core::errors::ErrorSeverity::Info,
                           core::errors::codes::QtDebug,
                           "app::qtMessageHandler", text, ctx);
      break;
    case QtInfoMsg:
      core::errors::report(reporter.get(), core::errors::ErrorSeverity::Info,
                           core::errors::codes::QtInfo, "app::qtMessageHandler",
                           text, ctx);
      break;
    case QtWarningMsg:
      core::errors::report(reporter.get(), core::errors::ErrorSeverity::Warning,
                           core::errors::codes::QtWarning,
                           "app::qtMessageHandler", text, ctx);
      break;
    case QtCriticalMsg:
      core::errors::report(reporter.get(), core::errors::ErrorSeverity::Error,
                           core::errors::codes::QtCritical,
                           "app::qtMessageHandler", text, ctx);
      break;
    case QtFatalMsg:
      core::errors::report(reporter.get(), core::errors::ErrorSeverity::Critical,
                           core::errors::codes::QtFatal,
                           "app::qtMessageHandler", text, ctx);
      abort();
  }
}

} // namespace

/**
 * @brief Start the QML application UI.
 *
 * Implemented in `main_qml.cpp`.
 */
extern int startQmlApp(
    QApplication& app,
    core::ports::workspace::IWorkspaceReader& workspaceReader,
    core::ports::workspace::IWorkspaceWriter& workspaceWriter,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> analysisRunner,
    std::shared_ptr<core::ports::annual::IAnnualRunner> annualRunner,
    std::shared_ptr<core::ports::exporting::IExportRunner> exportRunner,
    std::shared_ptr<core::ports::importing::IImportRunner> importRunner);

int main(int argc, char* argv[]) {
  auto errorReporter = diagnostics::createDefaultErrorReporter();
  setQtMessageReporter(errorReporter);

#if defined(_DEBUG)
  if (qEnvironmentVariableIsEmpty("QT_ACCESSIBILITY")) {
    qputenv("QT_ACCESSIBILITY", QByteArrayLiteral("0"));
  }
#endif

  const auto previousQtMessageHandler =
      qInstallMessageHandler(qtMessageHandler);

  QQuickStyle::setStyle(core::constants::runtime::kQtStyle.data());

  QApplication app(argc, argv);
  app.setStyle(core::constants::runtime::kQtStyle.data());
  app.setOrganizationName(QString::fromLatin1(
      core::constants::preferences::kOrganizationName.data()));
  app.setApplicationName(QString::fromLatin1(
      core::constants::preferences::kApplicationName.data()));
  app.setWindowIcon(QIcon(ui::config::kAppIconResource));

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
  ensureParentDirectoryExists(defaultDbPath, errorReporter,
                              "app::main::createConfigDirectory");
  ensureParentDirectoryExists(registryDbPath, errorReporter,
                              "app::main::createRegistryDirectory");
  printStartBanner(app, defaultDbPath, registryDbPath);

  std::shared_ptr<core::ports::storage::IRegistry> registry;
  try {
    registry = createSqliteRegistry(registryDbPath.string());
  } catch (const std::exception& ex) {
    core::errors::report(errorReporter.get(), core::errors::ErrorSeverity::Warning,
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
  appStateFacade.setAtomicStoreLoad([](const std::string& dbPath) {
    auto db = createSqliteDb(dbPath);
    WorkspaceStateStore store(db);
    return store.load();
  });
  appStateFacade.setAtomicStoreSave(
      [](const std::string& dbPath,
         const core::application::workspace::WorkspaceSessionState& document) {
        auto db = createSqliteDb(dbPath);
        WorkspaceStateStore store(db);
        return store.save(document);
      });

  try {
    appStateFacade.openLatest();
  } catch (const std::exception& ex) {
    (void)ex;
    core::errors::reportException(errorReporter.get(),
                                  core::errors::ErrorSeverity::Warning,
                                  "app::main::openLatest",
                                  std::current_exception());
    // continue with empty state
  }

  if (appStateFacade.currentPath().empty() &&
      appStateFacade.workspaceSnapshot().empty()) {
    appStateFacade.newFile(defaultDbPath.string());
  }

  QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

  try {
    auto runners = createUseCaseRunners(errorReporter);
    const int exitCode = startQmlApp(
        app, appStateFacade, appStateFacade, errorReporter,
        std::move(runners.analysisRunner), std::move(runners.annualRunner),
        std::move(runners.exportRunner), std::move(runners.importRunner));
    printShutdownMessage(exitCode);
    qInstallMessageHandler(previousQtMessageHandler);
    appStateFacade.setErrorReporter({});
    setQtMessageReporter({});
    return exitCode;
  } catch (const std::exception& ex) {
    qInstallMessageHandler(previousQtMessageHandler);
    core::errors::reportException(errorReporter.get(),
                                  core::errors::ErrorSeverity::Critical,
                                  "app::main::startQmlApp",
                                  std::current_exception());
    QMessageBox::critical(
        nullptr, QObject::tr("Fatal error"),
        QObject::tr("Startup failed: %1").arg(QString::fromUtf8(ex.what())));
    appStateFacade.setErrorReporter({});
    setQtMessageReporter({});
    return -1;
  } catch (...) {
    qInstallMessageHandler(previousQtMessageHandler);
    core::errors::reportException(errorReporter.get(),
                                  core::errors::ErrorSeverity::Critical,
                                  "app::main::startQmlAppUnknown",
                                  std::current_exception());
    QMessageBox::critical(nullptr, QObject::tr("Fatal error"),
                          QObject::tr("Startup failed: unknown exception"));
    appStateFacade.setErrorReporter({});
    setQtMessageReporter({});
    return -2;
  }
}
