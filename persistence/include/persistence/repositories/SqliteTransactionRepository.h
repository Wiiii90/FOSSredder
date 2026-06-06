/**
 * @file persistence/include/persistence/repositories/SqliteTransactionRepository.h
 * @brief Declares the SQLite-backed transaction repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/ports/repositories/ITransactionRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteTransactionRepository : public core::ports::repositories::ITransactionRepository {
public:
    /**
     * @brief Create a transaction repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteTransactionRepository(const std::string& dbPath);

    /**
     * @brief Create a transaction repository for the database at the given path.
     * @param dbPath SQLite database path.
     * @param errorReporter Error reporter used by the repository.
     */
    SqliteTransactionRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Create a transaction repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteTransactionRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Create a transaction repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     * @param errorReporter Error reporter used by the repository.
     */
    SqliteTransactionRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteTransactionRepository() override;

    /**
     * @brief Insert a new transaction.
     * @param transaction Transaction to add.
     */
    void addTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;

    /**
     * @brief Retrieve all transactions.
     * @return All stored transactions.
     */
    std::vector<std::shared_ptr<core::domain::Transaction>> getTransactions() const override;

    /**
     * @brief Retrieve a transaction by identifier.
     * @param id Transaction identifier.
     * @return Transaction with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Transaction>> getTransactionById(const std::string& id) const override;

    /**
     * @brief Remove a transaction by identifier.
     * @param id Transaction identifier.
     */
    void removeTransaction(const std::string& id) override;

    /**
     * @brief Update an existing transaction.
     * @param transaction Transaction to update.
     */
    void updateTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;

    /**
     * @brief Insert or update a transaction.
     * @param transaction Transaction to upsert.
     */
    void upsertTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;

    /**
     * @brief Remove all transactions.
     */
    void clearTransactions() override;

    /**
     * @brief Retrieve transactions associated with a given contract.
     * @param contractId Contract identifier.
     * @return Transactions linked to the contract.
     */
    std::vector<std::shared_ptr<core::domain::Transaction>> getTransactionsForContract(const std::string& contractId) const override;

    /**
     * @brief Assign transactions to a contract.
     * @param contractId Contract identifier.
     * @param transactionIds Transaction identifiers to assign.
     */
    void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
