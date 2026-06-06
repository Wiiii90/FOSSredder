/**
 * @file core/include/core/ports/repositories/ITransactionRepository.h
 * @brief Repository port for core::domain::Transaction persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Transaction;
}

namespace core::ports::repositories {

class ITransactionRepository {
public:
    /**
     * @brief Destroy the transaction repository interface.
     */
    virtual ~ITransactionRepository() = default;

    /**
     * @brief Add a new transaction.
     * @param transaction Transaction to add.
     */
    virtual void addTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Retrieve all transactions.
     * @return All stored transactions.
     */
    virtual std::vector<std::shared_ptr<core::domain::Transaction>> getTransactions() const = 0;

    /**
     * @brief Retrieve a transaction by identifier.
     * @param id Transaction identifier.
     * @return Transaction with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Transaction>> getTransactionById(const std::string& id) const = 0;

    /**
     * @brief Remove a transaction by identifier.
     * @param id Transaction identifier.
     */
    virtual void removeTransaction(const std::string& id) = 0;

    /**
     * @brief Update an existing transaction.
     * @param transaction Transaction to update.
     */
    virtual void updateTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Insert or update a transaction.
     * @param transaction Transaction to upsert.
     */
    virtual void upsertTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Remove all transactions.
     */
    virtual void clearTransactions() = 0;

    /**
     * @brief Retrieve transactions associated with a given contract.
     * @param contractId Contract identifier.
     * @return Transactions linked to the given contract.
     */
    virtual std::vector<std::shared_ptr<core::domain::Transaction>> getTransactionsForContract(const std::string& contractId) const = 0;

    /**
     * @brief Assign transactions to a contract.
     * @param contractId Contract identifier.
     * @param transactionIds Transaction identifiers to assign.
     */
    virtual void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) = 0;
};

} // namespace core::ports::repositories
