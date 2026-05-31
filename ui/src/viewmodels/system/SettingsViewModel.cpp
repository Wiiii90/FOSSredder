/**
 * @file ui/src/viewmodels/system/SettingsViewModel.cpp
 * @brief Implements the centralized UI settings view model exposed to QML.
 */

#include "ui/viewmodels/system/SettingsViewModel.h"

#include <QDate>
#include <QSettings>

#include "core/constants/import.h"
#include "core/constants/localization.h"
#include "core/constants/preferences.h"

namespace ui {

namespace {

QString fromCoreString(std::string_view value)
{
    return QString::fromLatin1(value.data());
}

QSettings openSettingsStore()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     fromCoreString(core::constants::preferences::kOrganizationName),
                     fromCoreString(core::constants::preferences::kApplicationName));
}

}

SettingsViewModel::SettingsViewModel(QObject* parent)
    : QObject(parent)
{
    load();
}

QString SettingsViewModel::normalizeText(const QString& value)
{
    return value.trimmed();
}

int SettingsViewModel::normalizeArchiveFormat(int value) noexcept
{
    return value == 1 ? 1 : 0;
}

QString SettingsViewModel::normalizeThemeMode(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("dark") ? QStringLiteral("dark") : QStringLiteral("light");
}

QString normalizeAnalysisDateMode(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("range") ? QStringLiteral("range") : QStringLiteral("year");
}

int normalizeAnalysisYear(int value) noexcept
{
    return value > 0 ? value : (QDate::currentDate().year() - 1);
}

void SettingsViewModel::emitStateChanged()
{
    emit stateChanged();
}

void SettingsViewModel::applyDefaults()
{
    language_ = fromCoreString(core::constants::localization::languages::kEnglish);
    themeMode_ = QStringLiteral("light");
    importDefaultPath_.clear();
    importPoppler_.clear();
    importOpenCv_.clear();
    importTesseract_.clear();
    importParser_.clear();
    importMatcher_.clear();
    exportDefaultDirectory_.clear();
    exportArchiveFormat_ = 0;
    exportIncludeFormulas_ = true;
    analysisDefaultDateMode_ = QStringLiteral("year");
    analysisDefaultYear_ = QDate::currentDate().year() - 1;
    toolbarShowBooking_ = true;
    toolbarShowActors_ = true;
    toolbarShowProperties_ = true;
    toolbarShowContracts_ = true;
    toolbarShowImport_ = true;
    toolbarShowExport_ = true;
    toolbarShowAnalysis_ = true;
    toolbarShowAnnual_ = true;
    toolbarShowSettings_ = true;
}

void SettingsViewModel::loadFromPersistentStore()
{
    auto settings = openSettingsStore();
    language_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kLanguage), fromCoreString(core::constants::localization::languages::kEnglish)).toString());
    if (language_.isEmpty()) language_ = fromCoreString(core::constants::localization::languages::kEnglish);
    themeMode_ = normalizeThemeMode(settings.value(fromCoreString(core::constants::preferences::keys::kThemeMode), QStringLiteral("light")).toString());
    importDefaultPath_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kImportDefaultPath)).toString());
    importPoppler_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportPoppler)).toString();
    importOpenCv_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportOpenCv)).toString();
    importTesseract_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportTesseract)).toString();
    importParser_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportParser)).toString();
    importMatcher_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportMatcher)).toString();
    exportDefaultDirectory_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kExportDefaultDirectory)).toString());
    exportArchiveFormat_ = normalizeArchiveFormat(settings.value(fromCoreString(core::constants::preferences::keys::kExportArchiveFormat), 0).toInt());
    exportIncludeFormulas_ = settings.value(fromCoreString(core::constants::preferences::keys::kExportIncludeFormulas), true).toBool();
    analysisDefaultDateMode_ = normalizeAnalysisDateMode(settings.value(fromCoreString(core::constants::preferences::keys::kAnalysisDefaultDateMode), QStringLiteral("year")).toString());
    analysisDefaultYear_ = normalizeAnalysisYear(settings.value(fromCoreString(core::constants::preferences::keys::kAnalysisDefaultYear), QDate::currentDate().year() - 1).toInt());
    toolbarShowBooking_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowBooking), true).toBool();
    toolbarShowActors_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowActors), true).toBool();
    toolbarShowProperties_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowProperties), true).toBool();
    toolbarShowContracts_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowContracts), true).toBool();
    toolbarShowImport_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowImport), true).toBool();
    toolbarShowExport_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowExport), true).toBool();
    toolbarShowAnalysis_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowAnalysis), true).toBool();
    toolbarShowAnnual_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowAnnual), true).toBool();
    toolbarShowSettings_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowSettings), true).toBool();
}

