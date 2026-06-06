/**
 * @file ui/include/ui/shell/window/DropHandler.h
 * @brief Declarations for the UI drop handling component.
 */

#pragma once

#include <QStringList>

class QEvent;

namespace ui::window {

/**
 * @brief Result of handling a drag/drop event.
 */
struct DropEventOutcome {
  bool handled = false;
  bool accepted = false;
  QStringList files;
};

/**
 * @brief Extracts local file drops from Qt drag/drop events.
 */
class DropHandler {
public:
  /**
   * @brief Handles a drag, drag-move or drop event.
   * @param event Qt event to inspect.
   * @return Outcome describing whether the event was handled and accepted.
   */
  DropEventOutcome handle(QEvent* event) const;
};

} // namespace ui::window
