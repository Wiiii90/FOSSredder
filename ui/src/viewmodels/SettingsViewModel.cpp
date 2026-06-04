/**
 * @file ui/src/viewmodels/SettingsViewModel.cpp
 * @brief Implements the QML-facing SettingsViewModel API.
 */

#include "ui/viewmodels/SettingsViewModel.h"

#include "ui/shell/Defaults.h"
#include "ui/platform/LanguageService.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/NavigationState.h"
#include "ui/shell/Settings.h"
#include "ui/observability/Trace.h"

namespace ui {

namespace {

constexpr int kGeneralCategory = 0;
constexpr int kImportCategory = 1;
constexpr int kExportCategory = 2;
constexpr int kMiscellaneousCategory = 3;

QVariantMap categoryRow(int value, const QString &text, bool selected) {
  return {{QStringLiteral("category"), value},
          {QStringLiteral("text"), text},
          {QStringLiteral("selected"), selected}};
}

QString languageCode(const QVariant &value) {
  return value.toMap().value(QStringLiteral("code")).toString();
}

bool languageAvailable(const QVariant &value) {
  const QVariantMap map = value.toMap();
  return !map.contains(QStringLiteral("available")) ||
         map.value(QStringLiteral("available")).toBool();
}

QVariantMap themeModeOption(const QString &code, const QString &label) {
  return {{QStringLiteral("code"), code}, {QStringLiteral("label"), label}};
}

QString themeModeCode(const QVariant &value) {
  return value.toMap().value(QStringLiteral("code")).toString();
}

int autosaveIntervalMinutes(const QVariant &value) {
  return value.toMap().value(QStringLiteral("minutes")).toInt();
}

QVariantMap autosaveIntervalOption(int minutes, const QString &label) {
  return {{QStringLiteral("minutes"), minutes},
          {QStringLiteral("label"), label}};
}

} // namespace

SettingsViewModel::SettingsViewModel(QObject *parent) : QObject(parent) {}

void SettingsViewModel::setNavigation(NavigationState *value) {
  if (navigation_ == value) {
    return;
  }
  bindNavigation(value);
  emitChanged();
}

void SettingsViewModel::setSettings(Settings *value) {
  if (settings_ == value) {
    return;
  }
  bindSettings(value);
  emitChanged();
}

void SettingsViewModel::setActions(Actions *value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emitChanged();
}

void SettingsViewModel::setLanguageService(LanguageService *value) {
  if (languageService_ == value) {
    return;
  }
  if (languageService_) {
    disconnect(languageService_, nullptr, this, nullptr);
  }
  languageService_ = value;
  if (languageService_) {
    connect(languageService_, &LanguageService::currentLanguageChanged, this,
            &SettingsViewModel::emitChanged);
    connect(languageService_, &LanguageService::availableLanguagesChanged, this,
            &SettingsViewModel::emitChanged);
  }
  emitChanged();
}

void SettingsViewModel::bindNavigation(NavigationState *value) {
  if (navigation_) {
    disconnect(navigation_, nullptr, this, nullptr);
  }
  navigation_ = value;
  if (navigation_) {
    connect(navigation_, &NavigationState::settingsCategoryChanged, this,
            &SettingsViewModel::emitChanged);
  }
}

void SettingsViewModel::bindSettings(Settings *value) {
  if (settings_) {
    disconnect(settings_, nullptr, this, nullptr);
  }
  settings_ = value;
  if (settings_) {
    connect(settings_, &Settings::stateChanged, this,
            &SettingsViewModel::emitChanged);
    connect(settings_, &Settings::languageChanged, this,
            &SettingsViewModel::emitChanged);
    connect(settings_, &Settings::themeModeChanged, this,
            &SettingsViewModel::emitChanged);
  }
}

void SettingsViewModel::bindActions(Actions *value) {
  if (actions_) {
    disconnect(actions_, nullptr, this, nullptr);
  }
  actions_ = value;
  if (!actions_) {
    return;
  }
  connect(actions_, &Actions::importFileSelected, this,
          [this](const QString &path) {
            if (!path.isEmpty()) {
              setImportDefaultPath(path);
            }
          });
  connect(actions_, &Actions::importFilesSelected, this,
          [this](const QStringList &paths) {
            if (!paths.isEmpty() && !paths.first().isEmpty()) {
              setImportDefaultPath(paths.first());
            }
          });
  connect(actions_, &Actions::exportDirectorySelected, this,
          [this](const QString &path) {
            if (!path.isEmpty()) {
              setExportDefaultDirectory(path);
            }
          });
}

void SettingsViewModel::emitChanged() { emit changed(); }

int SettingsViewModel::currentCategory() const {
  return navigation_ ? navigation_->settingsCategoryValue() : firstCategory();
}

QVariantList SettingsViewModel::categoryRows() const {
  const int selected = currentCategory();
  return {categoryRow(kGeneralCategory, tr("General"),
                      selected == kGeneralCategory),
          categoryRow(kImportCategory, tr("Import"),
                      selected == kImportCategory),
          categoryRow(kExportCategory, tr("Export"),
                      selected == kExportCategory),
          categoryRow(kMiscellaneousCategory, tr("Miscellaneous"),
                      selected == kMiscellaneousCategory)};
}

bool SettingsViewModel::canNavigateCategories() const noexcept {
  return lastCategory() > firstCategory();
}

void SettingsViewModel::activate() {
  if (settings_) {
    settings_->load();
  }
  if (navigation_) {
    navigation_->setSettingsCategoryValue(currentCategory());
  }
  emitChanged();
}

void SettingsViewModel::navigateCategory(int delta) {
  const int first = firstCategory();
  const int last = lastCategory();
  const int total = last - first + 1;
  if (!navigation_ || total <= 0) {
    return;
  }
  const int offset = currentCategory() - first;
  const int next = first + ((offset + delta + total) % total);
  selectCategory(next);
}

void SettingsViewModel::selectCategory(int category) {
  if (navigation_) {
    navigation_->setSettingsCategoryValue(category);
  }
}

void SettingsViewModel::resetSettings() {
  selectCategory(kGeneralCategory);
  if (settings_) {
    settings_->resetToDefaults();
  }
  emitChanged();
}

int SettingsViewModel::firstCategory() const noexcept {
  return kGeneralCategory;
}

int SettingsViewModel::lastCategory() const noexcept {
  return kMiscellaneousCategory;
}

QVariantList SettingsViewModel::languageOptions() const {
  return languageService_ ? languageService_->availableLanguages()
                          : QVariantList();
}

int SettingsViewModel::languageIndex() const {
  const QString selectedLanguage = language();
  const QVariantList options = languageOptions();
  for (int i = 0; i < options.size(); ++i) {
    if (languageCode(options.at(i)) == selectedLanguage) {
      return i;
    }
  }
  return -1;
}

QVariantList SettingsViewModel::themeModeOptions() const {
  return {themeModeOption(QStringLiteral("light"), tr("Light")),
          themeModeOption(QStringLiteral("dark"), tr("Dark"))};
}

int SettingsViewModel::themeModeIndex() const {
  const QString selectedThemeMode = themeMode();
  const QVariantList options = themeModeOptions();
  for (int i = 0; i < options.size(); ++i) {
    if (themeModeCode(options.at(i)) == selectedThemeMode) {
      return i;
    }
  }
  return 0;
}

QString SettingsViewModel::language() const {
  if (settings_) {
    return settings_->language();
  }
  return languageService_ ? languageService_->currentLanguage() : QString();
}

void SettingsViewModel::setLanguage(const QString &value) {
  if (settings_) {
    settings_->setLanguage(value);
  }
  if (languageService_) {
    languageService_->applyLanguage(value);
  }
}

QString SettingsViewModel::themeMode() const {
  return settings_ ? settings_->themeMode() : QStringLiteral("light");
}

void SettingsViewModel::setThemeMode(const QString &value) {
  if (settings_) {
    settings_->setThemeMode(value);
  }
}

void SettingsViewModel::selectLanguageAt(int index) {
  const QVariantList options = languageOptions();
  if (index < 0 || index >= options.size() ||
      !languageAvailable(options.at(index))) {
    emitChanged();
    return;
  }
  setLanguage(languageCode(options.at(index)));
}

void SettingsViewModel::selectThemeModeAt(int index) {
  const QVariantList options = themeModeOptions();
  if (index < 0 || index >= options.size()) {
    emitChanged();
    return;
  }
  setThemeMode(themeModeCode(options.at(index)));
}

bool SettingsViewModel::autosaveOnClose() const {
  return !settings_ || settings_->autosaveOnClose();
}

void SettingsViewModel::setAutosaveOnClose(bool value) {
  if (settings_) {
    settings_->setAutosaveOnClose(value);
  }
}

QVariantList SettingsViewModel::autosaveIntervalOptions() const {
  return {autosaveIntervalOption(
              config::autosave::kIntervalOff, tr("Off")),
          autosaveIntervalOption(
              config::autosave::kInterval1Minute, tr("Every 1 minute")),
          autosaveIntervalOption(
              config::autosave::kInterval5Minutes, tr("Every 5 minutes")),
          autosaveIntervalOption(
              config::autosave::kInterval10Minutes, tr("Every 10 minutes")),
          autosaveIntervalOption(
              config::autosave::kInterval15Minutes, tr("Every 15 minutes")),
          autosaveIntervalOption(
              config::autosave::kInterval30Minutes, tr("Every 30 minutes"))};
}

int SettingsViewModel::autosaveIntervalIndex() const {
  const int selectedMinutes =
      settings_ ? settings_->autosaveIntervalMinutes()
                  : config::autosave::kIntervalOff;
  const QVariantList options = autosaveIntervalOptions();
  for (int i = 0; i < options.size(); ++i) {
    if (autosaveIntervalMinutes(options.at(i)) == selectedMinutes) {
      return i;
    }
  }
  return 0;
}

void SettingsViewModel::selectAutosaveIntervalAt(int index) {
  const QVariantList options = autosaveIntervalOptions();
  if (!settings_ || index < 0 || index >= options.size()) {
    emitChanged();
    return;
  }
  settings_->setAutosaveIntervalMinutes(
      autosaveIntervalMinutes(options.at(index)));
}

QString SettingsViewModel::importDefaultPath() const {
  return settings_ ? settings_->importDefaultPath() : QString();
}

void SettingsViewModel::setImportDefaultPath(const QString &value) {
  if (settings_) {
    settings_->setImportDefaultPath(value);
  }
}

QString SettingsViewModel::importPoppler() const {
  return settings_ ? settings_->importPoppler() : QString();
}

void SettingsViewModel::setImportPoppler(const QString &value) {
  if (settings_) {
    settings_->setImportPoppler(value);
  }
}

QString SettingsViewModel::importOpenCv() const {
  return settings_ ? settings_->importOpenCv() : QString();
}

void SettingsViewModel::setImportOpenCv(const QString &value) {
  if (settings_) {
    settings_->setImportOpenCv(value);
  }
}

QString SettingsViewModel::importTesseract() const {
  return settings_ ? settings_->importTesseract() : QString();
}

void SettingsViewModel::setImportTesseract(const QString &value) {
  if (settings_) {
    settings_->setImportTesseract(value);
  }
}

QString SettingsViewModel::importParser() const {
  return settings_ ? settings_->importParser() : QString();
}

void SettingsViewModel::setImportParser(const QString &value) {
  if (settings_) {
    settings_->setImportParser(value);
  }
}

QString SettingsViewModel::importMatcher() const {
  return settings_ ? settings_->importMatcher() : QString();
}

void SettingsViewModel::setImportMatcher(const QString &value) {
  if (settings_) {
    settings_->setImportMatcher(value);
  }
}

QString SettingsViewModel::exportDefaultDirectory() const {
  return settings_ ? settings_->exportDefaultDirectory() : QString();
}

void SettingsViewModel::setExportDefaultDirectory(const QString &value) {
  if (settings_) {
    settings_->setExportDefaultDirectory(value);
  }
}

int SettingsViewModel::exportArchiveFormat() const {
  return settings_ ? settings_->exportArchiveFormat() : 0;
}

void SettingsViewModel::setExportArchiveFormat(int value) {
  if (settings_) {
    settings_->setExportArchiveFormat(value);
  }
}

bool SettingsViewModel::exportIncludeFormulas() const {
  return !settings_ || settings_->exportIncludeFormulas();
}

void SettingsViewModel::setExportIncludeFormulas(bool value) {
  if (settings_) {
    settings_->setExportIncludeFormulas(value);
  }
}

bool SettingsViewModel::toolbarShowBooking() const {
  return !settings_ || settings_->toolbarShowBooking();
}

void SettingsViewModel::setToolbarShowBooking(bool value) {
  if (settings_) {
    settings_->setToolbarShowBooking(value);
  }
}

bool SettingsViewModel::toolbarShowImport() const {
  return !settings_ || settings_->toolbarShowImport();
}

void SettingsViewModel::setToolbarShowImport(bool value) {
  if (settings_) {
    settings_->setToolbarShowImport(value);
  }
}

bool SettingsViewModel::toolbarShowActors() const {
  return !settings_ || settings_->toolbarShowActors();
}

void SettingsViewModel::setToolbarShowActors(bool value) {
  if (settings_) {
    settings_->setToolbarShowActors(value);
  }
}

bool SettingsViewModel::toolbarShowExport() const {
  return !settings_ || settings_->toolbarShowExport();
}

void SettingsViewModel::setToolbarShowExport(bool value) {
  if (settings_) {
    settings_->setToolbarShowExport(value);
  }
}

bool SettingsViewModel::toolbarShowProperties() const {
  return !settings_ || settings_->toolbarShowProperties();
}

void SettingsViewModel::setToolbarShowProperties(bool value) {
  if (settings_) {
    settings_->setToolbarShowProperties(value);
  }
}

bool SettingsViewModel::toolbarShowAnalysis() const {
  return !settings_ || settings_->toolbarShowAnalysis();
}

void SettingsViewModel::setToolbarShowAnalysis(bool value) {
  if (settings_) {
    settings_->setToolbarShowAnalysis(value);
  }
}

bool SettingsViewModel::toolbarShowContracts() const {
  return !settings_ || settings_->toolbarShowContracts();
}

void SettingsViewModel::setToolbarShowContracts(bool value) {
  if (settings_) {
    settings_->setToolbarShowContracts(value);
  }
}

bool SettingsViewModel::toolbarShowAnnual() const {
  return !settings_ || settings_->toolbarShowAnnual();
}

void SettingsViewModel::setToolbarShowAnnual(bool value) {
  if (settings_) {
    settings_->setToolbarShowAnnual(value);
  }
}

bool SettingsViewModel::toolbarShowSettings() const {
  return !settings_ || settings_->toolbarShowSettings();
}

void SettingsViewModel::setToolbarShowSettings(bool value) {
  if (settings_) {
    settings_->setToolbarShowSettings(value);
  }
}

void SettingsViewModel::saveSettings() {
  observability::traceViewModel("SettingsViewModel::saveSettings",
                                "Settings save submitted");
  if (settings_) {
    settings_->save();
  }
  if (languageService_ && settings_) {
    languageService_->applyLanguage(settings_->language());
  }
  emitChanged();
}

void SettingsViewModel::browseImportPath() {
  if (actions_) {
    actions_->browseImportPdf();
  }
}

void SettingsViewModel::browseExportDirectory() {
  if (actions_) {
    actions_->browseExportDirectory();
  }
}

} // namespace ui
