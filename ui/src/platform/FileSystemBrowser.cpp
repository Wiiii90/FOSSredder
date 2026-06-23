/**
 * @file ui/src/platform/FileSystemBrowser.cpp
 * @brief Implements the UI filesystem helper exposed to QML.
 */

#include "ui/platform/FileSystemBrowser.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "ui/presentation/PayloadKeys.h"

namespace ui {

FileSystemBrowser::FileSystemBrowser(QObject* parent) : QObject(parent) {}

QVariantList FileSystemBrowser::listDir(const QString& path) const {
  QVariantList out;
  QDir d(path);
  if (!d.exists())
    return out;
  QFileInfoList entries = d.entryInfoList(
      QDir::NoDotAndDotDot | QDir::AllEntries, QDir::DirsFirst | QDir::Name);
  for (const QFileInfo& fi : entries) {
    QVariantMap m;
    m[payload::keys::common::kName] = fi.fileName();
    m[payload::keys::fileSystem::kPath] = fi.absoluteFilePath();
    m[payload::keys::fileSystem::kIsDir] = fi.isDir();
    out.append(m);
  }
  return out;
}

QString FileSystemBrowser::appDir() const {
  return QCoreApplication::applicationDirPath();
}

bool FileSystemBrowser::openLocation(const QString& path) const {
  const QString trimmed = path.trimmed();
  if (trimmed.isEmpty()) {
    return false;
  }

  const QFileInfo info(trimmed);
  QString folderPath;
  if (info.exists()) {
    folderPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
  } else {
    const QDir dir(trimmed);
    if (dir.exists()) {
      folderPath = dir.absolutePath();
    }
  }
  return !folderPath.isEmpty() &&
         QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

} // namespace ui
