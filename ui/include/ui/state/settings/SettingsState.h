/**
 * @file ui/include/ui/state/settings/SettingsState.h
 * @brief Declares the UI state adapter for Settings views.
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
class SettingsViewModel;

/**
 * @brief Owns Settings view navigation and delegates persisted values.
 */
class SettingsState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(SettingsState)

  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY changed)
  Q_PROPERTY(ui::SettingsViewModel *settingsViewModel READ settingsViewModel WRITE
                 setSettingsViewModel NOTIFY changed)
  Q_PROPERTY(ui::Actions *actions READ actions WRITE setActions NOTIFY changed)
  Q_PROPERTY(ui::LanguageService *languageService READ languageService WRITE
                 setLanguageService NOTIFY changed)

  Q_PROPERTY(int currentCategory READ currentCategory NOTIFY changed)
  Q_PROPERTY(QVariantList categoryRows READ categoryRows NOTIFY changed)
  Q_PROPERTY(bool canNavigateCategories READ canNavigateCategories NOTIFY changed)
  Q_PROPERTY(QVariantList languageOptions READ languageOptions NOTIFY changed)
  Q_PROPERTY(int languageIndex READ languageIndex NOTIFY changed)

  Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY changed)
  Q_PROPERTY(QString importDefaultPath READ importDefaultPath WRITE
                 setImportDefaultPath NOTIFY changed)
  Q_PROPERTY(QString importPoppler READ importPoppler WRITE setImportPoppler
                 NOTIFY changed)
  Q_PROPERTY(QString importOpenCv READ importOpenCv WRITE setImportOpenCv
                 NOTIFY changed)
  Q_PROPERTY(QString importTesseract READ importTesseract WRITE
                 setImportTesseract NOTIFY changed)
  Q_PROPERTY(QString importParser READ importParser WRITE setImportParser
                 NOTIFY changed)
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
  explicit SettingsState(QObject *parent = nullptr);

  NavigationState *navigation() const noexcept { return navigation_; }
  SettingsViewModel *settingsViewModel() const noexcept {
    return settingsViewModel_;
  }
  Actions *actions() const noexcept { return actions_; }
  LanguageService *languageService() const noexcept { return languageService_; }

  void setNavigation(NavigationState *value);
  void setSettingsViewModel(SettingsViewModel *value);
  void setActions(Actions *value);
  void setLanguageService(LanguageService *value);

  int currentCategory() const;
  QVariantList categoryRows() const;
  bool canNavigateCategories() const noexcept;
  QVariantList languageOptions() const;
  int languageIndex() const;

  QString language() const;
  void setLanguage(const QString &value);
  QString importDefaultPath() const;
  void setImportDefaultPath(const QString &value);
  QString importPoppler() const;
  void setImportPoppler(const QString &value);
  QString importOpenCv() const;
  void setImportOpenCv(const QString &value);
  QString importTesseract() const;
  void setImportTesseract(const QString &value);
  QString importParser() const;
  void setImportParser(const QString &value);
  QString importMatcher() const;
  void setImportMatcher(const QString &value);
  QString exportDefaultDirectory() const;
  void setExportDefaultDirectory(const QString &value);
  int exportArchiveFormat() const;
  void setExportArchiveFormat(int value);
  bool exportIncludeFormulas() const;
  void setExportIncludeFormulas(bool value);
  bool toolbarShowBooking() const;
  void setToolbarShowBooking(bool value);
  bool toolbarShowImport() const;
  void setToolbarShowImport(bool value);
  bool toolbarShowActors() const;
  void setToolbarShowActors(bool value);
  bool toolbarShowExport() const;
  void setToolbarShowExport(bool value);
  bool toolbarShowProperties() const;
  void setToolbarShowProperties(bool value);
  bool toolbarShowAnalysis() const;
  void setToolbarShowAnalysis(bool value);
  bool toolbarShowContracts() const;
  void setToolbarShowContracts(bool value);
  bool toolbarShowAnnual() const;
  void setToolbarShowAnnual(bool value);
  bool toolbarShowSettings() const;
  void setToolbarShowSettings(bool value);

  Q_INVOKABLE void activate();
  Q_INVOKABLE void navigateCategory(int delta);
  Q_INVOKABLE void selectCategory(int category);
  Q_INVOKABLE void saveSettings();
  Q_INVOKABLE void resetSettings();
  Q_INVOKABLE void selectLanguageAt(int index);
  Q_INVOKABLE void browseImportPath();
  Q_INVOKABLE void browseExportDirectory();

signals:
  void changed();

private:
  int firstCategory() const noexcept;
  int lastCategory() const noexcept;
  void bindNavigation(NavigationState *value);
  void bindSettings(SettingsViewModel *value);
  void bindActions(Actions *value);
  void emitChanged();

  NavigationState *navigation_ = nullptr;
  SettingsViewModel *settingsViewModel_ = nullptr;
  Actions *actions_ = nullptr;
  LanguageService *languageService_ = nullptr;
};

} // namespace ui
