/**
 * @file ui/include/ui/shell/window/MainWindowContext.h
 * @brief Declarations for the UI MainWindowContext component.
 */

#pragma once

#include <functional>

class MainWindow;
class QObject;
class QQmlEngine;

namespace ui {
class Actions;
class FileSystemBrowser;
class LanguageService;
class NavigationState;
class Settings;
class StatusState;
class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;
} // namespace ui

namespace ui::window {

struct MainWindowServices {
  ui::Actions *actions = nullptr;
  ui::NavigationState *navigation = nullptr;
  ui::WorkspaceStore *workspaceStore = nullptr;
  ui::WorkspaceCommands *workspaceCommands = nullptr;
  ui::WorkspaceSelection *workspaceSelection = nullptr;
  ui::WorkspaceSelectors *workspaceSelectors = nullptr;
  ui::FileSystemBrowser *fileSystemBrowser = nullptr;
  ui::LanguageService *languageService = nullptr;
  ui::Settings *settings = nullptr;
  ui::StatusState *status = nullptr;
};

MainWindowServices installMainWindowContext(QQmlEngine *qmlEngine,
                                            QObject *parent);
void wireMainWindowActions(MainWindow &window,
                           const MainWindowServices &services,
                           const std::function<void()> &showAbout);

} // namespace ui::window
