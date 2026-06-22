/**
 * @file ui/include/ui/shell/QmlDiagnostics.h
 * @brief QML runtime diagnostics reporting helpers.
 */

#pragma once

#include <memory>

#include <QUrl>

class QQmlEngine;
class QQuickView;
class QObject;

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace ui::bootstrap {

/**
 * @brief Reports QQuickView load errors through the structured error reporter.
 * @param quickView QML view whose load status should be inspected.
 * @param source Source URL or module marker used for context.
 */
void reportQmlLoadErrors(QQuickView* quickView, const QUrl& source,
                         core::ports::diagnostics::IErrorReporter* reporter);

/**
 * @brief Wires QQmlEngine runtime warnings into structured error reporting.
 * @param engine QML engine to observe.
 * @param context Qt context object owning the connection lifetime.
 */
void wireQmlWarnings(
    QQmlEngine* engine, QObject* context,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> reporter);

} // namespace ui::bootstrap
