/**
 * @file ui/src/shell/window/DropHandler.cpp
 * @brief Implementation of the UI drop handling component.
 */

#include "ui/shell/window/DropHandler.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>

namespace ui::window {

namespace {

QStringList droppedLocalFiles(const QMimeData* mimeData) {
  QStringList files;
  if (!mimeData || !mimeData->hasUrls())
    return files;

  const auto urls = mimeData->urls();
  for (const auto& url : urls) {
    const QString localFile = url.toLocalFile();
    if (!localFile.isEmpty())
      files.push_back(localFile);
  }
  return files;
}

} // namespace

DropEventOutcome DropHandler::handle(QEvent* event) const {
  if (!event)
    return {};

  switch (event->type()) {
    case QEvent::DragEnter: {
      auto* dragEvent = static_cast<QDragEnterEvent*>(event);
      const QStringList files = droppedLocalFiles(dragEvent->mimeData());
      if (!files.isEmpty())
        dragEvent->acceptProposedAction();
      else
        dragEvent->ignore();
      return {true, !files.isEmpty(), files};
    }
    case QEvent::DragMove: {
      auto* dragEvent = static_cast<QDragMoveEvent*>(event);
      const QStringList files = droppedLocalFiles(dragEvent->mimeData());
      if (!files.isEmpty())
        dragEvent->acceptProposedAction();
      else
        dragEvent->ignore();
      return {true, !files.isEmpty(), files};
    }
    case QEvent::Drop: {
      auto* dropEvent = static_cast<QDropEvent*>(event);
      const QStringList files = droppedLocalFiles(dropEvent->mimeData());
      if (!files.isEmpty())
        dropEvent->acceptProposedAction();
      else
        dropEvent->ignore();
      return {true, !files.isEmpty(), files};
    }
    default:
      return {};
  }
}

} // namespace ui::window
