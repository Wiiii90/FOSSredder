/**
 * @file core/tests/application/workspace/TestWorkspaceStateManager.cpp
 * @brief Tests for workspace state manager load/save behavior.
 */

#include <gtest/gtest.h>

#include "core/application/storage/RepositoryBundle.h"
#include "core/application/workspace/WorkspaceStateManager.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/domain/entities/Contract.h"

namespace core::application {

TEST(WorkspaceStateManagerTest, LoadsEmptyStateFromEmptyRepositoryBundle) {
    WorkspaceStateManager manager(core::storage::RepositoryBundle{});

    const auto state = manager.load();

    EXPECT_TRUE(state.empty());
}

TEST(WorkspaceStateManagerTest, RejectsInvalidStateWhenStrictValidationIsEnabled) {
    WorkspaceStateManager manager(core::storage::RepositoryBundle{});
    manager.setStrictValidation(true);

    workspace::WorkspaceSessionState state;
    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Lease");
    contract->setType("rent");
    state.catalog.setContracts({contract});

    EXPECT_THROW(manager.save(state), std::runtime_error);
}

} // namespace core::application
