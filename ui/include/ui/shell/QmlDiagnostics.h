/**
 * @file ui/include/ui/shell/QmlDiagnostics.h
 * @brief QML runtime diagnostics reporting helpers.
 */

#pragma once

#include <QUrl>

class QQmlEngine;
class QQuickView;
class QObject;

namespace ui::bootstrap {

/**
 * @brief Reports QQuickView load errors through the structured error reporter.
 * @param quickView QML view whose load status should be inspected.
 * @param source Source URL or module marker used for context.
 */
void reportQmlLoadErrors(QQuickView* quickView, const QUrl& source);

/**
 * @brief Wires QQmlEngine runtime warnings into structured error reporting.
 * @param engine QML engine to observe.
 * @param context Qt context object owning the connection lifetime.
 */
void wireQmlWarnings(QQmlEngine* engine, QObject* context);

} // namespace ui::bootstrap
