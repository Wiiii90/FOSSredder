/**
 * @file ui/include/ui/shell/Navigation.h
 * @brief Declarations for the UI Navigation component.
 */

#pragma once

#include <QObject>

namespace ui {

/**
 * @brief Stores the active top-level shell section and settings sub-section.
 */
class Navigation : public QObject {
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
  /**
   * @brief Top-level shell sections shown by the desktop UI.
   */
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

  /**
   * @brief Settings page categories shown inside the settings section.
   */
  enum class SettingsCategory {
    General,
    Import,
    Export,
    Miscellaneous
  };

  /**
   * @brief Creates the navigation state.
   * @param parent Optional QObject parent.
   */
  explicit Navigation(QObject* parent = nullptr);

  /**
   * @brief Returns the active settings category.
   * @return Active settings category.
   */
  SettingsCategory settingsCategory() const noexcept {
    return settingsCategory_;
  }

  /**
   * @brief Returns the active settings category as QML-friendly integer.
   * @return Integer value of the active settings category.
   */
  int settingsCategoryValue() const noexcept {
    return static_cast<int>(settingsCategory_);
  }

  /**
   * @brief Sets the active settings category.
   * @param c Settings category to activate.
   */
  void setSettingsCategory(SettingsCategory c);

  /**
   * @brief Sets the active settings category from QML.
   * @param value Integer settings category value.
   */
  Q_INVOKABLE void setSettingsCategoryValue(int value);

  /**
   * @brief Returns the active shell section.
   * @return Active shell section.
   */
  Section section() const noexcept {
    return section_;
  }

  /**
   * @brief Returns the active shell section as QML-friendly integer.
   * @return Integer value of the active shell section.
   */
  int sectionValue() const noexcept {
    return static_cast<int>(section_);
  }

  /**
   * @brief Returns the active shell section value used by QML.
   * @return Integer value of the active shell section.
   */
  int activeSection() const noexcept {
    return sectionValue();
  }

  /** @brief Returns the actor section enum value for QML. */
  int actorSection() const noexcept {
    return static_cast<int>(Section::Actors);
  }

  /** @brief Returns the property section enum value for QML. */
  int propertySection() const noexcept {
    return static_cast<int>(Section::Properties);
  }

  /** @brief Returns the contract section enum value for QML. */
  int contractSection() const noexcept {
    return static_cast<int>(Section::Contracts);
  }

  /** @brief Returns the booking section enum value for QML. */
  int bookingSection() const noexcept {
    return static_cast<int>(Section::Booking);
  }

  /** @brief Returns the import section enum value for QML. */
  int importSection() const noexcept {
    return static_cast<int>(Section::Import);
  }

  /** @brief Returns the export section enum value for QML. */
  int exportSection() const noexcept {
    return static_cast<int>(Section::Export);
  }

  /** @brief Returns the settings section enum value for QML. */
  int settingsSection() const noexcept {
    return static_cast<int>(Section::Settings);
  }

  /** @brief Returns the analysis section enum value for QML. */
  int analysisSection() const noexcept {
    return static_cast<int>(Section::Analysis);
  }

  /** @brief Returns the annual section enum value for QML. */
  int annualSection() const noexcept {
    return static_cast<int>(Section::Annual);
  }

  /**
   * @brief Sets the active shell section.
   * @param s Section to activate.
   */
  void setSection(Section s);

  /**
   * @brief Sets the active shell section from QML.
   * @param value Integer section value.
   */
  Q_INVOKABLE void setSectionValue(int value);

  /**
   * @brief Navigates to a top-level shell section from QML.
   * @param section Integer section value.
   */
  Q_INVOKABLE void navigateToSection(int section);

signals:
  /**
   * @brief Emitted when the active shell section changes.
   */
  void sectionChanged();

  /**
   * @brief Emitted when the active settings category changes.
   */
  void settingsCategoryChanged();

private:
  Section section_ = Section::Import;
  SettingsCategory settingsCategory_ = SettingsCategory::General;
  bool sectionChangeQueued_ = false;
  bool settingsCategoryChangeQueued_ = false;
};

} // namespace ui
