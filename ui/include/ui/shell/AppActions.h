/**
 * @file ui/include/ui/shell/AppActions.h
 * @brief Declares QAction wrappers exposed to the UI layer and QML.
 */

#pragma once

#include <QObject>
#include <QStringList>

class QAction;

namespace ui {

/**
 * @brief Owns the main window actions and exposes them to QML.
 */
class Actions : public QObject {
  Q_OBJECT
  Q_PROPERTY(QAction *newFileAction READ newFileAction CONSTANT)
  Q_PROPERTY(QAction *openFileAction READ openFileAction CONSTANT)
  Q_PROPERTY(QAction *saveFileAction READ saveFileAction CONSTANT)
  Q_PROPERTY(QAction *saveFileAsAction READ saveFileAsAction CONSTANT)
  Q_PROPERTY(QAction *quitAction READ quitAction CONSTANT)
  Q_PROPERTY(QAction *aboutAction READ aboutAction CONSTANT)

public:
  /**
   * @brief Construct the actions container.
   * @param parent QObject parent
   */
  explicit Actions(QObject *parent = nullptr);

  /** @brief Return the QAction for creating a new file.
   *  @return Pointer to the QAction
   */
  QAction *newFileAction() const;
  /** @brief Return the QAction for opening a file.
   *  @return Pointer to the QAction
   */
  QAction *openFileAction() const;
  /** @brief Return the QAction for saving the current file.
   *  @return Pointer to the QAction
   */
  QAction *saveFileAction() const;
  /** @brief Return the QAction for saving the current file under a new name.
   *  @return Pointer to the QAction
   */
  QAction *saveFileAsAction() const;
  /** @brief Return the QAction for quitting the application.
   *  @return Pointer to the QAction
   */
  QAction *quitAction() const;
  /** @brief Return the QAction for showing the about dialog.
   *  @return Pointer to the QAction
   */
  QAction *aboutAction() const;

  Q_INVOKABLE void newFile();
  Q_INVOKABLE void openFile();
  Q_INVOKABLE void saveFile();
  Q_INVOKABLE void saveFileAs();

  Q_INVOKABLE void browseImportPdf();
  Q_INVOKABLE void browseExportFile();
  Q_INVOKABLE void browseExportDirectory();

signals:
  /**
   * @brief Emitted when QML requests an import file dialog.
   * @param filter Native file dialog filter string (e.g. "PDF Files (*.pdf)").
   */
  void importBrowseRequested(const QString &filter);

  /**
   * @brief Emitted after a single import file was selected by the user.
   * @param path Absolute path of the selected file.
   */
  void importFileSelected(const QString &path);

  /**
   * @brief Emitted after multiple import files were selected by the user.
   * @param paths List of absolute file paths.
   */
  void importFilesSelected(const QStringList &paths);

  /**
   * @brief Emitted when a file was dropped onto an import target in QML.
   * @param path Absolute path of the dropped file.
   */
  void importFileDropped(const QString &path);

  /**
   * @brief Emitted when multiple files were dropped onto an import target in
   * QML.
   * @param paths List of absolute file paths.
   */
  void importFilesDropped(const QStringList &paths);

  /**
   * @brief Emitted when QML requests an export file dialog.
   * @param filter Native file dialog filter string (e.g. "All Files (*.*)").
   */
  void exportBrowseRequested(const QString &filter);
  void exportDirectoryBrowseRequested(const QString &title);

  /**
   * @brief Emitted after an export path was chosen by the user.
   * @param path Absolute path of the target file.
   */
  void exportFileSelected(const QString &path);
  void exportDirectorySelected(const QString &path);

private:
  QAction *newFileAction_ = nullptr;
  QAction *openFileAction_ = nullptr;
  QAction *saveFileAction_ = nullptr;
  QAction *saveFileAsAction_ = nullptr;
  QAction *quitAction_ = nullptr;
  QAction *aboutAction_ = nullptr;
};

} // namespace ui
