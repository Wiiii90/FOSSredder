/**
 * @file core/tests/application/import/draft/TestDraftMatcherProjection.cpp
 * @brief Tests for draft matcher projection helpers.
 */

#include <gtest/gtest.h>

#include "core/application/import/draft/DraftMatcher.h"

namespace core::application::importing::draft {

TEST(DraftMatcherProjectionTest, BuildsDerivedStateForEmptySelection) {
    core::domain::catalog::WorkspaceCatalog state;
    DraftLinkSelection selection;

    const auto derived = buildDraftDerivedState(state, selection);

    EXPECT_EQ(derived.actorChoices.size(), 1u);
    EXPECT_EQ(derived.contractChoices.size(), 1u);
    EXPECT_TRUE(derived.propertyRows.empty());
    EXPECT_FALSE(derived.effectiveAllocatable);
}

TEST(DraftMatcherProjectionTest, DerivesContractTopSuggestionFromScoredContracts) {
    core::domain::catalog::WorkspaceCatalog state;

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alice");
    state.setActors({actor});

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Power Main Building");
    contract->setType("strom");
    contract->setActorIds({"actor-1"});
    state.setContracts({contract});

    DraftLinkSelection selection;
    selection.actorText = "Alice";
    selection.type = "strom";

    const auto derived = buildDraftDerivedState(state, selection);

    EXPECT_TRUE(derived.hasContractTopSuggestion);
    EXPECT_EQ(derived.contractTopSuggestion.entityId, std::string("contract-1"));
    EXPECT_GT(derived.contractTopSuggestion.confidence, 0.0);
}

TEST(DraftMatcherProjectionTest, UsesInferredContractForAllocatableDerivation) {
    core::domain::catalog::WorkspaceCatalog state;

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alice");
    state.setActors({actor});

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Power Main Building");
    contract->setType("strom");
    contract->setActorIds({"actor-1"});
    state.setContracts({contract});

    auto tx = std::make_shared<core::domain::Transaction>();
    tx->setId("tx-1");
    tx->setContractId("contract-1");
    tx->setAllocatable(true);
    state.setTransactions({tx});

    DraftLinkSelection selection;
    selection.actorText = "Alice";
    selection.type = "strom";
    selection.allocatable = false;
    selection.allocatableSelected = false;

    const auto derived = buildDraftDerivedState(state, selection);

    EXPECT_TRUE(derived.effectiveAllocatable);
}

TEST(DraftMatcherProjectionTest, ContractAllocatableModeOverridesDerivedAllocatable) {
    core::domain::catalog::WorkspaceCatalog state;

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alice");
    state.setActors({actor});

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Power Main Building");
    contract->setType("strom");
    contract->setActorIds({"actor-1"});
    contract->setAllocatableMode("non-allocatable");
    state.setContracts({contract});

    auto tx = std::make_shared<core::domain::Transaction>();
    tx->setId("tx-1");
    tx->setContractId("contract-1");
    tx->setAllocatable(true);
    state.setTransactions({tx});

    DraftLinkSelection selection;
    selection.actorText = "Alice";
    selection.type = "strom";
    selection.allocatable = false;
    selection.allocatableSelected = false;

    const auto derived = buildDraftDerivedState(state, selection);

    EXPECT_FALSE(derived.effectiveAllocatable);
}

} // namespace core::application::importing::draft
