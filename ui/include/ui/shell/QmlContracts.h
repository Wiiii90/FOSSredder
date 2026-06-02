/**
 * @file ui/include/ui/shell/QmlContracts.h
 * @brief QML registration and context property name constants used by the UI.
 */

#pragma once

#include <QObject>

namespace ui::qml::contracts {

Q_NAMESPACE

/** Module/version and type registration metadata for the QML module. */
namespace module {
inline constexpr auto kName = "FossRedder";
inline constexpr auto kMainTypeName = "Main";
constexpr int kMajorVersion = 1;
constexpr int kMinorVersion = 0;
inline constexpr auto kAppContextTypeName = "AppContext";
inline constexpr auto kQmlContractsTypeName = "QmlContracts";
inline constexpr auto kQmlContractsTypeDescription =
    "QML contracts are exposed as enums only";
inline constexpr auto kNavigationTypeName = "Navigation";
inline constexpr auto kNavigationTypeDescription =
    "Navigation is exposed through AppContext";
} // namespace module

/** Convenience property keys used by QML components. */
namespace properties {
inline constexpr auto kWidth = "width";
inline constexpr auto kHeight = "height";
} // namespace properties

/** Reusable QML labels and object names. */
namespace labels {
inline constexpr auto kAdd = "+";
inline constexpr auto kRemove = "-";
inline constexpr auto kCreate = "Create";
inline constexpr auto kClear = "Clear";
inline constexpr auto kDelete = "Delete";
inline constexpr auto kUpdate = "Update";
inline constexpr auto kActorCreateModeButton = "actorCreateModeButton";
inline constexpr auto kPropertyCreateModeButton = "propertyCreateModeButton";
inline constexpr auto kContractCreateModeButton = "contractCreateModeButton";
} // namespace labels

/** Supported export formats exposed to QML. */
enum class ExportFormat : int { Csv = 0, Xlsx = 1 };

Q_ENUM_NS(ExportFormat)

enum class NavigationValue : int {
  SectionActors = 0,
  SectionProperties = 1,
  SectionContracts = 2,
  SectionBooking = 3,
  SectionImport = 4,
  SectionExport = 5,
  SectionSettings = 6,
  SectionAnalysis = 7,
  SectionAnnual = 8,
  BookingStatements = 0,
  BookingCalendar = 1,
  BookingTransactions = 2,
  SettingsGeneral = 0,
  SettingsImport = 1,
  SettingsExport = 2,
  SettingsMiscellaneous = 3
};

Q_ENUM_NS(NavigationValue)

} // namespace ui::qml::contracts
