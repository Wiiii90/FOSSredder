/**
 * @file ui/include/ui/shared/observability/ErrorCodes.h
 * @brief Declarations for the UI ErrorCodes component.
 */

#pragma once

namespace ui::observability::codes {

inline constexpr const char *QtDebug = "QT_DEBUG";
inline constexpr const char *QtInfo = "QT_INFO";
inline constexpr const char *QtWarning = "QT_WARNING";
inline constexpr const char *QtCritical = "QT_CRITICAL";
inline constexpr const char *QtFatal = "QT_FATAL";

inline constexpr const char *QmlWarning = "QML_WARNING";
inline constexpr const char *QmlLoadFailed = "QML_LOAD_FAILED";

inline constexpr const char *FlowImportStarted = "UI_FLOW_IMPORT_STARTED";
inline constexpr const char *FlowImportFinished = "UI_FLOW_IMPORT_FINISHED";
inline constexpr const char *FlowImportCanceled = "UI_FLOW_IMPORT_CANCELED";
inline constexpr const char *FlowImportFailed = "UI_FLOW_IMPORT_FAILED";
inline constexpr const char *FlowImportRejected = "UI_FLOW_IMPORT_REJECTED";
inline constexpr const char *FlowExportStarted = "UI_FLOW_EXPORT_STARTED";
inline constexpr const char *FlowExportFinished = "UI_FLOW_EXPORT_FINISHED";
inline constexpr const char *FlowExportFailed = "UI_FLOW_EXPORT_FAILED";
inline constexpr const char *FlowExportFallback = "UI_FLOW_EXPORT_FALLBACK";
inline constexpr const char *FlowMainWindowAction = "UI_FLOW_MAINWINDOW_ACTION";

} // namespace ui::observability::codes
