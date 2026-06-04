/**
 * @file ui/include/ui/shell/Settings.h
 * @brief Declares the centralized UI settings view model exposed to QML.
 */

#pragma once

#include <utility>

#include <QObject>
#include <QString>

namespace ui {

/**
 * @brief Stores and exposes globally persisted UI settings for settings-related
 * views.
 */
class Settings : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      QString language READ language WRITE setLanguage NOTIFY languageChanged)
  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY
                 themeModeChanged)
  Q_PROPERTY(bool autosaveOnClose READ autosaveOnClose WRITE setAutosaveOnClose
                 NOTIFY autosaveOnCloseChanged)
  Q_PROPERTY(int autosaveIntervalMinutes READ autosaveIntervalMinutes WRITE
                 setAutosaveIntervalMinutes NOTIFY autosaveIntervalMinutesChanged)
  Q_PROPERTY(QString importDefaultPath READ importDefaultPath WRITE
                 setImportDefaultPath NOTIFY importDefaultPathChanged)
  Q_PROPERTY(QString importPoppler READ importPoppler WRITE setImportPoppler
                 NOTIFY importPopplerChanged)
  Q_PROPERTY(QString importOpenCv READ importOpenCv WRITE setImportOpenCv NOTIFY
                 importOpenCvChanged)
  Q_PROPERTY(QString importTesseract READ importTesseract WRITE
                 setImportTesseract NOTIFY importTesseractChanged)
  Q_PROPERTY(QString importParser READ importParser WRITE setImportParser NOTIFY
                 importParserChanged)
  Q_PROPERTY(QString importMatcher READ importMatcher WRITE setImportMatcher
                 NOTIFY importMatcherChanged)
  Q_PROPERTY(QString exportDefaultDirectory READ exportDefaultDirectory WRITE
                 setExportDefaultDirectory NOTIFY exportDefaultDirectoryChanged)
  Q_PROPERTY(int exportArchiveFormat READ exportArchiveFormat WRITE
                 setExportArchiveFormat NOTIFY exportArchiveFormatChanged)
  Q_PROPERTY(bool exportIncludeFormulas READ exportIncludeFormulas WRITE
                 setExportIncludeFormulas NOTIFY exportIncludeFormulasChanged)
  Q_PROPERTY(
      QString analysisDefaultDateMode READ analysisDefaultDateMode WRITE
          setAnalysisDefaultDateMode NOTIFY analysisDefaultDateModeChanged)
  Q_PROPERTY(int analysisDefaultYear READ analysisDefaultYear WRITE
                 setAnalysisDefaultYear NOTIFY analysisDefaultYearChanged)
  Q_PROPERTY(bool toolbarShowBooking READ toolbarShowBooking WRITE
                 setToolbarShowBooking NOTIFY toolbarShowBookingChanged)
  Q_PROPERTY(bool toolbarShowActors READ toolbarShowActors WRITE
                 setToolbarShowActors NOTIFY toolbarShowActorsChanged)
  Q_PROPERTY(bool toolbarShowProperties READ toolbarShowProperties WRITE
                 setToolbarShowProperties NOTIFY toolbarShowPropertiesChanged)
  Q_PROPERTY(bool toolbarShowContracts READ toolbarShowContracts WRITE
                 setToolbarShowContracts NOTIFY toolbarShowContractsChanged)
  Q_PROPERTY(bool toolbarShowImport READ toolbarShowImport WRITE
                 setToolbarShowImport NOTIFY toolbarShowImportChanged)
  Q_PROPERTY(bool toolbarShowExport READ toolbarShowExport WRITE
                 setToolbarShowExport NOTIFY toolbarShowExportChanged)
  Q_PROPERTY(bool toolbarShowAnalysis READ toolbarShowAnalysis WRITE
                 setToolbarShowAnalysis NOTIFY toolbarShowAnalysisChanged)
  Q_PROPERTY(bool toolbarShowAnnual READ toolbarShowAnnual WRITE
                 setToolbarShowAnnual NOTIFY toolbarShowAnnualChanged)
  Q_PROPERTY(bool toolbarShowSettings READ toolbarShowSettings WRITE
                 setToolbarShowSettings NOTIFY toolbarShowSettingsChanged)

