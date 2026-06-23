/**
 * @file core/tests/domain/entities/TestStatement.cpp
 * @brief Tests for statement behavior.
 */

#include <gtest/gtest.h>

#include "core/domain/entities/Statement.h"

namespace core::domain {

TEST(StatementTest, NormalizesAndKeepsTransactionOrder) {
    Statement statement;

    statement.addTransaction(" tx-1 ");
    statement.addTransaction("tx-1");
    statement.addTransaction("tx-2");

    EXPECT_EQ(statement.transactionCount(), 2u);
    EXPECT_EQ(statement.indexOfTransaction("tx-1"), 0u);
    EXPECT_EQ(statement.indexOfTransaction("tx-2"), 1u);
    EXPECT_TRUE(statement.containsTransaction(" tx-2 "));
}

TEST(StatementTest, InsertsMovesAndRemovesTransactionIds) {
    Statement statement;
    statement.setTransactionIds({"tx-1", "tx-2", "tx-3"});

    statement.insertTransaction("tx-0", 0);
    statement.moveTransaction("tx-3", 1);
    statement.removeTransaction("tx-2");

    ASSERT_EQ(statement.transactionCount(), 3u);
    EXPECT_EQ(statement.transactionIds().at(0), "tx-0");
    EXPECT_EQ(statement.transactionIds().at(1), "tx-3");
    EXPECT_EQ(statement.transactionIds().at(2), "tx-1");
    EXPECT_FALSE(statement.containsTransaction("tx-2"));
}

TEST(StatementTest, ClearsAndTracksEmptiness) {
    Statement statement;
    statement.addTransaction("tx-1");

    EXPECT_FALSE(statement.empty());

    statement.clearTransactions();

    EXPECT_TRUE(statement.empty());
    EXPECT_EQ(statement.transactionCount(), 0u);
}

} // namespace core::domain
