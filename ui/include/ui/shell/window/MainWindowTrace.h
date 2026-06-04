/**
 * @file ui/include/ui/shell/window/MainWindowTrace.h
 * @brief Declarations for the UI MainWindowTrace component.
 */

#pragma once

#include <string>
#include <utility>

#include <QString>
#include <QStringList>

#include "ui/observability/Trace.h"

namespace ui::window {

inline core::errors::ErrorContext makePathContext(const QString &path) {
  return {{ui::observability::context::kPath, path.toStdString()}};
}

inline core::errors::ErrorContext
makeFileListContext(const QStringList &files) {
  core::errors::ErrorContext context{
      {ui::observability::context::kCount, std::to_string(files.size())}};
  if (!files.isEmpty())
    context.emplace_back(ui::observability::context::kFirstFile,
                         files.front().toStdString());
  return context;
}

inline void reportMainWindowFlow(
    const char *origin, std::string message,
    core::errors::ErrorSeverity severity = core::errors::ErrorSeverity::Info,
    core::errors::ErrorContext context = {}) {
  ui::observability::reportFlow(severity,
                                ui::observability::codes::FlowMainWindowAction,
                                origin, std::move(message), std::move(context));
}

} // namespace ui::window
