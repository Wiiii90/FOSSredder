/**
 * @file ui/include/ui/shell/NavigationState.h
 * @brief Declarations for the UI NavigationState component.
 */

#pragma once

#include <QObject>

namespace ui {

class NavigationState : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      Section section READ section WRITE setSection NOTIFY sectionChanged)
  Q_PROPERTY(int sectionValue READ sectionValue NOTIFY sectionChanged)
  Q_PROPERTY(int activeSection READ activeSection NOTIFY sectionChanged)
  Q_PROPERTY(int actorSection READ actorSection CONSTANT)
  Q_PROPERTY(int propertySection READ propertySection CONSTANT)
  Q_PROPERTY(int contractSection READ contractSection CONSTANT)
  Q_PROPERTY(int bookingSection READ bookingSection CONSTANT)
  Q_PROPERTY(int importSection READ importSection CONSTANT)
  Q_PROPERTY(int exportSection READ exportSection CONSTANT)
  Q_PROPERTY(int settingsSection READ settingsSection CONSTANT)
  Q_PROPERTY(int analysisSection READ analysisSection CONSTANT)
  Q_PROPERTY(int annualSection READ annualSection CONSTANT)

public:
  enum class Section {
    Actors,
    Properties,
    Contracts,
    Booking,
    Import,
    Export,
    Settings,
    Analysis,
    Annual
  };
  Q_ENUM(Section)

  Q_PROPERTY(SettingsCategory settingsCategory READ settingsCategory WRITE
                 setSettingsCategory NOTIFY settingsCategoryChanged)
  Q_PROPERTY(int settingsCategoryValue READ settingsCategoryValue NOTIFY
                 settingsCategoryChanged)

  enum class SettingsCategory { General, Import, Export, Miscellaneous };

  explicit NavigationState(QObject *parent = nullptr);
  SettingsCategory settingsCategory() const noexcept {
    return settingsCategory_;
  }
  int settingsCategoryValue() const noexcept {
    return static_cast<int>(settingsCategory_);
  }
  void setSettingsCategory(SettingsCategory c);
  Q_INVOKABLE void setSettingsCategoryValue(int value);

  Section section() const noexcept { return section_; }
  int sectionValue() const noexcept { return static_cast<int>(section_); }
  int activeSection() const noexcept { return sectionValue(); }
  int actorSection() const noexcept { return static_cast<int>(Section::Actors); }
  int propertySection() const noexcept {
    return static_cast<int>(Section::Properties);
  }
  int contractSection() const noexcept {
    return static_cast<int>(Section::Contracts);
  }
  int bookingSection() const noexcept {
    return static_cast<int>(Section::Booking);
  }
  int importSection() const noexcept { return static_cast<int>(Section::Import); }
  int exportSection() const noexcept { return static_cast<int>(Section::Export); }
  int settingsSection() const noexcept {
    return static_cast<int>(Section::Settings);
  }
  int analysisSection() const noexcept {
    return static_cast<int>(Section::Analysis);
  }
  int annualSection() const noexcept { return static_cast<int>(Section::Annual); }
  void setSection(Section s);
  Q_INVOKABLE void setSectionValue(int value);
  Q_INVOKABLE void navigateToSection(int section);

signals:
  void sectionChanged();
  void settingsCategoryChanged();

private:
  Section section_ = Section::Import;
  SettingsCategory settingsCategory_ = SettingsCategory::General;
  bool sectionChangeQueued_ = false;
  bool settingsCategoryChangeQueued_ = false;
};

} // namespace ui
