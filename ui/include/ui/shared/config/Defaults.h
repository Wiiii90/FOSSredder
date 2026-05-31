/**
 * @file ui/include/ui/config/Defaults.h
 * @brief Defines UI-specific runtime constants that are not shared with `core`.
 */

#pragma once

#include <QString>

namespace ui::config {

/** @brief Resource URL of the root QML document loaded into the main window. */
inline const auto kMainQmlSource = QStringLiteral("qrc:/qml/FossRedder/Main.qml");
/** @brief Resource URL of the application icon embedded into the executable. */
inline const auto kAppIconResource = QStringLiteral(":/app/icons/fossredder.png");

/** @brief Default width of the widget-hosted desktop main window. */
constexpr int kMainWindowDefaultWidth = 1200;
/** @brief Default height of the widget-hosted desktop main window. */
constexpr int kMainWindowDefaultHeight = 800;
/** @brief Pixel size used for the About dialog application icon. */
constexpr int kAboutDialogIconSize = 96;

/** @brief Deployment directory containing filesystem QML modules next to the executable. */
inline const auto kAppQmlDirName = QStringLiteral("qml");
/** @brief Deployment directory containing Qt image format plugins next to the executable. */
inline const auto kImageFormatsDirName = QStringLiteral("imageformats");
/** @brief Deployment directory containing compiled translation catalogs. */
inline const auto kTranslationsDirName = QStringLiteral("i18n");
/** @brief Resource prefix used when loading bundled translation catalogs from the Qt resource system. */
inline const auto kTranslationResourcePrefix = QStringLiteral(":/i18n/");
/** @brief Basename prefix for compiled translation catalogs such as `fossredder_de.qm`. */
inline const auto kTranslationBaseName = QStringLiteral("fossredder");
/** @brief Resource import path used by the QQmlEngine for QRC-backed modules. */
inline const auto kQrcQmlImportPath = QStringLiteral("qrc:/qml");

namespace operationKeys {

/** @brief Operation key emitted for create-new-file actions. */
inline const auto kNewFile = QStringLiteral("newFile");

/** @brief Operation key emitted for open-file actions. */
inline const auto kOpenFile = QStringLiteral("openFile");

/** @brief Operation key emitted for save-file actions. */
inline const auto kSaveFile = QStringLiteral("saveFile");

/** @brief Operation key emitted for save-as actions. */
inline const auto kSaveFileAs = QStringLiteral("saveFileAs");

}

namespace errorCodes {

inline constexpr auto kExportRunnerUnavailable = "UI_EXPORT_RUNNER_UNAVAILABLE";

}

namespace importProgress {

/** @brief Minimum visible progress value shown by the import UI. */
constexpr double kMinimum = 0.0;
/** @brief Initial progress value shown once an import has started. */
constexpr double kInitial = 0.01;
/** @brief Terminal progress value shown after a successful import. */
constexpr double kMaximum = 1.0;

}

namespace importPaging {

/** @brief Sentinel value used when no page information is currently available. */
constexpr int kNone = 0;

}

/** @brief Regex pattern extracting current and total page counters from status text. */
inline const auto kImportProgressPagePattern = QStringLiteral("\\[(\\d+)\\s*/\\s*(\\d+)\\]");

namespace objectNames {

/** @brief Object name assigned to the main window's "new file" QAction. */
inline constexpr auto kActionNewFile = "actionNewFile";
/** @brief Object name assigned to the main window's "open file" QAction. */
inline constexpr auto kActionOpenFile = "actionOpenFile";
/** @brief Object name assigned to the main window's "save file" QAction. */
inline constexpr auto kActionSaveFile = "actionSaveFile";
/** @brief Object name assigned to the main window's "save as" QAction. */
inline constexpr auto kActionSaveFileAs = "actionSaveFileAs";
/** @brief Object name assigned to the main window's "quit" QAction. */
inline constexpr auto kActionQuit = "actionQuit";
/** @brief Object name assigned to the main window's "about" QAction. */
inline constexpr auto kActionAbout = "actionAbout";

}

}
