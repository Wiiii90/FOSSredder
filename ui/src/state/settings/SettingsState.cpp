/**
 * @file ui/src/state/settings/SettingsState.cpp
 * @brief Implements the UI state adapter for Settings views.
 */

#include "ui/state/settings/SettingsState.h"

#include <QStringList>
#include <QVariantMap>

#include "ui/platform/localization/LanguageService.h"
#include "ui/shell/AppActions.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"

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

} // namespace

SettingsState::SettingsState(QObject *parent) : QObject(parent) {}

void SettingsState::setNavigation(NavigationState *value) {
  if (navigation_ == value) {
    return;
  }
  bindNavigation(value);
  emitChanged();
}

void SettingsState::setSettingsViewModel(SettingsViewModel *value) {
  if (settingsViewModel_ == value) {
    return;
  }
  bindSettings(value);
  emitChanged();
}

void SettingsState::setActions(Actions *value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emitChanged();
}

void SettingsState::setLanguageService(LanguageService *value) {
  if (languageService_ == value) {
    return;
  }
  if (languageService_) {
    disconnect(languageService_, nullptr, this, nullptr);
  }
  languageService_ = value;
  if (languageService_) {
    connect(languageService_, &LanguageService::currentLanguageChanged, this,
            &SettingsState::emitChanged);
  }
  emitChanged();
}

int SettingsState::currentCategory() const {
  return navigation_ ? navigation_->settingsCategoryValue() : firstCategory();
}

QVariantList SettingsState::categoryRows() const {
  const int selected = currentCategory();
  return {categoryRow(kGeneralCategory, tr("General"), selected == kGeneralCategory),
          categoryRow(kImportCategory, tr("Import"), selected == kImportCategory),
          categoryRow(kExportCategory, tr("Export"), selected == kExportCategory),
          categoryRow(kMiscellaneousCategory, tr("Miscellaneous"),
                      selected == kMiscellaneousCategory)};
}

bool SettingsState::canNavigateCategories() const noexcept {
  return lastCategory() > firstCategory();
}

QVariantList SettingsState::languageOptions() const {
  return languageService_ ? languageService_->availableLanguages() : QVariantList();
}

int SettingsState::languageIndex() const {
  const QString selectedLanguage = language();
  const QVariantList options = languageOptions();
  for (int i = 0; i < options.size(); ++i) {
    if (languageCode(options.at(i)) == selectedLanguage) {
      return i;
    }
  }
  return -1;
}

QString SettingsState::language() const {
  if (settingsViewModel_) {
    return settingsViewModel_->language();
  }
  return languageService_ ? languageService_->currentLanguage() : QString();
}

void SettingsState::setLanguage(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setLanguage(value);
  }
  if (languageService_) {
    languageService_->applyLanguage(value);
  }
}

QString SettingsState::importDefaultPath() const {
  return settingsViewModel_ ? settingsViewModel_->importDefaultPath() : QString();
}

void SettingsState::setImportDefaultPath(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportDefaultPath(value);
  }
}

QString SettingsState::importPoppler() const {
  return settingsViewModel_ ? settingsViewModel_->importPoppler() : QString();
}

void SettingsState::setImportPoppler(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportPoppler(value);
  }
}

QString SettingsState::importOpenCv() const {
  return settingsViewModel_ ? settingsViewModel_->importOpenCv() : QString();
}

void SettingsState::setImportOpenCv(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportOpenCv(value);
  }
}

QString SettingsState::importTesseract() const {
  return settingsViewModel_ ? settingsViewModel_->importTesseract() : QString();
}

void SettingsState::setImportTesseract(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportTesseract(value);
  }
}

QString SettingsState::importParser() const {
  return settingsViewModel_ ? settingsViewModel_->importParser() : QString();
}

void SettingsState::setImportParser(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportParser(value);
  }
}

QString SettingsState::importMatcher() const {
  return settingsViewModel_ ? settingsViewModel_->importMatcher() : QString();
}

void SettingsState::setImportMatcher(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setImportMatcher(value);
  }
}

QString SettingsState::exportDefaultDirectory() const {
  return settingsViewModel_ ? settingsViewModel_->exportDefaultDirectory()
                            : QString();
}

void SettingsState::setExportDefaultDirectory(const QString &value) {
  if (settingsViewModel_) {
    settingsViewModel_->setExportDefaultDirectory(value);
  }
}

int SettingsState::exportArchiveFormat() const {
  return settingsViewModel_ ? settingsViewModel_->exportArchiveFormat() : 0;
}

