/**
 * @file persistence/include/persistence/repositories/SqliteContractRepository.h
 * @brief Declares the SQLite-backed contract repository.
 */

#pragma once

#include "core/ports/repositories/IContractRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteContractRepository : public core::ports::repositories::IContractRepository {
public:
    /**
     * @brief Create a contract repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteContractRepository(const std::string& dbPath);

    /**
     * @brief Create a contract repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteContractRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteContractRepository() override;

    /**
     * @brief Insert a new contract.
     * @param contract Contract to add.
     */
    void addContract(const std::shared_ptr<core::domain::Contract>& contract) override;

    /**
     * @brief Retrieve all contracts.
     * @return All stored contracts.
     */
    std::vector<std::shared_ptr<core::domain::Contract>> getContracts() const override;

    /**
     * @brief Retrieve a contract by identifier.
     * @param id Contract identifier.
     * @return Contract with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Contract>> getContractById(const std::string& id) const override;

    /**
     * @brief Remove a contract by identifier.
     * @param id Contract identifier.
     */
    void removeContract(const std::string& id) override;

    /**
     * @brief Update an existing contract.
     * @param contract Contract to update.
     */
    void updateContract(const std::shared_ptr<core::domain::Contract>& contract) override;

    /**
     * @brief Insert or update a contract.
     * @param contract Contract to upsert.
     */
    void upsertContract(const std::shared_ptr<core::domain::Contract>& contract) override;

    /**
     * @brief Remove all contracts.
     */
    void clearContracts() override;

    /**
     * @brief Retrieve contracts associated with a given actor.
     * @param actorId Actor identifier.
     * @return Contracts linked to the actor.
     */
    std::vector<std::shared_ptr<core::domain::Contract>> getContractsForActor(const std::string& actorId) const override;
    /**
     * @brief Retrieve contracts associated with a given property.
     * @param propertyId Property identifier.
     * @return Contracts linked to the property.
     */
    std::vector<std::shared_ptr<core::domain::Contract>> getContractsForProperty(const std::string& propertyId) const override;
    /**
     * @brief Retrieve actor identifiers linked to a contract.
     * @param contractId Contract identifier.
     * @return Actor identifiers linked to the contract.
     */
    std::vector<std::string> getActorIdsForContract(const std::string& contractId) const override;
    /**
     * @brief Retrieve property identifiers linked to a contract.
     * @param contractId Contract identifier.
     * @return Property identifiers linked to the contract.
     */
    std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
