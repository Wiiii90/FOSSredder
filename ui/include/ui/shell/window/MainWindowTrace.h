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

/**
 * @brief Builds trace context for a single filesystem path.
 * @param path Filesystem path reported by the main window flow.
 * @return Error context containing the path.
 */
inline core::errors::ErrorContext makePathContext(const QString& path) {
  return {{ui::observability::context::kPath, path.toStdString()}};
}

/**
 * @brief Builds trace context for a list of selected or dropped files.
 * @param files Files selected through a dialog or dropped into the UI.
 * @return Error context containing the count and first file when available.
 */
inline core::errors::ErrorContext
makeFileListContext(const QStringList& files) {
  core::errors::ErrorContext context{
      {ui::observability::context::kCount, std::to_string(files.size())}};
  if (!files.isEmpty())
    context.emplace_back(ui::observability::context::kFirstFile,
                         files.front().toStdString());
  return context;
}

/**
 * @brief Emits a structured main-window flow diagnostic.
 * @param origin Stable origin identifier.
 * @param message Human-readable diagnostic message.
 * @param severity Diagnostic severity.
 * @param context Optional structured diagnostic context.
 */
inline void reportMainWindowFlow(
    core::ports::diagnostics::IErrorReporter* reporter,
    const char* origin, std::string message,
    core::errors::ErrorSeverity severity = core::errors::ErrorSeverity::Info,
    core::errors::ErrorContext context = {}) {
  ui::observability::reportFlow(reporter, severity,
                                ui::observability::codes::FlowMainWindowAction,
                                origin, std::move(message), std::move(context));
}

} // namespace ui::window
