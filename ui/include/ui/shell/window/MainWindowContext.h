/**
 * @file ui/include/ui/shell/window/MainWindowContext.h
 * @brief Declarations for the UI MainWindowContext component.
 */

#pragma once

#include <functional>
#include <memory>

class MainWindow;
class QObject;
class QQmlEngine;

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace ui {
class Actions;
class FileSystemBrowser;
class LanguageService;
class Navigation;
class Settings;
class Status;
class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;
} // namespace ui

namespace ui::window {

/**
 * @brief Container for shell services owned by the main window context.
 */
struct MainWindowServices {
  ui::Actions* actions = nullptr;
  ui::Navigation* navigation = nullptr;
  ui::WorkspaceStore* workspaceStore = nullptr;
  ui::WorkspaceCommands* workspaceCommands = nullptr;
  ui::WorkspaceSelection* workspaceSelection = nullptr;
  ui::WorkspaceSelectors* workspaceSelectors = nullptr;
  ui::FileSystemBrowser* fileSystemBrowser = nullptr;
  ui::LanguageService* languageService = nullptr;
  ui::Settings* settings = nullptr;
  ui::Status* status = nullptr;
};

/**
 * @brief Creates shell services used by the main window and QML context.
 * @param qmlEngine QML engine bound to runtime language switching.
 * @param parent QObject parent that owns the created services.
 * @return Bundle of created shell services.
 */
MainWindowServices installMainWindowContext(QQmlEngine* qmlEngine,
                                            QObject* parent);

/**
 * @brief Wires main window actions to dialogs, status text and window signals.
 * @param window Main window receiving action callbacks.
 * @param services Shell service bundle containing actions and status state.
 * @param showAbout Callback that opens the about dialog.
 */
void wireMainWindowActions(MainWindow& window,
                           const MainWindowServices& services,
                           const std::shared_ptr<
                               core::ports::diagnostics::IErrorReporter>&
                               errorReporter,
                           const std::function<void()>& showAbout);

} // namespace ui::window
