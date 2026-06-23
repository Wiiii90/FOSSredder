/**
 * @file core/tests/domain/catalog/TestWorkspaceCatalog.cpp
 * @brief Tests for the workspace catalog aggregate.
 */

#include <gtest/gtest.h>

#include "core/domain/catalog/WorkspaceCatalog.h"

namespace core::domain::catalog {

TEST(WorkspaceCatalogTest, TracksEmptyAndReplacesCollections) {
    WorkspaceCatalog catalog;

    EXPECT_TRUE(catalog.empty());

    WorkspaceCatalog::ActorList actors;
    actors.push_back(std::make_shared<core::domain::Actor>());
    catalog.setActors(std::move(actors));

    EXPECT_FALSE(catalog.empty());
    EXPECT_EQ(catalog.actors().size(), 1u);
    EXPECT_EQ(catalog.properties().size(), 0u);
}

TEST(WorkspaceCatalogTest, HoldsIndependentCollections) {
    WorkspaceCatalog catalog;

    WorkspaceCatalog::PropertyList properties;
    properties.push_back(std::make_shared<core::domain::Property>());
    WorkspaceCatalog::ContractList contracts;
    contracts.push_back(std::make_shared<core::domain::Contract>());
    WorkspaceCatalog::StatementList statements;
    statements.push_back(std::make_shared<core::domain::Statement>());
    WorkspaceCatalog::TransactionList transactions;
    transactions.push_back(std::make_shared<core::domain::Transaction>());
    WorkspaceCatalog::AnalysisList analyses;
    analyses.push_back(std::make_shared<core::domain::Analysis>());
    WorkspaceCatalog::AnnualList annuals;
    annuals.push_back(std::make_shared<core::domain::Annual>());

    catalog.setProperties(std::move(properties));
    catalog.setContracts(std::move(contracts));
    catalog.setStatements(std::move(statements));
    catalog.setTransactions(std::move(transactions));
    catalog.setAnalyses(std::move(analyses));
    catalog.setAnnuals(std::move(annuals));

    EXPECT_EQ(catalog.properties().size(), 1u);
    EXPECT_EQ(catalog.contracts().size(), 1u);
    EXPECT_EQ(catalog.statements().size(), 1u);
    EXPECT_EQ(catalog.transactions().size(), 1u);
    EXPECT_EQ(catalog.analyses().size(), 1u);
    EXPECT_EQ(catalog.annuals().size(), 1u);
}

} // namespace core::domain::catalog
