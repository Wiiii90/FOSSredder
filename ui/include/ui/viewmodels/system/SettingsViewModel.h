/**
 * @file ui/include/ui/viewmodels/system/SettingsViewModel.h
 * @brief Declares the centralized UI settings view model exposed to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace ui {

/**
 * @brief Stores and exposes globally persisted UI settings for settings-related views.
 */
class SettingsViewModel : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SettingsViewModel)
    QML_UNCREATABLE("SettingsViewModel is provided by the application context")

    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QString importDefaultPath READ importDefaultPath WRITE setImportDefaultPath NOTIFY importDefaultPathChanged)
    Q_PROPERTY(QString importPoppler READ importPoppler WRITE setImportPoppler NOTIFY importPopplerChanged)
    Q_PROPERTY(QString importOpenCv READ importOpenCv WRITE setImportOpenCv NOTIFY importOpenCvChanged)
    Q_PROPERTY(QString importTesseract READ importTesseract WRITE setImportTesseract NOTIFY importTesseractChanged)
    Q_PROPERTY(QString importParser READ importParser WRITE setImportParser NOTIFY importParserChanged)
    Q_PROPERTY(QString importMatcher READ importMatcher WRITE setImportMatcher NOTIFY importMatcherChanged)
    Q_PROPERTY(QString exportDefaultDirectory READ exportDefaultDirectory WRITE setExportDefaultDirectory NOTIFY exportDefaultDirectoryChanged)
    Q_PROPERTY(int exportArchiveFormat READ exportArchiveFormat WRITE setExportArchiveFormat NOTIFY exportArchiveFormatChanged)
    Q_PROPERTY(bool exportIncludeFormulas READ exportIncludeFormulas WRITE setExportIncludeFormulas NOTIFY exportIncludeFormulasChanged)
    Q_PROPERTY(QString analysisDefaultDateMode READ analysisDefaultDateMode WRITE setAnalysisDefaultDateMode NOTIFY analysisDefaultDateModeChanged)
    Q_PROPERTY(int analysisDefaultYear READ analysisDefaultYear WRITE setAnalysisDefaultYear NOTIFY analysisDefaultYearChanged)
    Q_PROPERTY(bool toolbarShowBooking READ toolbarShowBooking WRITE setToolbarShowBooking NOTIFY toolbarShowBookingChanged)
    Q_PROPERTY(bool toolbarShowActors READ toolbarShowActors WRITE setToolbarShowActors NOTIFY toolbarShowActorsChanged)
    Q_PROPERTY(bool toolbarShowProperties READ toolbarShowProperties WRITE setToolbarShowProperties NOTIFY toolbarShowPropertiesChanged)
    Q_PROPERTY(bool toolbarShowContracts READ toolbarShowContracts WRITE setToolbarShowContracts NOTIFY toolbarShowContractsChanged)
    Q_PROPERTY(bool toolbarShowImport READ toolbarShowImport WRITE setToolbarShowImport NOTIFY toolbarShowImportChanged)
    Q_PROPERTY(bool toolbarShowExport READ toolbarShowExport WRITE setToolbarShowExport NOTIFY toolbarShowExportChanged)
    Q_PROPERTY(bool toolbarShowAnalysis READ toolbarShowAnalysis WRITE setToolbarShowAnalysis NOTIFY toolbarShowAnalysisChanged)
    Q_PROPERTY(bool toolbarShowAnnual READ toolbarShowAnnual WRITE setToolbarShowAnnual NOTIFY toolbarShowAnnualChanged)
    Q_PROPERTY(bool toolbarShowSettings READ toolbarShowSettings WRITE setToolbarShowSettings NOTIFY toolbarShowSettingsChanged)

