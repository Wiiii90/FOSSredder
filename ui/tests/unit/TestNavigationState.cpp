/**
 * @file ui/tests/unit/TestNavigationState.cpp
 * @brief Tests for the UI NavigationState component.
 */

#include <gtest/gtest.h>

#include "ui/state/navigation/NavigationState.h"

namespace ui {

TEST(NavigationStateTest,
     StoresAndExposesTheCurrentSectionAndSettingsCategory) {
  NavigationState navigation;

  EXPECT_EQ(navigation.section(), NavigationState::Section::Import);
  EXPECT_EQ(navigation.sectionValue(),
            static_cast<int>(NavigationState::Section::Import));
  EXPECT_EQ(navigation.activeSection(), navigation.importSection());
  EXPECT_EQ(navigation.settingsCategory(),
            NavigationState::SettingsCategory::General);
  EXPECT_EQ(navigation.settingsCategoryValue(),
            static_cast<int>(NavigationState::SettingsCategory::General));

  navigation.setSection(NavigationState::Section::Booking);
  navigation.setSectionValue(
      static_cast<int>(NavigationState::Section::Analysis));
  navigation.setSettingsCategory(NavigationState::SettingsCategory::Export);
  navigation.setSettingsCategoryValue(
      static_cast<int>(NavigationState::SettingsCategory::Miscellaneous));

  EXPECT_EQ(navigation.section(), NavigationState::Section::Analysis);
  EXPECT_EQ(navigation.sectionValue(),
            static_cast<int>(NavigationState::Section::Analysis));
  EXPECT_EQ(navigation.activeSection(), navigation.analysisSection());
  EXPECT_EQ(navigation.settingsCategory(),
            NavigationState::SettingsCategory::Miscellaneous);
  EXPECT_EQ(navigation.settingsCategoryValue(),
            static_cast<int>(NavigationState::SettingsCategory::Miscellaneous));
}

TEST(NavigationStateTest, ExposesQmlSectionConstantsAndNavigatesByValue) {
  NavigationState navigation;

  EXPECT_EQ(navigation.actorSection(),
            static_cast<int>(NavigationState::Section::Actors));
  EXPECT_EQ(navigation.propertySection(),
            static_cast<int>(NavigationState::Section::Properties));
  EXPECT_EQ(navigation.contractSection(),
            static_cast<int>(NavigationState::Section::Contracts));
  EXPECT_EQ(navigation.bookingSection(),
            static_cast<int>(NavigationState::Section::Booking));
  EXPECT_EQ(navigation.importSection(),
            static_cast<int>(NavigationState::Section::Import));
  EXPECT_EQ(navigation.exportSection(),
            static_cast<int>(NavigationState::Section::Export));
  EXPECT_EQ(navigation.settingsSection(),
            static_cast<int>(NavigationState::Section::Settings));
  EXPECT_EQ(navigation.analysisSection(),
            static_cast<int>(NavigationState::Section::Analysis));
  EXPECT_EQ(navigation.annualSection(),
            static_cast<int>(NavigationState::Section::Annual));

  navigation.navigateToSection(navigation.bookingSection());
  EXPECT_EQ(navigation.section(), NavigationState::Section::Booking);
  EXPECT_EQ(navigation.activeSection(), navigation.bookingSection());
}

} // namespace ui
