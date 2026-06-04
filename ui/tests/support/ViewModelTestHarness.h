/**
 * @file ui/tests/support/ViewModelTestHarness.h
 * @brief Shared harness helpers for UI view-model boundary tests.
 */

#pragma once

#include <memory>
#include <utility>

#include "core/errors/IErrorReporter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "support/WorkspacePortFakes.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui::tests::support {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
  void report(const core::errors::ErrorEvent &) override {}
};

struct WorkspaceHarness {
  explicit WorkspaceHarness(
      core::ports::workspace::WorkspaceSnapshot snapshot = {})
      : workspace(std::make_unique<InMemoryWorkspace>(std::move(snapshot))),
        facade(std::make_unique<WorkspaceFacade>(workspace.get(),
                                                workspace.get())) {
    workspace->setSnapshotChangedCallback([this](
        const core::ports::workspace::WorkspaceSnapshot &nextSnapshot) {
      facade->loadFromState(nextSnapshot);
    });
  }

  std::unique_ptr<InMemoryWorkspace> workspace;
  std::unique_ptr<WorkspaceFacade> facade;
};

inline std::shared_ptr<NoopErrorReporter> noopErrorReporter() {
  return std::make_shared<NoopErrorReporter>();
}

} // namespace ui::tests::support
