/**
 * @file ui/src/shell/window/CloseWorkflow.cpp
 * @brief Implementation of the UI CloseWorkflow component.
 */

#include "ui/shell/window/CloseWorkflow.h"

#include <QCloseEvent>

namespace ui::window {

bool CloseWorkflow::allowImmediateClose(QCloseEvent* event) {
  if (!allowImmediateClose_)
    return false;
  allowImmediateClose_ = false;
  if (event)
    event->accept();
  return true;
}

void CloseWorkflow::requestClose(QCloseEvent* event,
                                 const std::function<void()>& requestSave) {
  if (!event)
    return;
  if (pendingCloseAfterSave_) {
    event->ignore();
    return;
  }

  pendingCloseAfterSave_ = true;
  event->ignore();
  try {
    if (requestSave)
      requestSave();
  } catch (...) {
    pendingCloseAfterSave_ = false;
    allowImmediateClose_ = true;
  }
}

bool CloseWorkflow::handleStorageOperationSucceeded(
    const QString& operation, const QString& saveOperation,
    const std::function<void()>& closeWindow) {
  if (!pendingCloseAfterSave_ || operation != saveOperation)
    return false;

  pendingCloseAfterSave_ = false;
  allowImmediateClose_ = true;
  if (closeWindow)
    closeWindow();
  return true;
}

bool CloseWorkflow::handleStorageOperationFailed(const QString& operation,
                                                 const QString& saveOperation) {
  if (!pendingCloseAfterSave_ || operation != saveOperation)
    return false;

  pendingCloseAfterSave_ = false;
  allowImmediateClose_ = true;
  return true;
}

} // namespace ui::window
