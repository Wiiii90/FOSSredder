/**
 * @file ui/src/MainWindow.cpp
 * @brief Implements the desktop main window hosting the QML UI surface.
 */

#include "MainWindow.h"

#include <QCloseEvent>
#include <QEvent>
#include <QIcon>
#include <QMessageBox>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlImageProviderBase>
#include <QVariant>
#include <QQuickItem>
#include <QQuickView>
#include <QSize>
#include <QSizePolicy>
#include <QWidget>
#include <qqml.h>
#include <string>

#include "core/constants/app.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/AppContext.h"
#include "ui/shell/QmlContracts.h"
#include "ui/shell/QmlRuntime.h"
#include "ui/shared/config/Defaults.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/shared/text/Text.h"
#include "ui/shell/window/MainWindowContext.h"
#include "ui/shell/window/MainWindowTrace.h"

namespace {

using ui::observability::context::kError;

/** @brief Keeps the QML root object's size properties aligned with the host size. */
void syncRootObjectSize(QQuickView *quickView, QWidget *hostWidget) {
  if (!quickView || !hostWidget || !quickView->rootObject())
    return;

  QObject *root = quickView->rootObject();
  root->setProperty(ui::qml::contracts::properties::kWidth,
                    hostWidget->width());
  root->setProperty(ui::qml::contracts::properties::kHeight,
                    hostWidget->height());
}

/** @brief Reports synchronous QML load failures from the quick view. */
void reportQmlLoadErrors(QQuickView *quickView, const QUrl &source) {
  if (!quickView || quickView->status() != QQuickView::Error)
    return;

  const auto errors = quickView->errors();
  if (errors.isEmpty()) {
    core::errors::report(core::errors::ErrorSeverity::Error,
                         ui::observability::codes::QmlLoadFailed,
                         ui::observability::origins::mainWindow::kLoadQml,
                         "QQuickView failed to load the main QML source",
                         {{"url", source.toString().toStdString()}});
    return;
  }

  for (const auto &error : errors) {
    core::errors::report(core::errors::ErrorSeverity::Error,
                         ui::observability::codes::QmlLoadFailed,
                         ui::observability::origins::mainWindow::kLoadQml,
                         error.toString().toStdString(),
                         {{"url", error.url().toString().toStdString()},
                          {"line", std::to_string(error.line())},
                          {"column", std::to_string(error.column())}});
  }
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle(
      QString::fromLatin1(core::constants::application::kDisplayName.data()));
  resize(ui::config::kMainWindowDefaultWidth,
         ui::config::kMainWindowDefaultHeight);

  setupQuickHost();
  setupUiContext();
  setCentralWidget(m_quickContainer);

  setupActionRouting();

  setupQmlRuntime();
}

void MainWindow::setupQuickHost() {
  if (m_quickView || m_quickContainer)
    return;

  m_quickView = new QQuickView();
  m_quickView->setResizeMode(QQuickView::SizeRootObjectToView);
  appContext_ = new ui::bootstrap::AppContext(this);

  m_quickContainer = QWidget::createWindowContainer(m_quickView, this);
  m_quickContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_quickContainer->setMinimumSize(0, 0);
  m_quickContainer->setAcceptDrops(true);
  m_quickContainer->installEventFilter(this);
}

void MainWindow::setupUiContext() {
  if (!m_quickView || !m_quickView->rootContext())
    return;

  if (appContext_) {
    m_quickView->rootContext()->setContextProperty(
        ui::qml::contracts::module::kAppContextTypeName, appContext_);
  }

  const auto services =
      ui::window::installMainWindowContext(*m_quickView->rootContext(), this,
                                           this);
  actions_ = services.actions;
  workspace_ = services.workspace;
  status_ = services.status;

  if (appContext_) {
    appContext_->setActions(services.actions);
    appContext_->setNavigation(services.navigation);
    appContext_->setWorkspace(services.workspace);
    appContext_->setFileSystemBrowser(services.fileSystemBrowser);
    appContext_->setLanguageService(services.languageService);
    appContext_->setSettingsViewModel(services.settingsViewModel);
    appContext_->setStatus(services.status);
  }
}

void MainWindow::setupActionRouting() {
  ui::window::wireMainWindowActions(
      *this, {actions_, nullptr, workspace_, nullptr, nullptr, nullptr, status_},
      [this]() { onAbout(); });
}

void MainWindow::setupQmlRuntime() {
  auto *engine = m_quickView ? m_quickView->engine() : nullptr;
  ui::bootstrap::registerTypes();
  ui::bootstrap::configureRuntime(engine);
}

void MainWindow::prepareForQmlShutdown() {
  if (qmlShutdownPrepared_ || (!m_quickView && !m_quickContainer))
    return;

  qmlShutdownPrepared_ = true;
  auto *quickView = m_quickView;
  auto *quickContainer = m_quickContainer;
  m_quickView = nullptr;
  m_quickContainer = nullptr;

  if (quickContainer)
    quickContainer->removeEventFilter(this);
  if (quickView && !quickView->source().isEmpty())
    quickView->setSource(QUrl());
  if (quickContainer && centralWidget() == quickContainer)
    takeCentralWidget();
  if (quickContainer) {
    quickContainer->hide();
    quickContainer->setParent(nullptr);
    delete quickContainer;
  } else if (quickView) {
    delete quickView;
  }
}

MainWindow::~MainWindow() { prepareForQmlShutdown(); }

void MainWindow::setQmlContextProperty(const QString &name, QObject *value) {
  if (!m_quickView)
    return;
  if (!m_quickView->rootContext())
    return;
  m_quickView->rootContext()->setContextProperty(name, value);
  if (appContext_)
    appContext_->setProperty(name.toUtf8().constData(), QVariant::fromValue(value));
}

void MainWindow::setQmlContextValue(const QString &name, const QVariant &value) {
  if (!m_quickView)
    return;
  if (!m_quickView->rootContext())
    return;
  m_quickView->rootContext()->setContextProperty(name, value);
  if (appContext_)
    appContext_->setProperty(name.toUtf8().constData(), value);
}

void MainWindow::addImageProvider(const QString &id,
                                  QQmlImageProviderBase *provider) {
  if (!m_quickView)
    return;
  if (!m_quickView->engine())
    return;
  m_quickView->engine()->addImageProvider(id, provider);
}

void MainWindow::loadQml(const QUrl &source) {
  if (!m_quickView)
    return;
  if (!source.isEmpty() && m_quickView->source() == source)
    return;

  if (source.isEmpty()) {
    m_quickView->loadFromModule(ui::qml::contracts::module::kName,
                                ui::qml::contracts::module::kMainTypeName);
    reportQmlLoadErrors(m_quickView,
                        QUrl(QStringLiteral("module:FossRedder/Main")));
  } else {
    m_quickView->setSource(source);
    reportQmlLoadErrors(m_quickView, source);
  }
  syncRootObjectSize(m_quickView, m_quickContainer);
}

QQmlEngine *MainWindow::qmlEngine() const noexcept {
  return m_quickView ? m_quickView->engine() : nullptr;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev) {
  if (obj == m_quickContainer && ev->type() == QEvent::Resize) {
    syncRootObjectSize(m_quickView, m_quickContainer);
  }

  if (obj == m_quickContainer) {
    const auto outcome = dropHandler_.handle(ev);
    if (outcome.handled) {
      if (ev->type() == QEvent::Drop && outcome.accepted && actions_) {
        ui::window::reportMainWindowFlow(
            ui::observability::origins::mainWindow::kDragDrop,
            "Import files dropped", core::errors::ErrorSeverity::Info,
            ui::window::makeFileListContext(outcome.files));
        emit actions_->importFilesDropped(outcome.files);
        if (outcome.files.size() == 1)
          emit actions_->importFileDropped(outcome.files.first());
      }
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (closeWorkflow_.allowImmediateClose(event)) {
    prepareForQmlShutdown();
    QMainWindow::closeEvent(event);
    return;
  }

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kClose,
      "Main window close requested; triggering save workflow");
  closeWorkflow_.requestClose(event, [this]() { emit saveFileRequested(); });
}

void MainWindow::handleStorageOperationSucceeded(const QString &operation) {
  if (!closeWorkflow_.handleStorageOperationSucceeded(
          operation, ui::config::operationKeys::kSaveFile,
          [this]() {
            QMetaObject::invokeMethod(
                this, [this]() { close(); }, Qt::QueuedConnection);
          })) {
    return;
  }

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kCloseSucceeded,
      "Pending close save finished; closing main window");
}

void MainWindow::handleStorageOperationFailed(const QString &operation,
                                              const QString &error) {
  if (!closeWorkflow_.handleStorageOperationFailed(
          operation, ui::config::operationKeys::kSaveFile))
    return;

  const QString message =
      error.isEmpty() ? ui::text::workflowErrors::storageSaveFailed()
                      : error;
  if (status_)
    status_->setText(message);

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kCloseFailed,
      "Pending close save failed; keeping main window open",
      core::errors::ErrorSeverity::Warning,
      {{kError, ui::strings::toStdString(message)}});

  QMetaObject::invokeMethod(
      this, [this]() { close(); }, Qt::QueuedConnection);
}

void MainWindow::onAbout() {
  QMessageBox dialog(this);
  dialog.setWindowTitle(ui::text::mainWindow::aboutTitle());
  dialog.setIconPixmap(QIcon(ui::config::kAppIconResource)
                           .pixmap(QSize(ui::config::kAboutDialogIconSize,
                                         ui::config::kAboutDialogIconSize)));
  dialog.setText(ui::text::mainWindow::aboutHeadline());
  dialog.setInformativeText(ui::text::mainWindow::aboutBody());
  dialog.setStandardButtons(QMessageBox::Ok);
  dialog.exec();
}
