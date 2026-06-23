/**
 * @file core/tests/domain/entities/TestContract.cpp
 * @brief Tests for contract behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Contract.h"

namespace core::domain {

namespace {

Alias MakeAlias(const std::string& value)
{
    return Alias{value, "kind", value, "created", "updated", 1, "last"};
}

} // namespace

TEST(ContractTest, ConfiguresNameTypeAndRelations) {
    Contract contract;

    contract.rename("  Lease Contract  ");
    contract.setType("  rent  ");
    contract.setActorIds({" actor-a ", "actor-a", "actor-b"});
    contract.setPropertyIds({" property-a ", "property-a"});

    EXPECT_EQ(contract.name(), "Lease Contract");
    EXPECT_EQ(contract.type(), "rent");
    EXPECT_EQ(contract.actorCount(), 2u);
    EXPECT_EQ(contract.propertyCount(), 1u);
    EXPECT_TRUE(contract.hasType());
    EXPECT_TRUE(contract.hasRelations());
    EXPECT_FALSE(contract.isStandalone());
    EXPECT_TRUE(contract.isConfigured());
}

TEST(ContractTest, NormalizesAliasesAndMatchesCanonicalValues) {
    Contract contract;
    contract.rename("  Lease Contract  ");
    contract.setType(" rent ");

    contract.addAlias(MakeAlias("alpha"));
    contract.addAlias(MakeAlias(" alpha "));
    contract.addAlias(MakeAlias("beta"));

    EXPECT_EQ(contract.aliasCount(), 2u);
    EXPECT_TRUE(contract.hasAlias("alpha"));
    EXPECT_TRUE(contract.isMatchingReady());
}

TEST(ContractTest, RemovesRelationAndAliasEntriesDeterministically) {
    Contract contract;
    contract.setActorIds({"actor-a", "actor-b"});
    contract.setPropertyIds({"property-a"});
    contract.setAliases({MakeAlias("alpha"), MakeAlias("beta")});

    contract.removeActorId(" actor-a ");
    contract.removePropertyId("property-a");
    contract.removeAlias("beta");

    EXPECT_FALSE(contract.containsActorId("actor-a"));
    EXPECT_FALSE(contract.containsPropertyId("property-a"));
    EXPECT_EQ(contract.relationCount(), 1u);
    EXPECT_EQ(contract.aliasCount(), 1u);
}

} // namespace core::domain
