/**
 * @file ui/include/ui/shared/payload/PayloadMapper.h
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

#include "ui/shared/payload/PayloadKeys.h"

namespace ui::payload::mapper {

inline QStringList toQStringList(const std::vector<std::string> &values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto &value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

inline QVariantList
toVariantStringList(const std::vector<std::string> &values) {
  QVariantList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto &value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

inline QVariantList toVariantStringList(const QStringList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(value);
  }
  return out;
}

inline QStringList toQStringList(const QVariantList &values) {
  QStringList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(value.toString());
  }
  return out;
}

inline QVariantList toVariantList(const QVariant &value) {
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    return toVariantStringList(value.toStringList());
  }
  return value.toList();
}

inline QVariantMap mapAt(const QVariantList &rows, int index) {
  return index >= 0 && index < rows.size() ? rows.at(index).toMap()
                                           : QVariantMap{};
}

inline QString rowId(const QVariantMap &row) {
  return row.value(keys::common::kId).toString();
}

inline QString rowType(const QVariantMap &row) {
  return row.value(keys::common::kType).toString();
}

inline QString rowDisplayName(const QVariantMap &row) {
  return row.value(keys::common::kName, row.value(keys::common::kDisplay))
      .toString();
}

inline int rowIntValue(const QVariantMap &row, int fallback = 0) {
  return row.value(keys::common::kValue, fallback).toInt();
}

inline QString rowStringValue(const QVariantMap &row,
                              const QString &fallback = {}) {
  return row.value(keys::common::kValue, fallback).toString();
}

inline int indexById(const QVariantList &rows, const QString &id) {
  for (int index = 0; index < rows.size(); ++index) {
    if (rowId(mapAt(rows, index)) == id) {
      return index;
    }
  }
  return 0;
}

inline QString stringValue(const QVariantMap &map, const QString &key) {
  return map.value(key).toString();
}

inline QVariantList listValue(const QVariantMap &map, const QString &key) {
  return toVariantList(map.value(key));
}

inline QStringList stringListValue(const QVariantMap &map,
                                   const QString &key) {
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

inline QVariantList options(const QString &neutral, const QString &unverified,
                            const QString &verified,
                            const QString &completed) {
  return {QVariantMap{{keys::common::kLabel, neutral},
                      {keys::common::kValue, kNeutral}},
          QVariantMap{{keys::common::kLabel, unverified},
                      {keys::common::kValue, kUnverified}},
          QVariantMap{{keys::common::kLabel, verified},
                      {keys::common::kValue, kVerified}},
          QVariantMap{{keys::common::kLabel, completed},
                      {keys::common::kValue, kCompleted}}};
}

inline int statusAt(const QVariantList &options, int index) {
  return mapper::rowIntValue(mapper::mapAt(options, index), kNeutral);
}

inline int indexForStatus(const QVariantList &options, int status) {
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

inline QVariantList options(const QString &mixed, const QString &allocatable,
                            const QString &nonAllocatable) {
  return {QVariantMap{{keys::common::kLabel, mixed},
                      {keys::common::kValue, kMixed}},
          QVariantMap{{keys::common::kLabel, allocatable},
                      {keys::common::kValue, kAllocatable}},
          QVariantMap{{keys::common::kLabel, nonAllocatable},
                      {keys::common::kValue, kNonAllocatable}}};
}

inline QString modeAt(const QVariantList &options, int index) {
  return mapper::rowStringValue(mapper::mapAt(options, index), kMixed);
}

inline int indexForMode(const QVariantList &options, const QString &mode) {
  for (int index = 0; index < options.size(); ++index) {
    if (modeAt(options, index) == mode) {
      return index;
    }
  }
  return 0;
}

} // namespace ui::payload::contract_allocatable_mode
