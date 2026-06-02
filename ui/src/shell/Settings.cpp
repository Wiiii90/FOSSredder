/**
 * @file ui/src/shell/Settings.cpp
 * @brief Implements the centralized UI settings view model exposed to QML.
 */

#include "ui/shell/Settings.h"

#include <QDate>
#include <QSettings>

#include "ui/shared/config/Defaults.h"

namespace ui {

namespace languages = config::languages;
namespace preferenceKeys = config::preferences::keys;

namespace {

QSettings openSettings() {
  return QSettings(QSettings::NativeFormat, QSettings::UserScope,
                   ui::config::kSettingsOrganizationName,
                   ui::config::kSettingsApplicationName);
}

} // namespace

Settings::Settings(QObject *parent) : QObject(parent) {
  load();
}

QString Settings::normalizeText(const QString &value) {
  return value.trimmed();
}

int Settings::normalizeArchiveFormat(int value) noexcept {
  return value == 1 ? 1 : 0;
}

QString Settings::normalizeThemeMode(const QString &value) {
  const QString normalized = value.trimmed().toLower();
  return normalized == QStringLiteral("dark") ? QStringLiteral("dark")
                                              : QStringLiteral("light");
}

QString normalizeAnalysisDateMode(const QString &value) {
  const QString normalized = value.trimmed().toLower();
  return normalized == QStringLiteral("range") ? QStringLiteral("range")
                                               : QStringLiteral("year");
}

int normalizeAnalysisYear(int value) noexcept {
  return value > 0 ? value : (QDate::currentDate().year() - 1);
}

void Settings::emitStateChanged() { emit stateChanged(); }

Settings::Values Settings::defaultValues() {
  Values out;
  out.language = languages::kEnglish;
  out.themeMode = QStringLiteral("light");
  out.exportIncludeFormulas = true;
  out.analysisDefaultDateMode = QStringLiteral("year");
  out.analysisDefaultYear = QDate::currentDate().year() - 1;
  return out;
}

void Settings::applyDefaults() { values_ = defaultValues(); }

void Settings::loadFromPersistentStore() {
  auto settings = openSettings();
  values_.language = normalizeText(
      settings.value(preferenceKeys::kLanguage, languages::kEnglish).toString());
  if (values_.language.isEmpty())
    values_.language = languages::kEnglish;
  values_.themeMode = normalizeThemeMode(
      settings.value(preferenceKeys::kThemeMode, QStringLiteral("light"))
          .toString());
  values_.importDefaultPath = normalizeText(
      settings.value(preferenceKeys::kImportDefaultPath).toString());
  values_.importPoppler =
      settings.value(preferenceKeys::kImportPoppler).toString();
  values_.importOpenCv =
      settings.value(preferenceKeys::kImportOpenCv).toString();
  values_.importTesseract =
      settings.value(preferenceKeys::kImportTesseract).toString();
  values_.importParser =
      settings.value(preferenceKeys::kImportParser).toString();
  values_.importMatcher =
      settings.value(preferenceKeys::kImportMatcher).toString();
  values_.exportDefaultDirectory = normalizeText(
      settings.value(preferenceKeys::kExportDefaultDirectory).toString());
  values_.exportArchiveFormat = normalizeArchiveFormat(
      settings.value(preferenceKeys::kExportArchiveFormat, 0).toInt());
  values_.exportIncludeFormulas =
      settings.value(preferenceKeys::kExportIncludeFormulas, true).toBool();
  values_.analysisDefaultDateMode = normalizeAnalysisDateMode(
      settings.value(preferenceKeys::kAnalysisDefaultDateMode,
                     QStringLiteral("year"))
          .toString());
  values_.analysisDefaultYear = normalizeAnalysisYear(
      settings.value(preferenceKeys::kAnalysisDefaultYear,
                     QDate::currentDate().year() - 1)
          .toInt());
  values_.toolbarShowBooking =
      settings.value(preferenceKeys::kToolbarShowBooking, true).toBool();
  values_.toolbarShowActors =
      settings.value(preferenceKeys::kToolbarShowActors, true).toBool();
  values_.toolbarShowProperties =
      settings.value(preferenceKeys::kToolbarShowProperties, true).toBool();
  values_.toolbarShowContracts =
      settings.value(preferenceKeys::kToolbarShowContracts, true).toBool();
  values_.toolbarShowImport =
      settings.value(preferenceKeys::kToolbarShowImport, true).toBool();
  values_.toolbarShowExport =
      settings.value(preferenceKeys::kToolbarShowExport, true).toBool();
  values_.toolbarShowAnalysis =
      settings.value(preferenceKeys::kToolbarShowAnalysis, true).toBool();
  values_.toolbarShowAnnual =
      settings.value(preferenceKeys::kToolbarShowAnnual, true).toBool();
  values_.toolbarShowSettings =
      settings.value(preferenceKeys::kToolbarShowSettings, true).toBool();
}

void Settings::persistToStore() const {
  auto settings = openSettings();
  settings.setValue(preferenceKeys::kLanguage, values_.language);
  settings.setValue(preferenceKeys::kThemeMode, values_.themeMode);
  settings.setValue(preferenceKeys::kImportDefaultPath,
                    values_.importDefaultPath);
  settings.setValue(preferenceKeys::kImportPoppler, values_.importPoppler);
  settings.setValue(preferenceKeys::kImportOpenCv, values_.importOpenCv);
  settings.setValue(preferenceKeys::kImportTesseract,
                    values_.importTesseract);
  settings.setValue(preferenceKeys::kImportParser, values_.importParser);
  settings.setValue(preferenceKeys::kImportMatcher, values_.importMatcher);
  settings.setValue(preferenceKeys::kExportDefaultDirectory,
                    values_.exportDefaultDirectory);
  settings.setValue(preferenceKeys::kExportArchiveFormat,
                    values_.exportArchiveFormat);
  settings.setValue(preferenceKeys::kExportIncludeFormulas,
                    values_.exportIncludeFormulas);
  settings.setValue(preferenceKeys::kAnalysisDefaultDateMode,
                    values_.analysisDefaultDateMode);
  settings.setValue(preferenceKeys::kAnalysisDefaultYear,
                    values_.analysisDefaultYear);
  settings.setValue(preferenceKeys::kToolbarShowBooking,
                    values_.toolbarShowBooking);
  settings.setValue(preferenceKeys::kToolbarShowActors,
                    values_.toolbarShowActors);
  settings.setValue(preferenceKeys::kToolbarShowProperties,
                    values_.toolbarShowProperties);
  settings.setValue(preferenceKeys::kToolbarShowContracts,
                    values_.toolbarShowContracts);
  settings.setValue(preferenceKeys::kToolbarShowImport,
                    values_.toolbarShowImport);
  settings.setValue(preferenceKeys::kToolbarShowExport,
                    values_.toolbarShowExport);
  settings.setValue(preferenceKeys::kToolbarShowAnalysis,
                    values_.toolbarShowAnalysis);
  settings.setValue(preferenceKeys::kToolbarShowAnnual,
                    values_.toolbarShowAnnual);
  settings.setValue(preferenceKeys::kToolbarShowSettings,
                    values_.toolbarShowSettings);
  settings.sync();
}

void Settings::captureSavedState() { savedValues_ = values_; }

void Settings::emitAllValueChanged() {
  emit languageChanged();
  emit themeModeChanged();
  emit importDefaultPathChanged();
  emit importPopplerChanged();
  emit importOpenCvChanged();
  emit importTesseractChanged();
  emit importParserChanged();
  emit importMatcherChanged();
  emit exportDefaultDirectoryChanged();
  emit exportArchiveFormatChanged();
  emit exportIncludeFormulasChanged();
  emit analysisDefaultDateModeChanged();
  emit analysisDefaultYearChanged();
  emit toolbarShowBookingChanged();
  emit toolbarShowActorsChanged();
  emit toolbarShowPropertiesChanged();
  emit toolbarShowContractsChanged();
  emit toolbarShowImportChanged();
  emit toolbarShowExportChanged();
  emit toolbarShowAnalysisChanged();
  emit toolbarShowAnnualChanged();
  emit toolbarShowSettingsChanged();
}

void Settings::setLanguage(const QString &value) {
  const QString normalized = normalizeText(value).toLower();
  const QString nextValue =
      normalized.isEmpty() ? languages::kEnglish : normalized;
  updateSetting(values_.language, nextValue, &Settings::languageChanged);
}

void Settings::setThemeMode(const QString &value) {
  updateSetting(values_.themeMode, normalizeThemeMode(value),
                &Settings::themeModeChanged);
}

void Settings::setImportDefaultPath(const QString &value) {
  updateSetting(values_.importDefaultPath, normalizeText(value),
                &Settings::importDefaultPathChanged);
}

void Settings::setImportPoppler(const QString &value) {
  updateSetting(values_.importPoppler, value,
                &Settings::importPopplerChanged);
}

void Settings::setImportOpenCv(const QString &value) {
  updateSetting(values_.importOpenCv, value,
                &Settings::importOpenCvChanged);
}

void Settings::setImportTesseract(const QString &value) {
  updateSetting(values_.importTesseract, value,
                &Settings::importTesseractChanged);
}

void Settings::setImportParser(const QString &value) {
  updateSetting(values_.importParser, value,
                &Settings::importParserChanged);
}

void Settings::setImportMatcher(const QString &value) {
  updateSetting(values_.importMatcher, value,
                &Settings::importMatcherChanged);
}

void Settings::setExportDefaultDirectory(const QString &value) {
  updateSetting(values_.exportDefaultDirectory, normalizeText(value),
                &Settings::exportDefaultDirectoryChanged);
}

void Settings::setExportArchiveFormat(int value) {
  updateSetting(values_.exportArchiveFormat, normalizeArchiveFormat(value),
                &Settings::exportArchiveFormatChanged);
}

void Settings::setExportIncludeFormulas(bool value) {
  updateSetting(values_.exportIncludeFormulas, value,
                &Settings::exportIncludeFormulasChanged);
}

void Settings::setAnalysisDefaultDateMode(const QString &value) {
  updateSetting(values_.analysisDefaultDateMode, normalizeAnalysisDateMode(value),
                &Settings::analysisDefaultDateModeChanged);
}

void Settings::setAnalysisDefaultYear(int value) {
  updateSetting(values_.analysisDefaultYear, normalizeAnalysisYear(value),
                &Settings::analysisDefaultYearChanged);
}

void Settings::setToolbarShowBooking(bool value) {
  updateSetting(values_.toolbarShowBooking, value,
                &Settings::toolbarShowBookingChanged);
}

void Settings::setToolbarShowActors(bool value) {
  updateSetting(values_.toolbarShowActors, value,
                &Settings::toolbarShowActorsChanged);
}

void Settings::setToolbarShowProperties(bool value) {
  updateSetting(values_.toolbarShowProperties, value,
                &Settings::toolbarShowPropertiesChanged);
}

void Settings::setToolbarShowContracts(bool value) {
  updateSetting(values_.toolbarShowContracts, value,
                &Settings::toolbarShowContractsChanged);
}

void Settings::setToolbarShowImport(bool value) {
  updateSetting(values_.toolbarShowImport, value,
                &Settings::toolbarShowImportChanged);
}

void Settings::setToolbarShowExport(bool value) {
  updateSetting(values_.toolbarShowExport, value,
                &Settings::toolbarShowExportChanged);
}

void Settings::setToolbarShowAnalysis(bool value) {
  updateSetting(values_.toolbarShowAnalysis, value,
                &Settings::toolbarShowAnalysisChanged);
}

void Settings::setToolbarShowAnnual(bool value) {
  updateSetting(values_.toolbarShowAnnual, value,
                &Settings::toolbarShowAnnualChanged);
}

void Settings::setToolbarShowSettings(bool value) {
  updateSetting(values_.toolbarShowSettings, value,
                &Settings::toolbarShowSettingsChanged);
}

void Settings::load() {
  applyDefaults();
  loadFromPersistentStore();
  captureSavedState();
  emit stateChanged();
}

void Settings::save() {
  persistToStore();
  captureSavedState();
  emit saved();
  emit stateChanged();
}

void Settings::resetToDefaults() {
  applyDefaults();
  emitAllValueChanged();
  emit reset();
  emitStateChanged();
}

bool Settings::hasChanges() const noexcept {
  return values_ != savedValues_;
}

} // namespace ui
