/**
 * @file ui/include/ui/presentation/PayloadMapper.h
 * @brief Declarations for the UI payload mapper helpers.
 */

#pragma once

#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <string>
#include <vector>

#include "ui/presentation/PayloadKeys.h"

namespace ui::payload::mapper {

/**
 * @brief Converts standard strings to a Qt string list.
 * @param values Standard string values.
 * @return Qt string list.
 */
inline QStringList toQStringList(const std::vector<std::string>& values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

/**
 * @brief Converts standard strings to a QVariant string list.
 * @param values Standard string values.
 * @return QVariantList containing QString values.
 */
inline QVariantList
toVariantStringList(const std::vector<std::string>& values) {
  QVariantList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

/**
 * @brief Converts Qt strings to a QVariant string list.
 * @param values Qt string values.
 * @return QVariantList containing QString values.
 */
inline QVariantList toVariantStringList(const QStringList& values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto& value : values) {
    out.push_back(value);
  }
  return out;
}

/**
 * @brief Converts a QVariant list to a Qt string list.
 * @param values QVariant values.
 * @return Qt string list.
 */
inline QStringList toQStringList(const QVariantList& values) {
  QStringList out;
  out.reserve(values.size());
  for (const auto& value : values) {
    out.push_back(value.toString());
  }
  return out;
}

/**
 * @brief Converts a QVariant value to a QVariantList.
 * @param value QVariant that may contain QStringList or QVariantList.
 * @return QVariantList projection.
 */
inline QVariantList toVariantList(const QVariant& value) {
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    return toVariantStringList(value.toStringList());
  }
  return value.toList();
}

/**
 * @brief Returns a map at an index in a row list.
 * @param rows QVariant row list.
 * @param index Row index.
 * @return Row map or an empty map.
 */
inline QVariantMap mapAt(const QVariantList& rows, int index) {
  return index >= 0 && index < rows.size() ? rows.at(index).toMap()
                                           : QVariantMap{};
}

/**
 * @brief Reads a row id.
 * @param row Row map.
 * @return Row id.
 */
inline QString rowId(const QVariantMap& row) {
  return row.value(keys::common::kId).toString();
}

/**
 * @brief Reads a row type.
 * @param row Row map.
 * @return Row type.
 */
inline QString rowType(const QVariantMap& row) {
  return row.value(keys::common::kType).toString();
}

/**
 * @brief Reads a row display name.
 * @param row Row map.
 * @return Row name or display text.
 */
inline QString rowDisplayName(const QVariantMap& row) {
  return row.value(keys::common::kName, row.value(keys::common::kDisplay))
      .toString();
}

/**
 * @brief Reads an integer row value.
 * @param row Row map.
 * @param fallback Value returned when the row has no integer value.
 * @return Row integer value.
 */
inline int rowIntValue(const QVariantMap& row, int fallback = 0) {
  return row.value(keys::common::kValue, fallback).toInt();
}

/**
 * @brief Reads a string row value.
 * @param row Row map.
 * @param fallback Value returned when the row has no string value.
 * @return Row string value.
 */
inline QString rowStringValue(const QVariantMap& row,
                              const QString& fallback = {}) {
  return row.value(keys::common::kValue, fallback).toString();
}

/**
 * @brief Finds the index of a row id.
 * @param rows QVariant row list.
 * @param id Row id to find.
 * @return Matching index or 0.
 */
inline int indexById(const QVariantList& rows, const QString& id) {
  for (int index = 0; index < rows.size(); ++index) {
    if (rowId(mapAt(rows, index)) == id) {
      return index;
    }
  }
  return 0;
}

/**
 * @brief Reads a string value from a map.
 * @param map Source map.
 * @param key Value key.
 * @return String value.
 */
inline QString stringValue(const QVariantMap& map, const QString& key) {
  return map.value(key).toString();
}

/**
 * @brief Reads a list value from a map.
 * @param map Source map.
 * @param key Value key.
 * @return List value.
 */
inline QVariantList listValue(const QVariantMap& map, const QString& key) {
  return toVariantList(map.value(key));
}

/**
 * @brief Reads a string list value from a map.
 * @param map Source map.
 * @param key Value key.
 * @return String list value.
 */
inline QStringList stringListValue(const QVariantMap& map, const QString& key) {
  const QVariant value = map.value(key);
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    return value.toStringList();
  }
  return toQStringList(value.toList());
}

} // namespace ui::payload::mapper

