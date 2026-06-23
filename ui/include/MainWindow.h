/**
 * @file ui/include/MainWindow.h
 * @brief Main window declaration for the UI application.
 */

#pragma once

#include <QMainWindow>
#include <QUrl>

#include <memory>

#include "ui/shell/AppActions.h"
#include "ui/shell/Defaults.h"
#include "ui/shell/Status.h"
#include "ui/shell/window/CloseWorkflow.h"
#include "ui/shell/window/DropHandler.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

QT_FORWARD_DECLARE_CLASS(QQmlImageProviderBase)
QT_FORWARD_DECLARE_CLASS(QQmlEngine)
QT_FORWARD_DECLARE_CLASS(QQuickView)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWidget)

namespace ui::bootstrap {
class AppContext;
}
namespace ui {
class Settings;
}
namespace core::ports::diagnostics {
class IErrorReporter;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(
      std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter,
      QWidget* parent = nullptr);
  ~MainWindow();

  void addImageProvider(const QString& id, QQmlImageProviderBase* provider);
  void loadQml(const QUrl& source = QUrl());
  QQmlEngine* qmlEngine() const noexcept;
  ui::bootstrap::AppContext* appContext() const noexcept {
    return appContext_;
  }
  ui::WorkspaceStore* workspaceStore() const noexcept {
    return workspaceStore_;
  }
  ui::WorkspaceCommands* workspaceCommands() const noexcept {
    return workspaceCommands_;
  }
  ui::WorkspaceSelection* workspaceSelection() const noexcept {
    return workspaceSelection_;
  }
  ui::WorkspaceSelectors* workspaceSelectors() const noexcept {
    return workspaceSelectors_;
  }
  ui::Settings* settings() const noexcept {
    return settings_;
  }

public slots:
  void handleStorageOperationSucceeded(const QString& operation);
  void handleStorageOperationFailed(const QString& operation,
                                    const QString& error);

signals:
  void newFileRequested(const QString& path);
  void openFileRequested(const QString& path);
  void saveFileRequested();
  void saveFileAsRequested(const QString& path);

private slots:
  void onAbout();
  void applyAutosaveSchedule();

protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  void closeEvent(QCloseEvent* event) override;

private:
  void setupQuickHost();
  void setupUiContext();
  void setupActionRouting();
  void setupQmlRuntime();
  /** @brief Unloads the hosted QML surface before widget teardown begins. */
  void prepareForQmlShutdown();
  void setupAutosaveTimer();

  QQuickView* m_quickView = nullptr;
  QTimer* autosaveTimer_ = nullptr;
  QWidget* m_quickContainer = nullptr;
  ui::WorkspaceStore* workspaceStore_ = nullptr;
  ui::WorkspaceCommands* workspaceCommands_ = nullptr;
  ui::WorkspaceSelection* workspaceSelection_ = nullptr;
  ui::WorkspaceSelectors* workspaceSelectors_ = nullptr;
  ui::Settings* settings_ = nullptr;
  ui::Actions* actions_ = nullptr;
  ui::Status* status_ = nullptr;
  ui::bootstrap::AppContext* appContext_ = nullptr;
  std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
  bool qmlShutdownPrepared_ = false;
  ui::window::CloseWorkflow closeWorkflow_;
  ui::window::DropHandler dropHandler_;
};
