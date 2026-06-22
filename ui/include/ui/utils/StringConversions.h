/**
 * @file ui/include/ui/utils/StringConversions.h
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

/**
 * @brief Converts a QString to a UTF-8 encoded std::string.
 * @param value Qt string value.
 * @return UTF-8 encoded standard string.
 */
inline std::string toStdString(const QString& value) {
  const auto utf8 = value.toUtf8();
  return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

/**
 * @brief Converts a filesystem path to the platform-encoded byte
 * representation.
 * @param path Qt filesystem path.
 * @return Platform-encoded path string.
 */
inline std::string toEncodedPath(const QString& path) {
  const auto encodedPath = QFile::encodeName(path);
  return std::string(encodedPath.constData(),
                     static_cast<size_t>(encodedPath.size()));
}

/**
 * @brief Converts a Qt string list into a standard string vector.
 * @param values Qt string list.
 * @return Standard string vector.
 */
inline std::vector<std::string> toStdList(const QStringList& values) {
  std::vector<std::string> out;
  out.reserve(static_cast<size_t>(values.size()));
  for (const auto& value : values) {
    out.push_back(value.toStdString());
  }
  return out;
}

/**
 * @brief Normalizes text for case-insensitive UI comparisons.
 * @param value Input text.
 * @return Trimmed, simplified, lower-case text.
 */
inline QString normalizedText(const QString& value) {
  return value.trimmed().simplified().toLower();
}

} // namespace ui::strings
