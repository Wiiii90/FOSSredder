/**
 * @file core/tests/application/workspace/TestWorkspaceCommandService.cpp
 * @brief Tests for workspace command routing and catalog commits.
 */

#include <gtest/gtest.h>

#include <algorithm>

#include "core/application/workspace/WorkspaceCommandService.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "core/domain/entities/Statement.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceCommandServiceTest, RoutesCatalogCommandsThroughInMemorySession) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::StatementCommand addCommand;
    addCommand.name = "Statement";
    const auto statementId = service.addStatement(addCommand);

    EXPECT_FALSE(statementId.empty());
    EXPECT_EQ(session.catalogState().statements().size(), 1u);
    ASSERT_FALSE(session.catalogState().statements().empty());
    EXPECT_EQ(session.catalogState().statements().front()->name(), "Statement");
    EXPECT_TRUE(storagePtr->savedState_.catalog.statements().empty());

    core::ports::workspace::StatementCommand updateCommand;
    updateCommand.id = statementId;
    updateCommand.name = "Statement Updated";
    service.updateStatement(updateCommand);

    ASSERT_FALSE(session.catalogState().statements().empty());
    EXPECT_EQ(session.catalogState().statements().front()->name(), "Statement Updated");

    service.deleteStatement(statementId);

    EXPECT_TRUE(session.catalogState().statements().empty());

    session.saveFile();
    EXPECT_TRUE(storagePtr->savedState_.catalog.statements().empty());
}

TEST(WorkspaceCommandServiceTest, InsertsTransactionAfterRequestedStatementTransactionAndKeepsValuta) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::StatementCommand statementCommand;
    statementCommand.name = "Statement";
    const auto statementId = service.addStatement(statementCommand);
    ASSERT_FALSE(statementId.empty());

    core::ports::workspace::TransactionCommand first;
    first.name = "First";
    first.bookingDate = "2026-01-01";
    first.valuta = "2026-01-03";
    first.amount = 10.0;
    first.statementId = statementId;
    const auto firstId = service.addTransaction(first);
    ASSERT_FALSE(firstId.empty());

    core::ports::workspace::TransactionCommand second = first;
    second.name = "Second";
    second.bookingDate = "2026-01-02";
    second.valuta = "2026-01-04";
    const auto secondId = service.addTransaction(second);
    ASSERT_FALSE(secondId.empty());

    core::ports::workspace::TransactionCommand inserted = first;
    inserted.name = "Inserted";
    inserted.bookingDate = "2026-01-05";
    inserted.valuta = "2026-01-06";
    inserted.insertAfterTransactionId = firstId;
    const auto insertedId = service.addTransaction(inserted);
    ASSERT_FALSE(insertedId.empty());

    ASSERT_EQ(session.catalogState().statements().size(), 1u);
    const auto& ids = session.catalogState().statements().front()->transactionIds();
    ASSERT_EQ(ids.size(), 3u);
    EXPECT_EQ(ids[0], firstId);
    EXPECT_EQ(ids[1], insertedId);
    EXPECT_EQ(ids[2], secondId);

    const auto transactions = session.catalogState().transactions();
    const auto match = std::find_if(transactions.begin(), transactions.end(), [&](const auto& tx) {
        return tx && tx->id() == insertedId;
    });
    ASSERT_NE(match, transactions.end());
    EXPECT_EQ((*match)->valuta(), "2026-01-06");
}

TEST(WorkspaceCommandServiceTest, TransactionAllocatableChangeForcesContractModeToMixed) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::ActorCommand actor;
    actor.name = "Actor A";
    const auto actorId = service.addActor(actor);
    ASSERT_FALSE(actorId.empty());

    core::ports::workspace::PropertyCommand property;
    property.name = "Property A";
    const auto propertyId = service.addProperty(property);
    ASSERT_FALSE(propertyId.empty());

    core::ports::workspace::ContractCommand contract;
    contract.name = "Contract A";
    contract.type = "strom";
    contract.allocatableMode = "allocatable";
    contract.actorIds = {actorId};
    contract.propertyIds = {propertyId};
    const auto contractId = service.addContract(contract);
    ASSERT_FALSE(contractId.empty());

    core::ports::workspace::StatementCommand statement;
    statement.name = "Statement";
    const auto statementId = service.addStatement(statement);
    ASSERT_FALSE(statementId.empty());

    core::ports::workspace::TransactionCommand tx;
    tx.name = "Tx";
    tx.bookingDate = "2026-01-01";
    tx.valuta = "2026-01-02";
    tx.amount = 10.0;
    tx.statementId = statementId;
    tx.contractId = contractId;
    tx.allocatable = true;
    const auto txId = service.addTransaction(tx);
    ASSERT_FALSE(txId.empty());

    tx.id = txId;
    tx.allocatable = false;
    service.updateTransaction(tx);

    ASSERT_FALSE(session.catalogState().contracts().empty());
    const auto contractIt = std::find_if(session.catalogState().contracts().begin(),
                                         session.catalogState().contracts().end(),
                                         [&](const auto& c) { return c && c->id() == contractId; });
    ASSERT_NE(contractIt, session.catalogState().contracts().end());
    EXPECT_EQ((*contractIt)->allocatableMode(), "mixed");
}