public:
    explicit SettingsViewModel(QObject* parent = nullptr);

    QString language() const { return language_; }
    QString themeMode() const { return themeMode_; }
    QString importDefaultPath() const { return importDefaultPath_; }
    QString importPoppler() const { return importPoppler_; }
    QString importOpenCv() const { return importOpenCv_; }
    QString importTesseract() const { return importTesseract_; }
    QString importParser() const { return importParser_; }
    QString importMatcher() const { return importMatcher_; }
    QString exportDefaultDirectory() const { return exportDefaultDirectory_; }
    int exportArchiveFormat() const noexcept { return exportArchiveFormat_; }
    bool exportIncludeFormulas() const noexcept { return exportIncludeFormulas_; }
    QString analysisDefaultDateMode() const { return analysisDefaultDateMode_; }
    int analysisDefaultYear() const noexcept { return analysisDefaultYear_; }
    bool toolbarShowBooking() const noexcept { return toolbarShowBooking_; }
    bool toolbarShowActors() const noexcept { return toolbarShowActors_; }
    bool toolbarShowProperties() const noexcept { return toolbarShowProperties_; }
    bool toolbarShowContracts() const noexcept { return toolbarShowContracts_; }
    bool toolbarShowImport() const noexcept { return toolbarShowImport_; }
    bool toolbarShowExport() const noexcept { return toolbarShowExport_; }
    bool toolbarShowAnalysis() const noexcept { return toolbarShowAnalysis_; }
    bool toolbarShowAnnual() const noexcept { return toolbarShowAnnual_; }
    bool toolbarShowSettings() const noexcept { return toolbarShowSettings_; }

    void setLanguage(const QString& value);
    void setThemeMode(const QString& value);
    void setImportDefaultPath(const QString& value);
    void setImportPoppler(const QString& value);
    void setImportOpenCv(const QString& value);
    void setImportTesseract(const QString& value);
    void setImportParser(const QString& value);
    void setImportMatcher(const QString& value);
    void setExportDefaultDirectory(const QString& value);
    void setExportArchiveFormat(int value);
    void setExportIncludeFormulas(bool value);
    void setAnalysisDefaultDateMode(const QString& value);
    void setAnalysisDefaultYear(int value);
    void setToolbarShowBooking(bool value);
    void setToolbarShowActors(bool value);
    void setToolbarShowProperties(bool value);
    void setToolbarShowContracts(bool value);
    void setToolbarShowImport(bool value);
    void setToolbarShowExport(bool value);
    void setToolbarShowAnalysis(bool value);
    void setToolbarShowAnnual(bool value);
    void setToolbarShowSettings(bool value);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();
    Q_INVOKABLE void resetToDefaults();
    Q_INVOKABLE bool hasChanges() const noexcept;

signals:
    void languageChanged();
    void themeModeChanged();
    void importDefaultPathChanged();
    void importPopplerChanged();
    void importOpenCvChanged();
    void importTesseractChanged();
    void importParserChanged();
    void importMatcherChanged();
    void exportDefaultDirectoryChanged();
    void exportArchiveFormatChanged();
    void exportIncludeFormulasChanged();
    void analysisDefaultDateModeChanged();
    void analysisDefaultYearChanged();
    void toolbarShowBookingChanged();
    void toolbarShowActorsChanged();
    void toolbarShowPropertiesChanged();
    void toolbarShowContractsChanged();
    void toolbarShowImportChanged();
    void toolbarShowExportChanged();
    void toolbarShowAnalysisChanged();
    void toolbarShowAnnualChanged();
    void toolbarShowSettingsChanged();
    void stateChanged();
    void saved();
    void reset();

private:
    static QString normalizeText(const QString& value);
    static QString normalizeThemeMode(const QString& value);
    static int normalizeArchiveFormat(int value) noexcept;

    void emitStateChanged();
    void applyDefaults();
    void loadFromPersistentStore();
    void persistToStore() const;

    QString language_;
    QString themeMode_;
    QString importDefaultPath_;
    QString importPoppler_;
    QString importOpenCv_;
    QString importTesseract_;
    QString importParser_;
    QString importMatcher_;
    QString exportDefaultDirectory_;
    int exportArchiveFormat_ = 0;
    bool exportIncludeFormulas_ = true;
    QString analysisDefaultDateMode_;
    int analysisDefaultYear_ = 0;
    bool toolbarShowBooking_ = true;
    bool toolbarShowActors_ = true;
    bool toolbarShowProperties_ = true;
    bool toolbarShowContracts_ = true;
    bool toolbarShowImport_ = true;
    bool toolbarShowExport_ = true;
    bool toolbarShowAnalysis_ = true;
    bool toolbarShowAnnual_ = true;
    bool toolbarShowSettings_ = true;

    QString savedLanguage_;
    QString savedThemeMode_;
    QString savedImportDefaultPath_;
    QString savedImportPoppler_;
    QString savedImportOpenCv_;
    QString savedImportTesseract_;
    QString savedImportParser_;
    QString savedImportMatcher_;
    QString savedExportDefaultDirectory_;
    int savedExportArchiveFormat_ = 0;
    bool savedExportIncludeFormulas_ = true;
    QString savedAnalysisDefaultDateMode_;
    int savedAnalysisDefaultYear_ = 0;
    bool savedToolbarShowBooking_ = true;
    bool savedToolbarShowActors_ = true;
    bool savedToolbarShowProperties_ = true;
    bool savedToolbarShowContracts_ = true;
    bool savedToolbarShowImport_ = true;
    bool savedToolbarShowExport_ = true;
    bool savedToolbarShowAnalysis_ = true;
    bool savedToolbarShowAnnual_ = true;
    bool savedToolbarShowSettings_ = true;
};

} // namespace ui
