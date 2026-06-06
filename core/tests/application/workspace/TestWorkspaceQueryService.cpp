/**
 * @file core/tests/application/workspace/TestWorkspaceQueryService.cpp
 * @brief Tests for workspace query projections.
 */

#include <gtest/gtest.h>

#include "core/application/workspace/WorkspaceQueryService.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceQueryServiceTest, ProjectsSessionStateIntoWorkspaceSnapshot) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alpha");
    session.mutableCatalogState().setActors({actor});

    auto statement = std::make_shared<core::domain::Statement>();
    statement->setId("statement-1");
    statement->rename("Statement");
    session.mutableCatalogState().setStatements({statement});

    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setName("Rent");
    transaction->setBookingDate("2024-01-31");
    transaction->setAmount(10.0);
    transaction->setStatementId("statement-1");
    session.mutableCatalogState().setTransactions({transaction});

    WorkspaceQueryService query(session);
    const auto snapshot = query.workspaceSnapshot();

    EXPECT_TRUE(snapshot.hasCurrentPath);
    EXPECT_EQ(snapshot.currentPath, "P:/workspace.db");
    EXPECT_EQ(snapshot.actors.size(), 1u);
    EXPECT_EQ(snapshot.statements.size(), 1u);
    EXPECT_EQ(snapshot.transactions.size(), 1u);
    EXPECT_EQ(query.currentPath(), "P:/workspace.db");
}

TEST(WorkspaceQueryServiceTest, ResolvesStatementDraftSnapshots) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    WorkspaceSession session(std::move(storage));

    auto draft = std::make_shared<core::application::importing::draft::StatementDraft>();
    draft->id = "draft-1";
    draft->name = "Draft";
    session.mutableState().workflow.statementDrafts.push_back(draft);

    WorkspaceQueryService query(session);

    const auto firstDraft = query.statementDraftSnapshot();
    ASSERT_TRUE(firstDraft.has_value());
    EXPECT_EQ(firstDraft->id, "draft-1");

    const auto missingDraft = query.statementDraftSnapshot("missing");
    EXPECT_FALSE(missingDraft.has_value());
}

} // namespace core::application