namespace ui::payload::transaction_status {

inline constexpr int kNeutral = 0;
inline constexpr int kUnverified = 1;
inline constexpr int kVerified = 2;
inline constexpr int kCompleted = 3;

/**
 * @brief Builds transaction status options.
 * @param neutral Neutral label.
 * @param unverified Unverified label.
 * @param verified Verified label.
 * @param completed Completed label.
 * @return Transaction status option rows.
 */
inline QVariantList options(const QString& neutral, const QString& unverified,
                            const QString& verified, const QString& completed) {
  return {QVariantMap{{keys::common::kLabel, neutral},
                      {keys::common::kValue, kNeutral}},
          QVariantMap{{keys::common::kLabel, unverified},
                      {keys::common::kValue, kUnverified}},
          QVariantMap{{keys::common::kLabel, verified},
                      {keys::common::kValue, kVerified}},
          QVariantMap{{keys::common::kLabel, completed},
                      {keys::common::kValue, kCompleted}}};
}

/**
 * @brief Reads the status value at an option index.
 * @param options Transaction status option rows.
 * @param index Option index.
 * @return Transaction status value.
 */
inline int statusAt(const QVariantList& options, int index) {
  return mapper::rowIntValue(mapper::mapAt(options, index), kNeutral);
}

/**
 * @brief Finds the option index for a transaction status.
 * @param options Transaction status option rows.
 * @param status Transaction status value.
 * @return Matching option index or 0.
 */
inline int indexForStatus(const QVariantList& options, int status) {
  for (int index = 0; index < options.size(); ++index) {
    if (statusAt(options, index) == status) {
      return index;
    }
  }
  return 0;
}

} // namespace ui::payload::transaction_status

namespace ui::payload::contract_allocatable_mode {

inline const auto kMixed = QStringLiteral("mixed");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kNonAllocatable = QStringLiteral("non-allocatable");

/**
 * @brief Builds contract allocatable mode options.
 * @param mixed Mixed-mode label.
 * @param allocatable Allocatable label.
 * @param nonAllocatable Non-allocatable label.
 * @return Contract allocatable mode option rows.
 */
inline QVariantList options(const QString& mixed, const QString& allocatable,
                            const QString& nonAllocatable) {
  return {QVariantMap{{keys::common::kLabel, mixed},
                      {keys::common::kValue, kMixed}},
          QVariantMap{{keys::common::kLabel, allocatable},
                      {keys::common::kValue, kAllocatable}},
          QVariantMap{{keys::common::kLabel, nonAllocatable},
                      {keys::common::kValue, kNonAllocatable}}};
}

/**
 * @brief Reads the allocatable mode at an option index.
 * @param options Allocatable mode option rows.
 * @param index Option index.
 * @return Allocatable mode key.
 */
inline QString modeAt(const QVariantList& options, int index) {
  return mapper::rowStringValue(mapper::mapAt(options, index), kMixed);
}

/**
 * @brief Finds the option index for an allocatable mode.
 * @param options Allocatable mode option rows.
 * @param mode Allocatable mode key.
 * @return Matching option index or 0.
 */
inline int indexForMode(const QVariantList& options, const QString& mode) {
  for (int index = 0; index < options.size(); ++index) {
    if (modeAt(options, index) == mode) {
      return index;
    }
  }
  return 0;
}

} // namespace ui::payload::contract_allocatable_mode
