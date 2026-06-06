/**
 * @file ui/src/shell/Navigation.cpp
 * @brief Implementation of the UI Navigation component.
 */

#include "ui/shell/Navigation.h"

#include <QMetaObject>

namespace ui {

Navigation::Navigation(QObject* parent) : QObject(parent) {}

void Navigation::setSection(Section s) {
  if (section_ == s)
    return;
  section_ = s;
  if (sectionChangeQueued_)
    return;
  sectionChangeQueued_ = true;
  QMetaObject::invokeMethod(
      this,
      [this]() {
        sectionChangeQueued_ = false;
        emit sectionChanged();
      },
      Qt::QueuedConnection);
}

void Navigation::setSectionValue(int value) {
  setSection(static_cast<Section>(value));
}

void Navigation::navigateToSection(int section) {
  setSectionValue(section);
}

void Navigation::setSettingsCategory(SettingsCategory c) {
  if (settingsCategory_ == c)
    return;
  settingsCategory_ = c;
  if (settingsCategoryChangeQueued_)
    return;
  settingsCategoryChangeQueued_ = true;
  QMetaObject::invokeMethod(
      this,
      [this]() {
        settingsCategoryChangeQueued_ = false;
        emit settingsCategoryChanged();
      },
      Qt::QueuedConnection);
}

void Navigation::setSettingsCategoryValue(int value) {
  setSettingsCategory(static_cast<SettingsCategory>(value));
}

} // namespace ui