TEST(WorkspaceCommandServiceTest, AssigningContractToAnotherActorReplacesPreviousActor) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::ActorCommand actorA;
    actorA.name = "Actor A";
    const auto actorAId = service.addActor(actorA);
    ASSERT_FALSE(actorAId.empty());

    core::ports::workspace::ActorCommand actorB;
    actorB.name = "Actor B";
    const auto actorBId = service.addActor(actorB);
    ASSERT_FALSE(actorBId.empty());

    core::ports::workspace::PropertyCommand property;
    property.name = "Property A";
    const auto propertyId = service.addProperty(property);
    ASSERT_FALSE(propertyId.empty());

    core::ports::workspace::ContractCommand contract;
    contract.name = "Contract A";
    contract.type = "strom";
    contract.actorIds = {actorAId};
    contract.propertyIds = {propertyId};
    const auto contractId = service.addContract(contract);
    ASSERT_FALSE(contractId.empty());

    core::ports::workspace::ActorCommand updateB;
    updateB.id = actorBId;
    updateB.name = "Actor B";
    updateB.contractIds = {contractId};
    service.updateActor(updateB);

    const auto& contracts = session.catalogState().contracts();
    const auto contractIt = std::find_if(contracts.begin(), contracts.end(),
                                         [&](const auto& c) { return c && c->id() == contractId; });
    ASSERT_NE(contractIt, contracts.end());
    ASSERT_EQ((*contractIt)->actorIds().size(), 1u);
    EXPECT_EQ((*contractIt)->actorIds().front(), actorBId);

    const auto& actors = session.catalogState().actors();
    const auto actorAIt = std::find_if(actors.begin(), actors.end(),
                                       [&](const auto& a) { return a && a->id() == actorAId; });
    const auto actorBIt = std::find_if(actors.begin(), actors.end(),
                                       [&](const auto& a) { return a && a->id() == actorBId; });
    ASSERT_NE(actorAIt, actors.end());
    ASSERT_NE(actorBIt, actors.end());
    EXPECT_TRUE((*actorAIt)->contractIds().empty());
    ASSERT_EQ((*actorBIt)->contractIds().size(), 1u);
    EXPECT_EQ((*actorBIt)->contractIds().front(), contractId);
}

TEST(WorkspaceCommandServiceTest, ContractUpdateKeepsOnlyFirstActorId) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::ActorCommand actorA;
    actorA.name = "Actor A";
    const auto actorAId = service.addActor(actorA);
    ASSERT_FALSE(actorAId.empty());

    core::ports::workspace::ActorCommand actorB;
    actorB.name = "Actor B";
    const auto actorBId = service.addActor(actorB);
    ASSERT_FALSE(actorBId.empty());

    core::ports::workspace::PropertyCommand property;
    property.name = "Property A";
    const auto propertyId = service.addProperty(property);
    ASSERT_FALSE(propertyId.empty());

    core::ports::workspace::ContractCommand contract;
    contract.name = "Contract A";
    contract.type = "strom";
    contract.actorIds = {actorAId};
    contract.propertyIds = {propertyId};
    const auto contractId = service.addContract(contract);
    ASSERT_FALSE(contractId.empty());

    core::ports::workspace::ContractCommand updateContract;
    updateContract.id = contractId;
    updateContract.name = "Contract A";
    updateContract.type = "strom";
    updateContract.actorIds = {actorAId, actorBId};
    updateContract.propertyIds = {propertyId};
    service.updateContract(updateContract);

    const auto& contracts = session.catalogState().contracts();
    const auto contractIt = std::find_if(contracts.begin(), contracts.end(),
                                         [&](const auto& c) { return c && c->id() == contractId; });
    ASSERT_NE(contractIt, contracts.end());
    ASSERT_EQ((*contractIt)->actorIds().size(), 1u);
    EXPECT_EQ((*contractIt)->actorIds().front(), actorAId);
}

} // namespace core::application
