/**
 * @file ui/tests/support/ViewModelTestHarness.h
 * @brief Shared harness helpers for UI view-model boundary tests.
 */

#pragma once

#include <memory>
#include <utility>

#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "support/WorkspacePortFakes.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui::tests::support {

class NoopErrorReporter final : public core::ports::diagnostics::IErrorReporter {
public:
  void report(const core::errors::ErrorEvent&) override {}
};

struct WorkspaceHarness {
  explicit WorkspaceHarness(
      core::ports::workspace::WorkspaceSnapshot snapshot = {})
      : workspace(std::make_unique<InMemoryWorkspace>(std::move(snapshot))),
        store(std::make_unique<WorkspaceStore>()),
        selectors(std::make_unique<WorkspaceSelectors>(*store)),
        selection(std::make_unique<WorkspaceSelection>(*store, *selectors)),
        commands(std::make_unique<WorkspaceCommands>(*store)) {
    store->setWorkspacePorts(workspace.get(), workspace.get());
    store->loadFromState(workspace->workspaceSnapshot());
    workspace->setSnapshotChangedCallback(
        [this](const core::ports::workspace::WorkspaceSnapshot& nextSnapshot) {
          store->loadFromState(nextSnapshot);
        });
  }

  std::unique_ptr<InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceStore> store;
  std::unique_ptr<WorkspaceSelectors> selectors;
  std::unique_ptr<WorkspaceSelection> selection;
  std::unique_ptr<WorkspaceCommands> commands;
};

inline std::shared_ptr<NoopErrorReporter> noopErrorReporter() {
  return std::make_shared<NoopErrorReporter>();
}

} // namespace ui::tests::support
