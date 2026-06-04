/**
 * @file ui/src/platform/FileDialogs.cpp
 * @brief Implements reusable native file dialog entry points for the UI.
 */

#include "ui/platform/FileDialogs.h"

#include <QFileDialog>

#include "ui/i18n/Text.h"

namespace ui::dialogs {

QStringList pickImportFiles(QWidget *parent, const QString &filter) {
  return QFileDialog::getOpenFileNames(
      parent, ui::text::dialogs::selectPdfTitle(), QString(), filter);
}

QString pickExportFile(QWidget *parent, const QString &filter) {
  return QFileDialog::getSaveFileName(
      parent, ui::text::dialogs::exportFileTitle(), QString(), filter);
}

QString pickExportDirectory(QWidget *parent, const QString &title) {
  return QFileDialog::getExistingDirectory(
      parent, title, QString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

QString pickNewStorageFile(QWidget *parent) {
  return QFileDialog::getSaveFileName(parent, ui::text::dialogs::newFileTitle(),
                                      QString(),
                                      ui::text::dialogs::databaseFilter());
}

QString pickOpenStorageFile(QWidget *parent) {
  return QFileDialog::getOpenFileName(
      parent, ui::text::dialogs::openFileTitle(), QString(),
      ui::text::dialogs::databaseFilter());
}

QString pickSaveStorageFileAs(QWidget *parent) {
  return QFileDialog::getSaveFileName(
      parent, ui::text::dialogs::saveFileAsTitle(), QString(),
      ui::text::dialogs::databaseFilter());
}

} // namespace ui::dialogs
