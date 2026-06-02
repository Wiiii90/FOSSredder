/**
 * @file ui/include/ui/shell/window/CloseWorkflow.h
 * @brief Declarations for the UI CloseWorkflow component.
 */

#pragma once

#include <functional>

#include <QString>

class QCloseEvent;

namespace ui::window {

class CloseWorkflow {
public:
  bool allowImmediateClose(QCloseEvent *event);
  void requestClose(QCloseEvent *event,
                    const std::function<void()> &requestSave);
  bool
  handleStorageOperationSucceeded(const QString &operation,
                                  const QString &saveOperation,
                                  const std::function<void()> &closeWindow);
  bool handleStorageOperationFailed(const QString &operation,
                                    const QString &saveOperation);

private:
  bool pendingCloseAfterSave_ = false;
  bool allowImmediateClose_ = false;
};

} // namespace ui::window
