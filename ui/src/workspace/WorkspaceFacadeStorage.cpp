/**
 * @file ui/src/workspace/WorkspaceFacadeStorage.cpp
 * @brief Implements file storage commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include "ui/shared/util/StringConversions.h"

namespace ui {

void WorkspaceFacade::newFile(const QString &path) {
  runStorageOperation(QStringLiteral("newFile"), [&]() {
    workspaceWriter_->newFile(strings::toEncodedPath(path));
  });
}

void WorkspaceFacade::openFile(const QString &path) {
  runStorageOperation(QStringLiteral("openFile"), [&]() {
    workspaceWriter_->openFile(strings::toEncodedPath(path));
  });
}

void WorkspaceFacade::saveFile() {
  runStorageOperation(QStringLiteral("saveFile"),
                      [&]() { workspaceWriter_->saveFile(); });
}

void WorkspaceFacade::saveFileAs(const QString &path) {
  runStorageOperation(QStringLiteral("saveFileAs"), [&]() {
    workspaceWriter_->saveFileAs(strings::toEncodedPath(path));
  });
}

} // namespace ui