public:
  explicit Settings(QObject *parent = nullptr);

  QString language() const { return values_.language; }
  QString themeMode() const { return values_.themeMode; }
  bool autosaveOnClose() const noexcept { return values_.autosaveOnClose; }
  int autosaveIntervalMinutes() const noexcept {
    return values_.autosaveIntervalMinutes;
  }
  QString importDefaultPath() const { return values_.importDefaultPath; }
  QString importPoppler() const { return values_.importPoppler; }
  QString importOpenCv() const { return values_.importOpenCv; }
  QString importTesseract() const { return values_.importTesseract; }
  QString importParser() const { return values_.importParser; }
  QString importMatcher() const { return values_.importMatcher; }
  QString exportDefaultDirectory() const {
    return values_.exportDefaultDirectory;
  }
  int exportArchiveFormat() const noexcept {
    return values_.exportArchiveFormat;
  }
  bool exportIncludeFormulas() const noexcept {
    return values_.exportIncludeFormulas;
  }
  QString analysisDefaultDateMode() const {
    return values_.analysisDefaultDateMode;
  }
  int analysisDefaultYear() const noexcept {
    return values_.analysisDefaultYear;
  }
  bool toolbarShowBooking() const noexcept {
    return values_.toolbarShowBooking;
  }
  bool toolbarShowActors() const noexcept {
    return values_.toolbarShowActors;
  }
  bool toolbarShowProperties() const noexcept {
    return values_.toolbarShowProperties;
  }
  bool toolbarShowContracts() const noexcept {
    return values_.toolbarShowContracts;
  }
  bool toolbarShowImport() const noexcept { return values_.toolbarShowImport; }
  bool toolbarShowExport() const noexcept { return values_.toolbarShowExport; }
  bool toolbarShowAnalysis() const noexcept {
    return values_.toolbarShowAnalysis;
  }
  bool toolbarShowAnnual() const noexcept { return values_.toolbarShowAnnual; }
  bool toolbarShowSettings() const noexcept {
    return values_.toolbarShowSettings;
  }

  void setLanguage(const QString &value);
  void setThemeMode(const QString &value);
  void setAutosaveOnClose(bool value);
  void setAutosaveIntervalMinutes(int value);
  void setImportDefaultPath(const QString &value);
  void setImportPoppler(const QString &value);
  void setImportOpenCv(const QString &value);
  void setImportTesseract(const QString &value);
  void setImportParser(const QString &value);
  void setImportMatcher(const QString &value);
  void setExportDefaultDirectory(const QString &value);
  void setExportArchiveFormat(int value);
  void setExportIncludeFormulas(bool value);
  void setAnalysisDefaultDateMode(const QString &value);
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

  void load();
  void save();
  void resetToDefaults();
  bool hasChanges() const noexcept;

signals:
  void languageChanged();
  void themeModeChanged();
  void autosaveOnCloseChanged();
  void autosaveIntervalMinutesChanged();
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
  struct Values {
    QString language;
    QString themeMode;
    bool autosaveOnClose = true;
    int autosaveIntervalMinutes = 0;
    QString importDefaultPath;
    QString importPoppler;
    QString importOpenCv;
    QString importTesseract;
    QString importParser;
    QString importMatcher;
    QString exportDefaultDirectory;
    int exportArchiveFormat = 0;
    bool exportIncludeFormulas = true;
    QString analysisDefaultDateMode;
    int analysisDefaultYear = 0;
    bool toolbarShowBooking = true;
    bool toolbarShowActors = true;
    bool toolbarShowProperties = true;
    bool toolbarShowContracts = true;
    bool toolbarShowImport = true;
    bool toolbarShowExport = true;
    bool toolbarShowAnalysis = true;
    bool toolbarShowAnnual = true;
    bool toolbarShowSettings = true;

    bool operator==(const Values &) const = default;
  };

  static QString normalizeText(const QString &value);
  static QString normalizeThemeMode(const QString &value);
  static int normalizeArchiveFormat(int value) noexcept;
  static int normalizeAutosaveIntervalMinutes(int value) noexcept;
  static Values defaultValues();

  template <typename TValue, typename TSignal>
  void updateSetting(TValue &target, TValue value, TSignal signal) {
    if (target == value) {
      return;
    }
    target = std::move(value);
    (this->*signal)();
    emitStateChanged();
  }

  void emitStateChanged();
  void applyDefaults();
  void loadFromPersistentStore();
  void persistToStore() const;
  void captureSavedState();
  void emitAllValueChanged();

  Values values_;
  Values savedValues_;
};

} // namespace ui
