/**
 * @file ui/include/ui/shell/Defaults.h
 * @brief Defines UI-specific runtime constants that are not shared with `core`.
 */

#pragma once

#include <QString>

namespace ui::config {

/** @brief Human-readable application name shown by the UI shell. */
inline const auto kApplicationDisplayName = QStringLiteral("FOSSredder");
/** @brief Resource URL of the application icon embedded into the executable. */
inline const auto kAppIconResource =
    QStringLiteral(":/app/icons/fossredder.png");

/** @brief Default width of the widget-hosted desktop main window. */
constexpr int kMainWindowDefaultWidth = 1200;
/** @brief Default height of the widget-hosted desktop main window. */
constexpr int kMainWindowDefaultHeight = 800;
/** @brief Pixel size used for the About dialog application icon. */
constexpr int kAboutDialogIconSize = 96;

/** @brief Deployment directory containing filesystem QML modules next to the
 * executable. */
inline const auto kAppQmlDirName = QStringLiteral("qml");
/** @brief Deployment directory containing Qt image format plugins next to the
 * executable. */
inline const auto kImageFormatsDirName = QStringLiteral("imageformats");
/** @brief Deployment directory containing compiled translation catalogs. */
inline const auto kTranslationsDirName = QStringLiteral("i18n");
/** @brief Resource prefix used when loading bundled translation catalogs from
 * the Qt resource system. */
inline const auto kTranslationResourcePrefix = QStringLiteral(":/i18n/");
/** @brief Basename prefix for compiled translation catalogs such as
 * `fossredder_de.qm`. */
inline const auto kTranslationBaseName = QStringLiteral("fossredder");
/** @brief Qt Quick Controls style used by the desktop/QML test runtime. */
inline const auto kQtStyle = QStringLiteral("Fusion");

/** @brief Organization name used for the UI's QSettings store. */
inline const auto kSettingsOrganizationName = QStringLiteral("FOSSredder");
/** @brief Application name used for the UI's QSettings store. */
inline const auto kSettingsApplicationName = QStringLiteral("FOSSredder");

namespace languages {

inline const auto kEnglish = QStringLiteral("en");
inline const auto kGerman = QStringLiteral("de");
inline const auto kFrench = QStringLiteral("fr");

} // namespace languages

namespace preferences::keys {

inline const auto kLanguage = QStringLiteral("general/language");
inline const auto kThemeMode = QStringLiteral("general/themeMode");
inline const auto kAutosaveOnClose = QStringLiteral("general/autosaveOnClose");
inline const auto kAutosaveIntervalMinutes =
    QStringLiteral("general/autosaveIntervalMinutes");
inline const auto kImportDefaultPath = QStringLiteral("import/defaultPath");
inline const auto kImportPoppler = QStringLiteral("import/poppler");
inline const auto kImportOpenCv = QStringLiteral("import/opencv");
inline const auto kImportTesseract = QStringLiteral("import/tesseract");
inline const auto kImportParser = QStringLiteral("import/parser");
inline const auto kImportMatcher = QStringLiteral("import/matcher");
inline const auto kExportDefaultDirectory =
    QStringLiteral("export/defaultDirectory");
inline const auto kExportArchiveFormat = QStringLiteral("export/archiveFormat");
inline const auto kExportIncludeFormulas =
    QStringLiteral("export/includeFormulas");
inline const auto kAnalysisDefaultDateMode =
    QStringLiteral("analysis/defaultDateMode");
inline const auto kAnalysisDefaultYear = QStringLiteral("analysis/defaultYear");
inline const auto kToolbarShowBooking =
    QStringLiteral("workspace/toolbar/showBooking");
inline const auto kToolbarShowActors =
    QStringLiteral("workspace/toolbar/showActors");
inline const auto kToolbarShowProperties =
    QStringLiteral("workspace/toolbar/showProperties");
inline const auto kToolbarShowContracts =
    QStringLiteral("workspace/toolbar/showContracts");
inline const auto kToolbarShowImport =
    QStringLiteral("workspace/toolbar/showImport");
inline const auto kToolbarShowExport =
    QStringLiteral("workspace/toolbar/showExport");
inline const auto kToolbarShowAnalysis =
    QStringLiteral("workspace/toolbar/showAnalysis");
inline const auto kToolbarShowAnnual =
    QStringLiteral("workspace/toolbar/showAnnual");
inline const auto kToolbarShowSettings =
    QStringLiteral("workspace/toolbar/showSettings");

} // namespace preferences::keys

namespace operationKeys {

/** @brief Operation key emitted for create-new-file actions. */
inline const auto kNewFile = QStringLiteral("newFile");

/** @brief Operation key emitted for open-file actions. */
inline const auto kOpenFile = QStringLiteral("openFile");

/** @brief Operation key emitted for save-file actions. */
inline const auto kSaveFile = QStringLiteral("saveFile");

/** @brief Operation key emitted for save-as actions. */
inline const auto kSaveFileAs = QStringLiteral("saveFileAs");

} // namespace operationKeys

namespace importProgress {

/** @brief Minimum visible progress value shown by the import UI. */
constexpr double kMinimum = 0.0;
/** @brief Initial progress value shown once an import has started. */
constexpr double kInitial = 0.01;
/** @brief Terminal progress value shown after a successful import. */
constexpr double kMaximum = 1.0;

} // namespace importProgress

namespace autosave {

/** @brief Allowed periodic autosave intervals in minutes; 0 means disabled. */
inline constexpr int kIntervalOff = 0;
inline constexpr int kInterval1Minute = 1;
inline constexpr int kInterval5Minutes = 5;
inline constexpr int kInterval10Minutes = 10;
inline constexpr int kInterval15Minutes = 15;
inline constexpr int kInterval30Minutes = 30;

} // namespace autosave

namespace importPaging {

/** @brief Sentinel value used when no page information is currently available.
 */
constexpr int kNone = 0;

} // namespace importPaging

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

} // namespace objectNames

} // namespace ui::config
