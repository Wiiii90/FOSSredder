/**
 * @file ui/src/observability/Trace.cpp
 * @brief Implements the debug-only UI trace channel.
 */

#include "ui/observability/Trace.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QProcessEnvironment>

namespace ui::observability {
namespace {

bool debugBuild() {
#if defined(QT_DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
  return true;
#else
  return false;
#endif
}

bool envAllowsTrace() {
  const auto env = QProcessEnvironment::systemEnvironment();
  const QString value = env.value(QStringLiteral("FOSSREDDER_UI_TRACE"));
  return value.compare(QStringLiteral("0"), Qt::CaseInsensitive) != 0 &&
         value.compare(QStringLiteral("false"), Qt::CaseInsensitive) != 0 &&
         value.compare(QStringLiteral("off"), Qt::CaseInsensitive) != 0;
}

QString contextText(const core::errors::ErrorContext &context) {
  QStringList parts;
  parts.reserve(static_cast<int>(context.size()));
  for (const auto &[key, value] : context) {
    parts.push_back(QStringLiteral("%1=%2")
                        .arg(QString::fromStdString(key),
                             QString::fromStdString(value)));
  }
  return parts.join(QStringLiteral(" "));
}

} // namespace

bool isTraceEnabled() { return debugBuild() && envAllowsTrace(); }

void trace(const char *layer, const char *origin, std::string message,
           core::errors::ErrorContext context) {
  if (!isTraceEnabled()) {
    return;
  }

  const QString layerText = QString::fromUtf8(layer ? layer : "ui");
  const QString originText = QString::fromUtf8(origin ? origin : "unknown");
  QString line = QStringLiteral("[ui.trace] %1 %2: %3")
                     .arg(layerText, originText,
                          QString::fromStdString(std::move(message)));
  const QString details = contextText(context);
  if (!details.isEmpty()) {
    line += QStringLiteral(" | %1").arg(details);
  }
  qInfo().noquote() << line;
}

} // namespace ui::observability
