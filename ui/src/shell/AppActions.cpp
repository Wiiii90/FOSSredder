/**
 * @file ui/src/shell/AppActions.cpp
 * @brief Implements QAction creation and browse request forwarding for the UI.
 */

#include "ui/shell/AppActions.h"

#include <QAction>
#include <QCoreApplication>
#include <QGuiApplication>

#include "ui/i18n/Text.h"
#include "ui/shell/Defaults.h"

namespace ui {

Actions::Actions(QObject *parent) : QObject(parent) {
  if (!qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
    return;
  }

  newFileAction_ = new QAction(ui::text::actions::newFile(), this);
  openFileAction_ = new QAction(ui::text::actions::openFile(), this);
  saveFileAction_ = new QAction(ui::text::actions::saveFile(), this);
  saveFileAsAction_ = new QAction(ui::text::actions::saveFileAs(), this);
  quitAction_ = new QAction(ui::text::actions::quit(), this);
  aboutAction_ = new QAction(ui::text::actions::about(), this);

  newFileAction_->setObjectName(ui::config::objectNames::kActionNewFile);
  openFileAction_->setObjectName(ui::config::objectNames::kActionOpenFile);
  saveFileAction_->setObjectName(ui::config::objectNames::kActionSaveFile);
  saveFileAsAction_->setObjectName(ui::config::objectNames::kActionSaveFileAs);

  quitAction_->setObjectName(ui::config::objectNames::kActionQuit);
  aboutAction_->setObjectName(ui::config::objectNames::kActionAbout);
}

QAction *Actions::newFileAction() const { return newFileAction_; }
QAction *Actions::openFileAction() const { return openFileAction_; }
QAction *Actions::saveFileAction() const { return saveFileAction_; }
QAction *Actions::saveFileAsAction() const { return saveFileAsAction_; }
QAction *Actions::quitAction() const { return quitAction_; }
QAction *Actions::aboutAction() const { return aboutAction_; }

void Actions::newFile() {
  if (newFileAction_)
    newFileAction_->trigger();
}
void Actions::openFile() {
  if (openFileAction_)
    openFileAction_->trigger();
}
void Actions::saveFile() {
  if (saveFileAction_)
    saveFileAction_->trigger();
}
void Actions::saveFileAs() {
  if (saveFileAsAction_)
    saveFileAsAction_->trigger();
}

void Actions::browseImportPdf() {
  emit importBrowseRequested(ui::text::dialogs::importPdfFilter());
}

void Actions::browseExportFile() {
  emit exportBrowseRequested(ui::text::dialogs::exportFileFilter());
}

void Actions::browseExportDirectory() {
  emit exportDirectoryBrowseRequested(
      ui::text::dialogs::exportDirectoryTitle());
}

} // namespace ui
