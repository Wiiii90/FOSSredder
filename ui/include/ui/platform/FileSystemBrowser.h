/**
 * @file ui/include/ui/platform/FileSystemBrowser.h
 * @brief Declares the UI helper that exposes a minimal filesystem view to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

namespace ui {

/**
 * @brief Exposes directory listing and application-directory lookup to QML.
 */
class FileSystemBrowser : public QObject {
  Q_OBJECT
public:
  /** @brief Create the filesystem helper exposed to QML. */
  explicit FileSystemBrowser(QObject *parent = nullptr);

  /** @brief List the contents of a directory in a QML-friendly payload shape.
   *  @param path Directory path
   *  @return Directory contents as QVariantList
   */
  Q_INVOKABLE QVariantList listDir(const QString &path) const;

  /** @brief Return the current application directory path.
   *  @return Application directory path
   */
  Q_INVOKABLE QString appDir() const;

  /**
   * @brief Opens the containing folder for a file or directory path.
   * @param path File or directory path.
   * @return True when an existing location was opened.
   */
  Q_INVOKABLE bool openLocation(const QString &path) const;
};

} // namespace ui
