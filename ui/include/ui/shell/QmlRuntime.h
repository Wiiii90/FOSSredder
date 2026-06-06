/**
 * @file ui/include/ui/shell/QmlRuntime.h
 * @brief QML runtime setup helpers for registering types and configuring the
 * engine.
 */

#pragma once

class QQmlEngine;

namespace ui::bootstrap {

/**
 * @brief Register QML types and meta objects used by the application.
 *
 * This function is idempotent and safe to call multiple times.
 */
void registerTypes();

/**
 * @brief Configure QQmlEngine import paths and plugin library paths.
 * @param engine QQmlEngine instance to configure (must not be null).
 */
void configureRuntime(QQmlEngine* engine);

} // namespace ui::bootstrap
