/**
 * @file ui/src/platform/FileSystemBrowser.cpp
 * @brief Implements the UI filesystem helper exposed to QML.
 */

#include "ui/platform/FileSystemBrowser.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include "ui/shared/payload/PayloadKeys.h"

namespace ui {

FileSystemBrowser::FileSystemBrowser(QObject *parent) : QObject(parent) {}

QVariantList FileSystemBrowser::listDir(const QString &path) const {
  QVariantList out;
  QDir d(path);
  if (!d.exists())
    return out;
  QFileInfoList entries = d.entryInfoList(
      QDir::NoDotAndDotDot | QDir::AllEntries, QDir::DirsFirst | QDir::Name);
  for (const QFileInfo &fi : entries) {
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

} // namespace ui
