/**
 * @file ui/tests/unit/shell/TestNavigation.cpp
 * @brief Tests for the UI Navigation component.
 */

#include <gtest/gtest.h>

#include "ui/shell/Navigation.h"

namespace ui {

TEST(NavigationTest,
     SHELL_NAVIGATION_001_StoresAndExposesCurrentSectionAndSettingsCategory) {
  Navigation navigation;

  EXPECT_EQ(navigation.section(), Navigation::Section::Import);
  EXPECT_EQ(navigation.sectionValue(),
            static_cast<int>(Navigation::Section::Import));
  EXPECT_EQ(navigation.activeSection(), navigation.importSection());
  EXPECT_EQ(navigation.settingsCategory(),
            Navigation::SettingsCategory::General);
  EXPECT_EQ(navigation.settingsCategoryValue(),
            static_cast<int>(Navigation::SettingsCategory::General));

  navigation.setSection(Navigation::Section::Booking);
  navigation.setSectionValue(
      static_cast<int>(Navigation::Section::Analysis));
  navigation.setSettingsCategory(Navigation::SettingsCategory::Export);
  navigation.setSettingsCategoryValue(
      static_cast<int>(Navigation::SettingsCategory::Miscellaneous));

  EXPECT_EQ(navigation.section(), Navigation::Section::Analysis);
  EXPECT_EQ(navigation.sectionValue(),
            static_cast<int>(Navigation::Section::Analysis));
  EXPECT_EQ(navigation.activeSection(), navigation.analysisSection());
  EXPECT_EQ(navigation.settingsCategory(),
            Navigation::SettingsCategory::Miscellaneous);
  EXPECT_EQ(navigation.settingsCategoryValue(),
            static_cast<int>(Navigation::SettingsCategory::Miscellaneous));
}

TEST(NavigationTest,
     SHELL_NAVIGATION_002_ExposesQmlSectionConstantsAndNavigatesByValue) {
  Navigation navigation;

  EXPECT_EQ(navigation.actorSection(),
            static_cast<int>(Navigation::Section::Actors));
  EXPECT_EQ(navigation.propertySection(),
            static_cast<int>(Navigation::Section::Properties));
  EXPECT_EQ(navigation.contractSection(),
            static_cast<int>(Navigation::Section::Contracts));
  EXPECT_EQ(navigation.bookingSection(),
            static_cast<int>(Navigation::Section::Booking));
  EXPECT_EQ(navigation.importSection(),
            static_cast<int>(Navigation::Section::Import));
  EXPECT_EQ(navigation.exportSection(),
            static_cast<int>(Navigation::Section::Export));
  EXPECT_EQ(navigation.settingsSection(),
            static_cast<int>(Navigation::Section::Settings));
  EXPECT_EQ(navigation.analysisSection(),
            static_cast<int>(Navigation::Section::Analysis));
  EXPECT_EQ(navigation.annualSection(),
            static_cast<int>(Navigation::Section::Annual));

  navigation.navigateToSection(navigation.bookingSection());
  EXPECT_EQ(navigation.section(), Navigation::Section::Booking);
  EXPECT_EQ(navigation.activeSection(), navigation.bookingSection());
}

} // namespace ui
