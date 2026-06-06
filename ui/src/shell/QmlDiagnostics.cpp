/**
 * @file ui/src/shell/QmlDiagnostics.cpp
 * @brief Implements QML runtime diagnostics reporting helpers.
 */

#include "ui/shell/QmlDiagnostics.h"

#include <QObject>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickView>

#include <string>

#include "core/errors/ErrorReporterRegistry.h"
#include "ui/observability/ErrorCodes.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"

namespace ui::bootstrap {
namespace {

core::errors::ErrorContext qmlErrorContext(const QQmlError& error) {
  core::errors::ErrorContext context;
  context.emplace_back(ui::observability::context::kUrl,
                       error.url().toString().toStdString());
  context.emplace_back(ui::observability::context::kLine,
                       std::to_string(error.line()));
  context.emplace_back(ui::observability::context::kColumn,
                       std::to_string(error.column()));
  return context;
}

void reportQmlError(core::errors::ErrorSeverity severity, const char* code,
                    const char* origin, const QQmlError& error) {
  core::errors::report(severity, code, origin, error.toString().toStdString(),
                       qmlErrorContext(error));
}

} // namespace

void reportQmlLoadErrors(QQuickView* quickView, const QUrl& source) {
  if (!quickView || quickView->status() != QQuickView::Error) {
    return;
  }

  const auto errors = quickView->errors();
  if (errors.isEmpty()) {
    core::errors::report(
        core::errors::ErrorSeverity::Error,
        ui::observability::codes::QmlLoadFailed,
        ui::observability::origins::qml::kLoad,
        "QQuickView failed to load the main QML source",
        {{ui::observability::context::kUrl, source.toString().toStdString()}});
    return;
  }

  for (const auto& error : errors) {
    reportQmlError(core::errors::ErrorSeverity::Error,
                   ui::observability::codes::QmlLoadFailed,
                   ui::observability::origins::qml::kLoad, error);
  }
}

void wireQmlWarnings(QQmlEngine* engine, QObject* context) {
  if (!engine || !context) {
    return;
  }

  QObject::connect(engine, &QQmlEngine::warnings, context,
                   [](const QList<QQmlError>& warnings) {
                     for (const auto& warning : warnings) {
                       reportQmlError(
                           core::errors::ErrorSeverity::Warning,
                           ui::observability::codes::QmlWarning,
                           ui::observability::origins::qml::kWarnings, warning);
                     }
                   });
}

} // namespace ui::bootstrap
