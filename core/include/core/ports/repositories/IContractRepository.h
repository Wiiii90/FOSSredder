/**
 * @file core/include/core/ports/repositories/IContractRepository.h
 * @brief Repository port for core::domain::Contract persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Contract;
}

namespace core::ports::repositories {

class IContractRepository {
public:
    /**
     * @brief Destroy the contract repository interface.
     */
    virtual ~IContractRepository() = default;

    /**
     * @brief Add a new contract.
     * @param contract Contract to add.
     */
    virtual void addContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Retrieve all contracts.
     * @return All stored contracts.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContracts() const = 0;

    /**
     * @brief Retrieve a contract by identifier.
     * @param id Contract identifier.
     * @return Contract with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Contract>> getContractById(const std::string& id) const = 0;

    /**
     * @brief Remove a contract by identifier.
     * @param id Contract identifier.
     */
    virtual void removeContract(const std::string& id) = 0;

    /**
     * @brief Update an existing contract.
     * @param contract Contract to update.
     */
    virtual void updateContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Insert or update a contract.
     * @param contract Contract to upsert.
     */
    virtual void upsertContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Remove all contracts.
     */
    virtual void clearContracts() = 0;

    /**
     * @brief Retrieve contracts associated with a given actor.
     * @param actorId Actor identifier.
     * @return Contracts linked to the given actor.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForActor(const std::string& actorId) const = 0;

    /**
     * @brief Retrieve contracts associated with a given property.
     * @param propertyId Property identifier.
     * @return Contracts linked to the given property.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForProperty(const std::string& propertyId) const = 0;

    /**
     * @brief Retrieve actor identifiers linked to a contract.
     * @param contractId Contract identifier.
     * @return Actor identifiers linked to the contract.
     */
    virtual std::vector<std::string> getActorIdsForContract(const std::string& contractId) const = 0;

    /**
     * @brief Retrieve property identifiers linked to a contract.
     * @param contractId Contract identifier.
     * @return Property identifiers linked to the contract.
     */
    virtual std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const = 0;
};

} // namespace core::ports::repositories
