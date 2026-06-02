/**
 * @file core/include/core/application/workspace/WorkspaceSnapshotCatalogMapper.h
 * @brief Maps workspace port snapshots into internal catalog state.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::application::workspace {

[[nodiscard]] core::domain::catalog::WorkspaceCatalog
toWorkspaceCatalog(const core::ports::workspace::WorkspaceSnapshot& snapshot);

} // namespace core::application::workspace
