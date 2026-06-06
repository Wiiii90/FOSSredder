/**
 * @file ui/include/ui/shell/window/CloseWorkflow.h
 * @brief Declarations for the UI CloseWorkflow component.
 */

#pragma once

#include <functional>

#include <QString>

class QCloseEvent;

namespace ui::window {

/**
 * @brief Coordinates close requests that may need a pending workspace save.
 */
class CloseWorkflow {
public:
  /**
   * @brief Accepts a close event when a deferred close is already authorized.
   * @param event Close event received by the main window.
   * @return True when the event was accepted immediately.
   */
  bool allowImmediateClose(QCloseEvent* event);

  /**
   * @brief Defers closing and invokes the supplied save request.
   * @param event Close event that should wait for save completion.
   * @param requestSave Callback that starts the save operation.
   */
  void requestClose(QCloseEvent* event,
                    const std::function<void()>& requestSave);

  /**
   * @brief Completes a deferred close after the matching save succeeded.
   * @param operation Completed storage operation name.
   * @param saveOperation Operation name expected for close-time save.
   * @param closeWindow Callback that closes the main window.
   * @return True when the operation was handled as a pending close.
   */
  bool
  handleStorageOperationSucceeded(const QString& operation,
                                  const QString& saveOperation,
                                  const std::function<void()>& closeWindow);

  /**
   * @brief Cancels a deferred close after the matching save failed.
   * @param operation Failed storage operation name.
   * @param saveOperation Operation name expected for close-time save.
   * @return True when the operation was handled as a pending close.
   */
  bool handleStorageOperationFailed(const QString& operation,
                                    const QString& saveOperation);

private:
  bool pendingCloseAfterSave_ = false;
  bool allowImmediateClose_ = false;
};

} // namespace ui::window