void SettingsState::setExportArchiveFormat(int value) {
  if (settingsViewModel_) {
    settingsViewModel_->setExportArchiveFormat(value);
  }
}

bool SettingsState::exportIncludeFormulas() const {
  return !settingsViewModel_ || settingsViewModel_->exportIncludeFormulas();
}

void SettingsState::setExportIncludeFormulas(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setExportIncludeFormulas(value);
  }
}

bool SettingsState::toolbarShowBooking() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowBooking();
}

void SettingsState::setToolbarShowBooking(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowBooking(value);
  }
}

bool SettingsState::toolbarShowImport() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowImport();
}

void SettingsState::setToolbarShowImport(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowImport(value);
  }
}

bool SettingsState::toolbarShowActors() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowActors();
}

void SettingsState::setToolbarShowActors(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowActors(value);
  }
}

bool SettingsState::toolbarShowExport() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowExport();
}

void SettingsState::setToolbarShowExport(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowExport(value);
  }
}

bool SettingsState::toolbarShowProperties() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowProperties();
}

void SettingsState::setToolbarShowProperties(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowProperties(value);
  }
}

bool SettingsState::toolbarShowAnalysis() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowAnalysis();
}

void SettingsState::setToolbarShowAnalysis(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowAnalysis(value);
  }
}

bool SettingsState::toolbarShowContracts() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowContracts();
}

void SettingsState::setToolbarShowContracts(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowContracts(value);
  }
}

bool SettingsState::toolbarShowAnnual() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowAnnual();
}

void SettingsState::setToolbarShowAnnual(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowAnnual(value);
  }
}

bool SettingsState::toolbarShowSettings() const {
  return !settingsViewModel_ || settingsViewModel_->toolbarShowSettings();
}

void SettingsState::setToolbarShowSettings(bool value) {
  if (settingsViewModel_) {
    settingsViewModel_->setToolbarShowSettings(value);
  }
}

void SettingsState::activate() {
  if (settingsViewModel_) {
    settingsViewModel_->load();
  }
  if (navigation_) {
    navigation_->setSettingsCategoryValue(currentCategory());
  }
  emitChanged();
}

void SettingsState::navigateCategory(int delta) {
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

void SettingsState::selectCategory(int category) {
  if (navigation_) {
    navigation_->setSettingsCategoryValue(category);
  }
}

void SettingsState::saveSettings() {
  if (settingsViewModel_) {
    settingsViewModel_->save();
  }
  if (languageService_ && settingsViewModel_) {
    languageService_->applyLanguage(settingsViewModel_->language());
  }
  emitChanged();
}

void SettingsState::resetSettings() {
  selectCategory(kGeneralCategory);
  if (settingsViewModel_) {
    settingsViewModel_->resetToDefaults();
  }
  emitChanged();
}

void SettingsState::selectLanguageAt(int index) {
  const QVariantList options = languageOptions();
  if (index < 0 || index >= options.size() || !languageAvailable(options.at(index))) {
    emitChanged();
    return;
  }
  setLanguage(languageCode(options.at(index)));
}

void SettingsState::browseImportPath() {
  if (actions_) {
    actions_->browseImportPdf();
  }
}

void SettingsState::browseExportDirectory() {
  if (actions_) {
    actions_->browseExportDirectory();
  }
}

int SettingsState::firstCategory() const noexcept { return kGeneralCategory; }

int SettingsState::lastCategory() const noexcept { return kMiscellaneousCategory; }

void SettingsState::bindNavigation(NavigationState *value) {
  if (navigation_) {
    disconnect(navigation_, nullptr, this, nullptr);
  }
  navigation_ = value;
  if (navigation_) {
    connect(navigation_, &NavigationState::settingsCategoryChanged, this,
            &SettingsState::emitChanged);
  }
}

void SettingsState::bindSettings(SettingsViewModel *value) {
  if (settingsViewModel_) {
    disconnect(settingsViewModel_, nullptr, this, nullptr);
  }
  settingsViewModel_ = value;
  if (settingsViewModel_) {
    connect(settingsViewModel_, &SettingsViewModel::stateChanged, this,
            &SettingsState::emitChanged);
    connect(settingsViewModel_, &SettingsViewModel::languageChanged, this,
            &SettingsState::emitChanged);
  }
}

void SettingsState::bindActions(Actions *value) {
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

void SettingsState::emitChanged() { emit changed(); }

} // namespace ui