void SettingsViewModel::persistToStore() const
{
    auto settings = openSettingsStore();
    settings.setValue(fromCoreString(core::constants::preferences::keys::kLanguage), language_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kThemeMode), themeMode_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportDefaultPath), importDefaultPath_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportPoppler), importPoppler_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportOpenCv), importOpenCv_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportTesseract), importTesseract_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportParser), importParser_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportMatcher), importMatcher_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportDefaultDirectory), exportDefaultDirectory_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportArchiveFormat), exportArchiveFormat_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportIncludeFormulas), exportIncludeFormulas_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kAnalysisDefaultDateMode), analysisDefaultDateMode_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kAnalysisDefaultYear), analysisDefaultYear_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowBooking), toolbarShowBooking_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowActors), toolbarShowActors_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowProperties), toolbarShowProperties_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowContracts), toolbarShowContracts_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowImport), toolbarShowImport_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowExport), toolbarShowExport_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowAnalysis), toolbarShowAnalysis_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowAnnual), toolbarShowAnnual_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowSettings), toolbarShowSettings_);
    settings.sync();
}

void SettingsViewModel::setLanguage(const QString& value)
{
    const QString normalized = normalizeText(value).toLower();
    const QString nextValue = normalized.isEmpty() ? fromCoreString(core::constants::localization::languages::kEnglish) : normalized;
    if (language_ == nextValue) return;
    language_ = nextValue;
    emit languageChanged();
    emitStateChanged();
}

