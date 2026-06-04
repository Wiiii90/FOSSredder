/**
 * @file ui/include/ui/util/StringConversions.h
 * @brief Declares lightweight conversions between common Qt string types and
 * standard strings.
 */

#pragma once

#include <QFile>
#include <QString>
#include <QStringList>

#include <string>
#include <vector>

namespace ui::strings {

/** @brief Convert a `QString` to a UTF-8 encoded `std::string`. */
inline std::string toStdString(const QString &value) {
  const auto utf8 = value.toUtf8();
  return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

/** @brief Convert a filesystem path to the platform-encoded byte
 * representation. */
inline std::string toEncodedPath(const QString &path) {
  const auto encodedPath = QFile::encodeName(path);
  return std::string(encodedPath.constData(),
                     static_cast<size_t>(encodedPath.size()));
}

/** @brief Convert a Qt string list into a standard string vector. */
inline std::vector<std::string> toStdList(const QStringList &values) {
  std::vector<std::string> out;
  out.reserve(static_cast<size_t>(values.size()));
  for (const auto &value : values) {
    out.push_back(value.toStdString());
  }
  return out;
}

/** @brief Trim, collapse whitespace, and lower-case text for comparisons. */
inline QString normalizedText(const QString &value) {
  return value.trimmed().simplified().toLower();
}

} // namespace ui::strings
