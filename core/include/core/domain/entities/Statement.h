/**
 * @file core/include/core/domain/entities/Statement.h
 * @brief Domain model for statements.
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <utility>

#include "core/domain/policies/StatementPolicy.h"
#include "core/domain/values/EntityName.h"

namespace core::domain {

class Statement {
public:
    Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&&) = delete;
    Statement& operator=(Statement&&) = delete;

    /**
     * @brief Renames the statement with normalized entity text.
     * @param value Normalized entity name wrapper.
     */
    void rename(EntityName value);
    /**
     * @brief Renames the statement with raw text.
     * @param value Raw entity name text.
     */
    void rename(std::string value);

    /**
     * @brief Adds a normalized transaction id when it is not already present.
     * @param value Transaction id to add.
     */
    void addTransaction(std::string value);

    /**
     * @brief Replaces the transaction id list with normalized values.
     * @param value Transaction id list to store.
     */
    void setTransactionIds(std::vector<std::string> value);

    /**
     * @brief Inserts a transaction id at a requested position.
     * @param value Transaction id to insert.
     * @param position Target insertion position.
     */
    void insertTransaction(std::string value, std::size_t position);

    /**
     * @brief Removes a transaction id from the statement.
     * @param value Transaction id to remove.
     */
    void removeTransaction(const std::string& value);

    /**
     * @brief Returns the index of a transaction id.
     * @param value Transaction id to search for.
     * @return Matching index or the list size when not found.
     */
    [[nodiscard]] std::size_t indexOfTransaction(const std::string& value) const;

    /**
     * @brief Checks whether a transaction exists at a given index.
     * @param index Index to inspect.
     * @return `true` when the index is within bounds.
     */
    [[nodiscard]] bool hasTransactionAt(std::size_t index) const noexcept;

    /**
     * @brief Moves a transaction id to a new position.
     * @param value Transaction id to move.
     * @param newPosition New insertion position.
     */
    void moveTransaction(const std::string& value, std::size_t newPosition);

    /**
     * @brief Checks whether a transaction id exists.
     * @param value Transaction id to search for.
     * @return `true` when the id exists.
     */
    [[nodiscard]] bool containsTransaction(const std::string& value) const;

    /**
     * @brief Clears all linked transactions.
     */
    void clearTransactions();
    /**
     * @brief Returns the number of linked transactions.
     * @return Transaction count.
     */
    [[nodiscard]] std::size_t transactionCount() const noexcept;
    /**
     * @brief Checks whether the statement is empty.
     * @return `true` when no transactions are linked.
     */
    [[nodiscard]] bool empty() const noexcept;

private:
    std::string id_;
    std::string name_;
    std::vector<std::string> transactionIds_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::vector<std::string>& transactionIds() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
};

} // namespace core::domain
