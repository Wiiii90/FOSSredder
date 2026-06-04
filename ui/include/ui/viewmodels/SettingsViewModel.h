/**
 * @file ui/include/ui/viewmodels/SettingsViewModel.h
 * @brief Declares the Settings page view model.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <qqmlintegration.h>

namespace ui {

class Actions;
class LanguageService;
class NavigationState;
class Settings;

/**
 * @brief Owns Settings view navigation and delegates persisted values.
 */
class SettingsViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(SettingsViewModel)

  Q_PROPERTY(int currentCategory READ currentCategory NOTIFY changed)
  Q_PROPERTY(QVariantList categoryRows READ categoryRows NOTIFY changed)
  Q_PROPERTY(
      bool canNavigateCategories READ canNavigateCategories NOTIFY changed)
  Q_PROPERTY(QVariantList languageOptions READ languageOptions NOTIFY changed)
  Q_PROPERTY(int languageIndex READ languageIndex NOTIFY changed)
  Q_PROPERTY(QVariantList themeModeOptions READ themeModeOptions NOTIFY changed)
  Q_PROPERTY(int themeModeIndex READ themeModeIndex NOTIFY changed)

  Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY changed)
  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY changed)
  Q_PROPERTY(bool autosaveOnClose READ autosaveOnClose WRITE setAutosaveOnClose
                 NOTIFY changed)
  Q_PROPERTY(QVariantList autosaveIntervalOptions READ autosaveIntervalOptions
                 NOTIFY changed)
  Q_PROPERTY(
      int autosaveIntervalIndex READ autosaveIntervalIndex NOTIFY changed)
  Q_PROPERTY(QString importDefaultPath READ importDefaultPath WRITE
                 setImportDefaultPath NOTIFY changed)
  Q_PROPERTY(QString importPoppler READ importPoppler WRITE setImportPoppler
                 NOTIFY changed)
  Q_PROPERTY(QString importOpenCv READ importOpenCv WRITE setImportOpenCv NOTIFY
                 changed)
  Q_PROPERTY(QString importTesseract READ importTesseract WRITE
                 setImportTesseract NOTIFY changed)
  Q_PROPERTY(QString importParser READ importParser WRITE setImportParser NOTIFY
                 changed)
  Q_PROPERTY(QString importMatcher READ importMatcher WRITE setImportMatcher
                 NOTIFY changed)
  Q_PROPERTY(QString exportDefaultDirectory READ exportDefaultDirectory WRITE
                 setExportDefaultDirectory NOTIFY changed)
  Q_PROPERTY(int exportArchiveFormat READ exportArchiveFormat WRITE
                 setExportArchiveFormat NOTIFY changed)
  Q_PROPERTY(bool exportIncludeFormulas READ exportIncludeFormulas WRITE
                 setExportIncludeFormulas NOTIFY changed)
  Q_PROPERTY(bool toolbarShowBooking READ toolbarShowBooking WRITE
                 setToolbarShowBooking NOTIFY changed)
  Q_PROPERTY(bool toolbarShowImport READ toolbarShowImport WRITE
                 setToolbarShowImport NOTIFY changed)
  Q_PROPERTY(bool toolbarShowActors READ toolbarShowActors WRITE
                 setToolbarShowActors NOTIFY changed)
  Q_PROPERTY(bool toolbarShowExport READ toolbarShowExport WRITE
                 setToolbarShowExport NOTIFY changed)
  Q_PROPERTY(bool toolbarShowProperties READ toolbarShowProperties WRITE
                 setToolbarShowProperties NOTIFY changed)
  Q_PROPERTY(bool toolbarShowAnalysis READ toolbarShowAnalysis WRITE
                 setToolbarShowAnalysis NOTIFY changed)
  Q_PROPERTY(bool toolbarShowContracts READ toolbarShowContracts WRITE
                 setToolbarShowContracts NOTIFY changed)
  Q_PROPERTY(bool toolbarShowAnnual READ toolbarShowAnnual WRITE
                 setToolbarShowAnnual NOTIFY changed)
  Q_PROPERTY(bool toolbarShowSettings READ toolbarShowSettings WRITE
                 setToolbarShowSettings NOTIFY changed)

