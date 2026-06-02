/**
 * @file ui/include/ui/shell/window/DropHandler.h
 * @brief Declarations for the UI drop handling component.
 */

#pragma once

#include <QStringList>

class QEvent;

namespace ui::window {

struct DropEventOutcome {
  bool handled = false;
  bool accepted = false;
  QStringList files;
};

class DropHandler {
public:
  DropEventOutcome handle(QEvent *event) const;
};

} // namespace ui::window
