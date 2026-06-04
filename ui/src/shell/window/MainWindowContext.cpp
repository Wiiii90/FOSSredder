/**
 * @file ui/src/shell/window/MainWindowContext.cpp
 * @brief Implements main window service wiring between widgets, QML and UI services.
 */

#include "ui/shell/window/MainWindowContext.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QQmlEngine>

#include "MainWindow.h"
#include "ui/shell/AppActions.h"
#include "ui/platform/FileSystemBrowser.h"
#include "ui/platform/LanguageService.h"
#include "ui/platform/FileDialogs.h"
#include "ui/observability/Origins.h"
#include "ui/shell/NavigationState.h"
#include "ui/shell/Settings.h"
#include "ui/shell/StatusState.h"
#include "ui/workspace/WorkspaceFacade.h"
#include "ui/i18n/Text.h"
#include "ui/shell/window/MainWindowTrace.h"

namespace ui::window {

MainWindowServices installMainWindowContext(QQmlEngine *qmlEngine,
                                            QObject *parent) {
  MainWindowServices services;
  services.actions = new ui::Actions(parent);
  services.navigation = new ui::NavigationState(parent);
  services.workspaceFacade = new ui::WorkspaceFacade(parent);
  services.fileSystemBrowser = new ui::FileSystemBrowser(parent);
  auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
  services.languageService = new ui::LanguageService(application, qmlEngine, parent);
  services.settings = new ui::Settings(parent);
  services.status = new ui::StatusState(parent);
  services.status->setText(ui::text::status::ready());

  return services;
}

void wireMainWindowActions(MainWindow &window,
                           const MainWindowServices &services,
                           const std::function<void()> &showAbout) {
  if (!services.actions)
    return;

  auto *actions = services.actions;
  QObject::connect(actions->newFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickNewStorageFile(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested new file", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.newFileRequested(file);
                   });
  QObject::connect(actions->openFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickOpenStorageFile(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested open file", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.openFileRequested(file);
                   });
  QObject::connect(actions->saveFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested save file");
                     emit window.saveFileRequested();
                   });
  QObject::connect(actions->saveFileAsAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickSaveStorageFileAs(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested save file as", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.saveFileAsRequested(file);
                   });

  QObject::connect(actions->quitAction(), &QAction::triggered, &window,
                   &QWidget::close);
  QObject::connect(actions->aboutAction(), &QAction::triggered, &window,
                   [showAbout]() {
                     if (showAbout)
                       showAbout();
                   });

  QObject::connect(
      actions, &ui::Actions::importBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &filter) {
        const QStringList files = ui::dialogs::pickImportFiles(&window, filter);
        if (!files.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected import files", core::errors::ErrorSeverity::Info,
              ui::window::makeFileListContext(files));
          emit actions->importFilesSelected(files);
          if (files.size() == 1)
            emit actions->importFileSelected(files.first());
          if (status)
            status->setText(
                ui::text::mainWindow::selectedStatusPattern().arg(files.front()));
        }
      });

  QObject::connect(
      actions, &ui::Actions::exportBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &filter) {
        const QString file = ui::dialogs::pickExportFile(&window, filter);
        if (!file.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected export path", core::errors::ErrorSeverity::Info,
              ui::window::makePathContext(file));
          emit actions->exportFileSelected(file);
          if (status)
            status->setText(
                ui::text::mainWindow::exportPathStatusPattern().arg(file));
        }
      });

  QObject::connect(
      actions, &ui::Actions::exportDirectoryBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &title) {
        const QString directory = ui::dialogs::pickExportDirectory(&window, title);
        if (!directory.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected export directory", core::errors::ErrorSeverity::Info,
              ui::window::makePathContext(directory));
          emit actions->exportDirectorySelected(directory);
          if (status)
            status->setText(
                ui::text::mainWindow::exportPathStatusPattern().arg(directory));
        }
      });
}

} // namespace ui::window
