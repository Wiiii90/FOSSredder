/**
 * @file ui/src/workspace/WorkspaceFacadeStorage.cpp
 * @brief Implements file storage commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui {

void WorkspaceFacade::setImportWorkflowForSave(ImportWorkflow *workflow) {
  importWorkflowForSave_ = workflow;
}

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
  runStorageOperation(QStringLiteral("saveFile"), [&]() {
    if (importWorkflowForSave_) {
      observability::traceWorkspace("WorkspaceFacade::saveFile",
                                    "Flushing import session before save");
      importWorkflowForSave_->flushSessionToWorkspace();
    }
    workspaceWriter_->saveFile();
  });
}

void WorkspaceFacade::saveFileAs(const QString &path) {
  runStorageOperation(QStringLiteral("saveFileAs"), [&]() {
    if (importWorkflowForSave_) {
      observability::traceWorkspace("WorkspaceFacade::saveFileAs",
                                    "Flushing import session before save-as");
      importWorkflowForSave_->flushSessionToWorkspace();
    }
    workspaceWriter_->saveFileAs(strings::toEncodedPath(path));
  });
}

} // namespace ui
