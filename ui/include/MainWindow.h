/**
 * @file ui/include/MainWindow.h
 * @brief Main window declaration for the UI application.
 */

#pragma once

#include <QMainWindow>
#include <QUrl>

#include "ui/shell/Defaults.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/StatusState.h"
#include "ui/shell/window/CloseWorkflow.h"
#include "ui/shell/window/DropHandler.h"
#include "ui/workspace/WorkspaceFacade.h"

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

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void addImageProvider(const QString& id, QQmlImageProviderBase* provider);
  void loadQml(const QUrl& source = QUrl());
  QQmlEngine* qmlEngine() const noexcept;
  ui::bootstrap::AppContext* appContext() const noexcept {
    return appContext_;
  }
  ui::WorkspaceFacade* workspace() const noexcept {
    return workspace_;
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
  ui::WorkspaceFacade* workspace_ = nullptr;
  ui::Settings* settings_ = nullptr;
  ui::Actions* actions_ = nullptr;
  ui::StatusState* status_ = nullptr;
  ui::bootstrap::AppContext* appContext_ = nullptr;
  bool qmlShutdownPrepared_ = false;
  ui::window::CloseWorkflow closeWorkflow_;
  ui::window::DropHandler dropHandler_;
};
