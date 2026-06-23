/**
 * @file ui/src/platform/FileDialogs.cpp
 * @brief Implements reusable native file dialog entry points for the UI.
 */

#include "ui/platform/FileDialogs.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

#include "ui/i18n/Text.h"

namespace ui::dialogs {

namespace {

QString writableDocumentsDirectory() {
  const QString documents =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  return documents.isEmpty() ? QDir::homePath() : documents;
}

QString ensureDirectory(const QString& path) {
  if (!path.isEmpty()) {
    QDir().mkpath(path);
  }
  return path;
}

QString workspaceDirectory() {
  return ensureDirectory(QDir(writableDocumentsDirectory())
                             .filePath(QStringLiteral("FOSSredder")));
}

QString exportDirectory() {
  return ensureDirectory(
      QDir(workspaceDirectory()).filePath(QStringLiteral("Exports")));
}

QString workspaceFilePath() {
  return QDir(workspaceDirectory())
      .filePath(QStringLiteral("workspace.fossredder"));
}

QString cleanForCompare(const QString& path) {
  return QDir::cleanPath(QFileInfo(path).absoluteFilePath()).toCaseFolded();
}

bool isInsideDirectory(const QString& path, const QString& directory) {
  if (path.isEmpty() || directory.isEmpty()) {
    return false;
  }

  const QString normalizedPath = cleanForCompare(path);
  const QString normalizedDirectory = cleanForCompare(directory);
  return normalizedPath == normalizedDirectory ||
         normalizedPath.startsWith(normalizedDirectory + QLatin1Char('/'));
}

bool isInsideAppData(const QString& path) {
  return isInsideDirectory(path, QStandardPaths::writableLocation(
                                     QStandardPaths::AppDataLocation)) ||
         isInsideDirectory(path, QStandardPaths::writableLocation(
                                     QStandardPaths::AppLocalDataLocation));
}

QString existingDirectoryFromPath(const QString& path) {
  const QString trimmed = path.trimmed();
  if (trimmed.isEmpty()) {
    return {};
  }

  const QFileInfo info(trimmed);
  if (info.isDir() && info.exists()) {
    return info.absoluteFilePath();
  }

  const QString parent = info.absoluteDir().absolutePath();
  return QDir(parent).exists() ? parent : QString();
}

QString startDirectory(const QString& initialPath,
                       const QString& fallbackDirectory) {
  const QString preferred = isInsideAppData(initialPath)
                                ? QString()
                                : existingDirectoryFromPath(initialPath);
  return QDir::toNativeSeparators(preferred.isEmpty() ? fallbackDirectory
                                                      : preferred);
}

QString startFilePath(const QString& initialPath, const QString& fallbackFile) {
  const QString trimmed = initialPath.trimmed();
  if (!trimmed.isEmpty() && !isInsideAppData(trimmed) &&
      !existingDirectoryFromPath(trimmed).isEmpty()) {
    return QDir::toNativeSeparators(trimmed);
  }
  return QDir::toNativeSeparators(fallbackFile);
}

} // namespace

QStringList pickImportFiles(QWidget* parent, const QString& filter,
                            const QString& initialPath) {
  return QFileDialog::getOpenFileNames(
      parent, ui::text::dialogs::selectPdfTitle(),
      startDirectory(initialPath, writableDocumentsDirectory()), filter);
}

QString pickExportFile(QWidget* parent, const QString& filter,
                       const QString& initialPath) {
  return QFileDialog::getSaveFileName(
      parent, ui::text::dialogs::exportFileTitle(),
      startDirectory(initialPath, exportDirectory()), filter);
}

QString pickExportDirectory(QWidget* parent, const QString& title,
                            const QString& initialPath) {
  return QFileDialog::getExistingDirectory(
      parent, title, startDirectory(initialPath, exportDirectory()),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

QString pickNewStorageFile(QWidget* parent, const QString& initialPath) {
  return QFileDialog::getSaveFileName(
      parent, ui::text::dialogs::newFileTitle(),
      startFilePath(initialPath, workspaceFilePath()),
      ui::text::dialogs::databaseFilter());
}

QString pickOpenStorageFile(QWidget* parent, const QString& initialPath) {
  return QFileDialog::getOpenFileName(
      parent, ui::text::dialogs::openFileTitle(),
      startDirectory(initialPath, workspaceDirectory()),
      ui::text::dialogs::databaseFilter());
}

QString pickSaveStorageFileAs(QWidget* parent, const QString& initialPath) {
  return QFileDialog::getSaveFileName(
      parent, ui::text::dialogs::saveFileAsTitle(),
      startFilePath(initialPath, workspaceFilePath()),
      ui::text::dialogs::databaseFilter());
}

} // namespace ui::dialogs