void SettingsViewModel::setThemeMode(const QString& value)
{
    const QString nextValue = normalizeThemeMode(value);
    if (themeMode_ == nextValue) return;
    themeMode_ = nextValue;
    emit themeModeChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportDefaultPath(const QString& value)
{
    const QString nextValue = normalizeText(value);
    if (importDefaultPath_ == nextValue) return;
    importDefaultPath_ = nextValue;
    emit importDefaultPathChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportPoppler(const QString& value)
{
    if (importPoppler_ == value) return;
    importPoppler_ = value;
    emit importPopplerChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportOpenCv(const QString& value)
{
    if (importOpenCv_ == value) return;
    importOpenCv_ = value;
    emit importOpenCvChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportTesseract(const QString& value)
{
    if (importTesseract_ == value) return;
    importTesseract_ = value;
    emit importTesseractChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportParser(const QString& value)
{
    if (importParser_ == value) return;
    importParser_ = value;
    emit importParserChanged();
    emitStateChanged();
}

void SettingsViewModel::setImportMatcher(const QString& value)
{
    if (importMatcher_ == value) return;
    importMatcher_ = value;
    emit importMatcherChanged();
    emitStateChanged();
}

void SettingsViewModel::setExportDefaultDirectory(const QString& value)
{
    const QString nextValue = normalizeText(value);
    if (exportDefaultDirectory_ == nextValue) return;
    exportDefaultDirectory_ = nextValue;
    emit exportDefaultDirectoryChanged();
    emitStateChanged();
}

void SettingsViewModel::setExportArchiveFormat(int value)
{
    const int nextValue = normalizeArchiveFormat(value);
    if (exportArchiveFormat_ == nextValue) return;
    exportArchiveFormat_ = nextValue;
    emit exportArchiveFormatChanged();
    emitStateChanged();
}

void SettingsViewModel::setExportIncludeFormulas(bool value)
{
    if (exportIncludeFormulas_ == value) return;
    exportIncludeFormulas_ = value;
    emit exportIncludeFormulasChanged();
    emitStateChanged();
}

void SettingsViewModel::setAnalysisDefaultDateMode(const QString& value)
{
    const QString nextValue = normalizeAnalysisDateMode(value);
    if (analysisDefaultDateMode_ == nextValue) return;
    analysisDefaultDateMode_ = nextValue;
    emit analysisDefaultDateModeChanged();
    emitStateChanged();
}

void SettingsViewModel::setAnalysisDefaultYear(int value)
{
    const int nextValue = normalizeAnalysisYear(value);
    if (analysisDefaultYear_ == nextValue) return;
    analysisDefaultYear_ = nextValue;
    emit analysisDefaultYearChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowBooking(bool value)
{
    if (toolbarShowBooking_ == value) return;
    toolbarShowBooking_ = value;
    emit toolbarShowBookingChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowActors(bool value)
{
    if (toolbarShowActors_ == value) return;
    toolbarShowActors_ = value;
    emit toolbarShowActorsChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowProperties(bool value)
{
    if (toolbarShowProperties_ == value) return;
    toolbarShowProperties_ = value;
    emit toolbarShowPropertiesChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowContracts(bool value)
{
    if (toolbarShowContracts_ == value) return;
    toolbarShowContracts_ = value;
    emit toolbarShowContractsChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowImport(bool value)
{
    if (toolbarShowImport_ == value) return;
    toolbarShowImport_ = value;
    emit toolbarShowImportChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowExport(bool value)
{
    if (toolbarShowExport_ == value) return;
    toolbarShowExport_ = value;
    emit toolbarShowExportChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowAnalysis(bool value)
{
    if (toolbarShowAnalysis_ == value) return;
    toolbarShowAnalysis_ = value;
    emit toolbarShowAnalysisChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowAnnual(bool value)
{
    if (toolbarShowAnnual_ == value) return;
    toolbarShowAnnual_ = value;
    emit toolbarShowAnnualChanged();
    emitStateChanged();
}

void SettingsViewModel::setToolbarShowSettings(bool value)
{
    if (toolbarShowSettings_ == value) return;
    toolbarShowSettings_ = value;
    emit toolbarShowSettingsChanged();
    emitStateChanged();
}

void SettingsViewModel::load()
{
    applyDefaults();
    loadFromPersistentStore();

    savedLanguage_ = language_;
    savedThemeMode_ = themeMode_;
    savedImportDefaultPath_ = importDefaultPath_;
    savedImportPoppler_ = importPoppler_;
    savedImportOpenCv_ = importOpenCv_;
    savedImportTesseract_ = importTesseract_;
    savedImportParser_ = importParser_;
    savedImportMatcher_ = importMatcher_;
    savedExportDefaultDirectory_ = exportDefaultDirectory_;
    savedExportArchiveFormat_ = exportArchiveFormat_;
    savedExportIncludeFormulas_ = exportIncludeFormulas_;
    savedAnalysisDefaultDateMode_ = analysisDefaultDateMode_;
    savedAnalysisDefaultYear_ = analysisDefaultYear_;
    savedToolbarShowBooking_ = toolbarShowBooking_;
    savedToolbarShowActors_ = toolbarShowActors_;
    savedToolbarShowProperties_ = toolbarShowProperties_;
    savedToolbarShowContracts_ = toolbarShowContracts_;
    savedToolbarShowImport_ = toolbarShowImport_;
    savedToolbarShowExport_ = toolbarShowExport_;
    savedToolbarShowAnalysis_ = toolbarShowAnalysis_;
    savedToolbarShowAnnual_ = toolbarShowAnnual_;
    savedToolbarShowSettings_ = toolbarShowSettings_;

    emit stateChanged();
}

void SettingsViewModel::save()
{
    persistToStore();
    savedLanguage_ = language_;
    savedThemeMode_ = themeMode_;
    savedImportDefaultPath_ = importDefaultPath_;
    savedImportPoppler_ = importPoppler_;
    savedImportOpenCv_ = importOpenCv_;
    savedImportTesseract_ = importTesseract_;
    savedImportParser_ = importParser_;
    savedImportMatcher_ = importMatcher_;
    savedExportDefaultDirectory_ = exportDefaultDirectory_;
    savedExportArchiveFormat_ = exportArchiveFormat_;
    savedExportIncludeFormulas_ = exportIncludeFormulas_;
    savedAnalysisDefaultDateMode_ = analysisDefaultDateMode_;
    savedAnalysisDefaultYear_ = analysisDefaultYear_;
    savedToolbarShowBooking_ = toolbarShowBooking_;
    savedToolbarShowActors_ = toolbarShowActors_;
    savedToolbarShowProperties_ = toolbarShowProperties_;
    savedToolbarShowContracts_ = toolbarShowContracts_;
    savedToolbarShowImport_ = toolbarShowImport_;
    savedToolbarShowExport_ = toolbarShowExport_;
    savedToolbarShowAnalysis_ = toolbarShowAnalysis_;
    savedToolbarShowAnnual_ = toolbarShowAnnual_;
    savedToolbarShowSettings_ = toolbarShowSettings_;
    emit saved();
    emit stateChanged();
}

void SettingsViewModel::resetToDefaults()
{
    applyDefaults();
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
    emit reset();
    emitStateChanged();
}

bool SettingsViewModel::hasChanges() const noexcept
{
    return language_ != savedLanguage_
        || themeMode_ != savedThemeMode_
        || importDefaultPath_ != savedImportDefaultPath_
        || importPoppler_ != savedImportPoppler_
        || importOpenCv_ != savedImportOpenCv_
        || importTesseract_ != savedImportTesseract_
        || importParser_ != savedImportParser_
        || importMatcher_ != savedImportMatcher_
        || exportDefaultDirectory_ != savedExportDefaultDirectory_
        || exportArchiveFormat_ != savedExportArchiveFormat_
        || exportIncludeFormulas_ != savedExportIncludeFormulas_
        || analysisDefaultDateMode_ != savedAnalysisDefaultDateMode_
        || analysisDefaultYear_ != savedAnalysisDefaultYear_
        || toolbarShowBooking_ != savedToolbarShowBooking_
        || toolbarShowActors_ != savedToolbarShowActors_
        || toolbarShowProperties_ != savedToolbarShowProperties_
        || toolbarShowContracts_ != savedToolbarShowContracts_
        || toolbarShowImport_ != savedToolbarShowImport_
        || toolbarShowExport_ != savedToolbarShowExport_
        || toolbarShowAnalysis_ != savedToolbarShowAnalysis_
        || toolbarShowAnnual_ != savedToolbarShowAnnual_
        || toolbarShowSettings_ != savedToolbarShowSettings_;
}

} // namespace ui