public:
  /**
   * @brief Creates the settings view model.
   * @param parent Optional Qt parent object.
   */
  explicit SettingsViewModel(QObject* parent = nullptr);

  /**
   * @brief Sets the shell navigation object used by the settings page.
   * @param value Navigation object or nullptr.
   */
  void setNavigation(NavigationState* value);

  /**
   * @brief Sets the settings state object used for persisted settings values.
   * @param value Settings object or nullptr.
   */
  void setSettings(Settings* value);

  /**
   * @brief Sets shell actions used for file browsing.
   * @param value Actions object or nullptr.
   */
  void setActions(Actions* value);

  /**
   * @brief Sets the language service used to build language options.
   * @param value Language service or nullptr.
   */
  void setLanguageService(LanguageService* value);

  /**
   * @brief Returns the currently selected settings category.
   * @return Category index.
   */
  int currentCategory() const;

  /**
   * @brief Returns settings category rows for the sidebar.
   * @return Category rows.
   */
  QVariantList categoryRows() const;

  /**
   * @brief Returns whether category navigation is available.
   * @return True when navigation state is attached.
   */
  bool canNavigateCategories() const noexcept;

  /**
   * @brief Returns available language options.
   * @return Language option rows.
   */
  QVariantList languageOptions() const;

  /**
   * @brief Returns the selected language option index.
   * @return Index into languageOptions().
   */
  int languageIndex() const;

  /**
   * @brief Returns available theme mode options.
   * @return Theme mode option rows.
   */
  QVariantList themeModeOptions() const;

  /**
   * @brief Returns the selected theme mode index.
   * @return Index into themeModeOptions().
   */
  int themeModeIndex() const;

  /**
   * @brief Returns the configured language code.
   * @return Language code.
   */
  QString language() const;

  /**
   * @brief Updates the configured language code.
   * @param value Language code.
   */
  void setLanguage(const QString& value);

  /**
   * @brief Returns the configured theme mode.
   * @return Theme mode key.
   */
  QString themeMode() const;

  /**
   * @brief Updates the configured theme mode.
   * @param value Theme mode key.
   */
  void setThemeMode(const QString& value);

  /**
   * @brief Returns whether autosave on close is enabled.
   * @return True when autosave on close is enabled.
   */
  bool autosaveOnClose() const;

  /**
   * @brief Updates whether autosave on close is enabled.
   * @param value True to enable autosave on close.
   */
  void setAutosaveOnClose(bool value);

  /**
   * @brief Returns autosave interval options.
   * @return Autosave interval rows.
   */
  QVariantList autosaveIntervalOptions() const;

  /**
   * @brief Returns the selected autosave interval index.
   * @return Index into autosaveIntervalOptions().
   */
  int autosaveIntervalIndex() const;

  /**
   * @brief Returns the default import path.
   * @return Import path text.
   */
  QString importDefaultPath() const;

  /**
   * @brief Updates the default import path.
   * @param value Import path text.
   */
  void setImportDefaultPath(const QString& value);

  /**
   * @brief Returns the configured Poppler path.
   * @return Poppler path text.
   */
  QString importPoppler() const;

  /**
   * @brief Updates the configured Poppler path.
   * @param value Poppler path text.
   */
  void setImportPoppler(const QString& value);

  /**
   * @brief Returns the configured OpenCV path.
   * @return OpenCV path text.
   */
  QString importOpenCv() const;

  /**
   * @brief Updates the configured OpenCV path.
   * @param value OpenCV path text.
   */
  void setImportOpenCv(const QString& value);

  /**
   * @brief Returns the configured Tesseract path.
   * @return Tesseract path text.
   */
  QString importTesseract() const;

  /**
   * @brief Updates the configured Tesseract path.
   * @param value Tesseract path text.
   */
  void setImportTesseract(const QString& value);

  /**
   * @brief Returns the configured import parser key.
   * @return Parser key.
   */
  QString importParser() const;

  /**
   * @brief Updates the configured import parser key.
   * @param value Parser key.
   */
  void setImportParser(const QString& value);

  /**
   * @brief Returns the configured import matcher key.
   * @return Matcher key.
   */
  QString importMatcher() const;

  /**
   * @brief Updates the configured import matcher key.
   * @param value Matcher key.
   */
  void setImportMatcher(const QString& value);

  /**
   * @brief Returns the default export directory.
   * @return Export directory text.
   */
  QString exportDefaultDirectory() const;

  /**
   * @brief Updates the default export directory.
   * @param value Export directory text.
   */
  void setExportDefaultDirectory(const QString& value);

  /**
   * @brief Returns the configured export archive format index.
   * @return Archive format index.
   */
  int exportArchiveFormat() const;

  /**
   * @brief Updates the configured export archive format index.
   * @param value Archive format index.
   */
  void setExportArchiveFormat(int value);

  /**
   * @brief Returns whether exported spreadsheets include formulas.
   * @return True when formulas are included.
   */
  bool exportIncludeFormulas() const;

  /**
   * @brief Updates whether exported spreadsheets include formulas.
   * @param value True to include formulas.
   */
  void setExportIncludeFormulas(bool value);

  /**
   * @brief Returns whether the booking toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowBooking() const;

  /**
   * @brief Updates whether the booking toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowBooking(bool value);

  /**
   * @brief Returns whether the import toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowImport() const;

  /**
   * @brief Updates whether the import toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowImport(bool value);

  /**
   * @brief Returns whether the actor toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowActors() const;

  /**
   * @brief Updates whether the actor toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowActors(bool value);

  /**
   * @brief Returns whether the export toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowExport() const;

  /**
   * @brief Updates whether the export toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowExport(bool value);

  /**
   * @brief Returns whether the property toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowProperties() const;

  /**
   * @brief Updates whether the property toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowProperties(bool value);

  /**
   * @brief Returns whether the analysis toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowAnalysis() const;

  /**
   * @brief Updates whether the analysis toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowAnalysis(bool value);

  /**
   * @brief Returns whether the contract toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowContracts() const;

  /**
   * @brief Updates whether the contract toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowContracts(bool value);

  /**
   * @brief Returns whether the annual toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowAnnual() const;

  /**
   * @brief Updates whether the annual toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowAnnual(bool value);

  /**
   * @brief Returns whether the settings toolbar entry is visible.
   * @return True when visible.
   */
  bool toolbarShowSettings() const;

  /**
   * @brief Updates whether the settings toolbar entry is visible.
   * @param value True when visible.
   */
  void setToolbarShowSettings(bool value);

  /**
   * @brief Activates the settings page state.
   */
  Q_INVOKABLE void activate();

  /**
   * @brief Navigates settings categories by a relative delta.
   * @param delta Relative movement.
   */
  Q_INVOKABLE void navigateCategory(int delta);

  /**
   * @brief Selects a settings category.
   * @param category Category index.
   */
  Q_INVOKABLE void selectCategory(int category);

  /**
   * @brief Persists the current settings values.
   */
  Q_INVOKABLE void saveSettings();

  /**
   * @brief Resets settings values to defaults.
   */
  Q_INVOKABLE void resetSettings();

  /**
   * @brief Selects a language option by index.
   * @param index Index into languageOptions().
   */
  Q_INVOKABLE void selectLanguageAt(int index);

  /**
   * @brief Selects a theme mode option by index.
   * @param index Index into themeModeOptions().
   */
  Q_INVOKABLE void selectThemeModeAt(int index);

  /**
   * @brief Selects an autosave interval option by index.
   * @param index Index into autosaveIntervalOptions().
   */
  Q_INVOKABLE void selectAutosaveIntervalAt(int index);

  /**
   * @brief Opens a browser for the import default path.
   */
  Q_INVOKABLE void browseImportPath();

  /**
   * @brief Opens a browser for the export default directory.
   */
  Q_INVOKABLE void browseExportDirectory();

signals:
  void changed();

private:
  /**
   * @brief Returns the first settings category index.
   * @return First category index.
   */
  int firstCategory() const noexcept;
  /**
   * @brief Returns the last settings category index.
   * @return Last category index.
   */
  int lastCategory() const noexcept;
  /**
   * @brief Connects navigation signals.
   * @param value Navigation state or nullptr.
   */
  void bindNavigation(NavigationState* value);
  /**
   * @brief Connects settings signals.
   * @param value Settings object or nullptr.
   */
  void bindSettings(Settings* value);
  /**
   * @brief Connects shell action signals.
   * @param value Actions object or nullptr.
   */
  void bindActions(Actions* value);
  /**
   * @brief Emits the shared changed signal.
   */
  void emitChanged();

  NavigationState* navigation_ = nullptr;
  Settings* settings_ = nullptr;
  Actions* actions_ = nullptr;
  LanguageService* languageService_ = nullptr;
};

} // namespace ui
