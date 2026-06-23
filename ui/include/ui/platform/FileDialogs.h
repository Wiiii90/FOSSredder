/**
 * @file ui/include/ui/platform/FileDialogs.h
 * @brief Declares shared native file dialog helpers used by the desktop UI.
 */

#pragma once

#include <QString>
#include <QStringList>

class QWidget;

namespace ui::dialogs {

/**
 * @brief Opens a multi-select file dialog for statement import files.
 * @param parent Parent widget for the dialog.
 * @param filter Native file dialog filter string.
 * @param initialPath Preferred initial file or directory path.
 * @return List of selected file paths.
 */
QStringList pickImportFiles(QWidget* parent, const QString& filter,
                            const QString& initialPath = {});

/**
 * @brief Opens a save dialog for export targets.
 * @param parent Parent widget for the dialog.
 * @param filter Native file dialog filter string.
 * @param initialPath Preferred initial file or directory path.
 * @return Chosen export file path or empty when cancelled.
 */
QString pickExportFile(QWidget* parent, const QString& filter,
                       const QString& initialPath = {});

/**
 * @brief Opens a directory picker for export targets.
 * @param parent Parent widget for the dialog.
 * @param title Native dialog title.
 * @param initialPath Preferred initial directory path.
 * @return Chosen export directory path or empty when cancelled.
 */
QString pickExportDirectory(QWidget* parent, const QString& title,
                            const QString& initialPath = {});

/**
 * @brief Opens a save dialog for creating a new storage database.
 * @param parent Parent widget for the dialog.
 * @param initialPath Preferred initial file or directory path.
 * @return Chosen storage file path or empty when cancelled.
 */
QString pickNewStorageFile(QWidget* parent, const QString& initialPath = {});

/**
 * @brief Opens a file dialog for selecting an existing storage database.
 * @param parent Parent widget for the dialog.
 * @param initialPath Preferred initial file or directory path.
 * @return Selected storage file path or empty when cancelled.
 */
QString pickOpenStorageFile(QWidget* parent, const QString& initialPath = {});

/**
 * @brief Opens a save dialog for choosing a database target path.
 * @param parent Parent widget for the dialog.
 * @param initialPath Preferred initial file or directory path.
 * @return Chosen storage file path or empty when cancelled.
 */
QString pickSaveStorageFileAs(QWidget* parent, const QString& initialPath = {});

} // namespace ui::dialogs
