/**
 * @file core/tests/domain/entities/TestTransaction.cpp
 * @brief Tests for transaction behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Transaction.h"

namespace core::domain {

TEST(TransactionTest, ConstructorStoresNormalizedCoreFields) {
    const Transaction transaction{"  rent booking  ", " 2024-01-31 ", "  EUR  ", 12.5, true};

    EXPECT_EQ(transaction.name(), "rent booking");
    EXPECT_EQ(transaction.bookingDate(), "2024-01-31");
    EXPECT_EQ(transaction.valuta(), "EUR");
    EXPECT_DOUBLE_EQ(transaction.amount(), 12.5);
    EXPECT_TRUE(transaction.isAllocatable());
    EXPECT_TRUE(transaction.isStandalone());
}

TEST(TransactionTest, MutatorsNormalizeRelationsAndTrackState) {
    Transaction transaction;

    transaction.setName("  rent  ");
    transaction.setBookingDate(" 2024-02-01 ");
    transaction.setValuta("  EUR ");
    transaction.setAmount(99.5);
    transaction.setContractId(" contract-1 ");
    transaction.setActorId(" actor-1 ");
    transaction.setStatementId(" statement-1 ");
    transaction.setPropertyIds({" property-a ", "property-a", "property-b"});
    transaction.setAllocatable(true);

    EXPECT_EQ(transaction.name(), "rent");
    EXPECT_EQ(transaction.bookingDate(), "2024-02-01");
    EXPECT_EQ(transaction.valuta(), "EUR");
    EXPECT_DOUBLE_EQ(transaction.amount(), 99.5);
    EXPECT_EQ(transaction.contractId(), "contract-1");
    EXPECT_EQ(transaction.actorId(), "actor-1");
    EXPECT_EQ(transaction.statementId(), "statement-1");
    EXPECT_EQ(transaction.propertyCount(), 2u);
    EXPECT_TRUE(transaction.hasRelations());
    EXPECT_TRUE(transaction.belongsToStatement(" statement-1 "));
    EXPECT_TRUE(transaction.hasStatus(Transaction::Status::Neutral));
}

TEST(TransactionTest, StatusAdvancesAndClearsRelations) {
    Transaction transaction;
    transaction.setStatus(Transaction::Status::Unverified);
    transaction.markVerified();
    transaction.markCompleted();

    EXPECT_TRUE(transaction.hasStatus(Transaction::Status::Completed));

    transaction.clearContract();
    transaction.clearActor();
    transaction.clearStatement();
    transaction.clearProperties();

    EXPECT_TRUE(transaction.isStandalone());
    EXPECT_FALSE(transaction.hasRelations());
    EXPECT_FALSE(transaction.hasContract());
    EXPECT_FALSE(transaction.hasActor());
    EXPECT_FALSE(transaction.hasStatement());
    EXPECT_FALSE(transaction.hasProperties());
}

} // namespace core::domain
