/**
 * @file core/include/core/domain/entities/Contract.h
 * @brief Domain model for contracts.
 */

#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include "core/domain/policies/AliasPolicy.h"
#include "core/domain/values/ContractType.h"
#include "core/domain/values/EntityName.h"
#include "core/domain/values/Alias.h"

namespace core::domain {

class Contract {
public:
    Contract();

    Contract(const Contract&) = delete;
    Contract& operator=(const Contract&) = delete;
    Contract(Contract&&) = delete;
    Contract& operator=(Contract&&) = delete;

    /**
     * @brief Renames the contract with normalized entity text.
     * @param value Normalized entity name wrapper.
     */
    void rename(EntityName value);
    /**
     * @brief Renames the contract with raw text.
     * @param value Raw entity name text.
     */
    void rename(std::string value);

    /**
     * @brief Sets the contract type from a normalized value object.
     * @param value Normalized contract type wrapper.
     */
    void setType(ContractType value);
    /**
     * @brief Sets the contract type from raw text.
     * @param value Raw contract type text.
     */
    void setType(std::string value);
    void setAllocatableMode(std::string value);

    /**
     * @brief Replaces the linked actor ids with normalized values.
     * @param value Actor id list to store.
     */
    void setActorIds(std::vector<std::string> value);

    /**
     * @brief Adds an actor id when it is not already present.
     * @param value Actor id to add.
     */
    void addActorId(std::string value);

    /**
     * @brief Removes an actor id from the contract.
     * @param value Actor id to remove.
     */
    void removeActorId(const std::string& value);

    /**
     * @brief Clears all linked actor ids.
     */
    void clearActorIds();

    /**
     * @brief Replaces the linked property ids with normalized values.
     * @param value Property id list to store.
     */
    void setPropertyIds(std::vector<std::string> value);

    /**
     * @brief Adds a property id when it is not already present.
     * @param value Property id to add.
     */
    void addPropertyId(std::string value);

    /**
     * @brief Removes a property id from the contract.
     * @param value Property id to remove.
     */
    void removePropertyId(const std::string& value);

    /**
     * @brief Clears all linked property ids.
     */
    void clearPropertyIds();

    /**
     * @brief Adds a normalized alias when it is not already present.
     * @param value Alias to add.
     */
    void addAlias(Alias value);

    /**
     * @brief Removes an alias.
     * @param value Alias to remove.
     */
    void removeAlias(std::string value);

    /**
     * @brief Records a usage hit for an alias.
     * @param value Alias value.
     * @param timestamp Optional usage timestamp.
     */
    void recordAliasHit(std::string value, std::string timestamp = {});

    /**
     * @brief Checks whether the contract has a name.
     * @return `true` when the name is set.
     */
    [[nodiscard]] bool hasName() const noexcept;

    /**
     * @brief Checks whether an alias exists on the contract.
     * @param value Alias text to search for.
     * @return `true` when the alias exists.
     */
    [[nodiscard]] bool hasAlias(const std::string& value) const;

    /**
     * @brief Returns the number of aliases.
     * @return Alias count.
     */
    [[nodiscard]] std::size_t aliasCount() const noexcept;

    /**
     * @brief Checks whether the contract has any aliases.
     * @return `true` when at least one alias exists.
     */
    [[nodiscard]] bool hasAliases() const noexcept;

    /**
     * @brief Clears all aliases from the contract.
     */
    void clearAliases();

    /**
     * @brief Checks whether an actor id exists.
     * @param value Actor id to search for.
     * @return `true` when the actor id exists.
     */
    [[nodiscard]] bool containsActorId(const std::string& value) const;

    /**
     * @brief Checks whether a property id exists.
     * @param value Property id to search for.
     * @return `true` when the property id exists.
     */
    [[nodiscard]] bool containsPropertyId(const std::string& value) const;

    /**
     * @brief Checks whether the contract has exactly one actor.
     * @return `true` when exactly one actor id is linked.
     */
    [[nodiscard]] bool hasSingleActor() const noexcept;

    /**
     * @brief Checks whether any actor relations exist.
     * @return `true` when at least one actor id is linked.
     */
    [[nodiscard]] bool hasActorRelations() const noexcept;

    /**
     * @brief Checks whether any property relations exist.
     * @return `true` when at least one property id is linked.
     */
    [[nodiscard]] bool hasPropertyRelations() const noexcept;

    /**
     * @brief Checks whether the contract has any relations.
     * @return `true` when actor or property relations exist.
     */
    [[nodiscard]] bool hasRelations() const noexcept;

    /**
     * @brief Returns the total number of linked relations.
     * @return Number of actor and property relations.
     */
    [[nodiscard]] std::size_t relationCount() const noexcept;

    /**
     * @brief Returns the number of linked actors.
     * @return Actor relation count.
     */
    [[nodiscard]] std::size_t actorCount() const noexcept;

    /**
     * @brief Returns the number of linked properties.
     * @return Property relation count.
     */
    [[nodiscard]] std::size_t propertyCount() const noexcept;

    /**
     * @brief Checks whether the contract has a type.
     * @return `true` when the type is set.
     */
    [[nodiscard]] bool hasType() const noexcept;

    /**
     * @brief Checks whether the contract is fully configured.
     * @return `true` when both name and type are set.
     */
    [[nodiscard]] bool isConfigured() const noexcept;

    /**
     * @brief Checks whether the contract is ready for matching.
     * @return `true` when the contract is configured and has aliases.
     */
    [[nodiscard]] bool isMatchingReady() const noexcept;

    /**
     * @brief Checks whether the contract has no actor or property links.
     * @return `true` when the contract is standalone.
     */
    [[nodiscard]] bool isStandalone() const noexcept;

private:
    static void normalizeIds(std::vector<std::string>& values) {
        std::vector<std::string> unique;
        unique.reserve(values.size());
        for (auto& value : values) {
            value = policies::alias::trimCopy(std::move(value));
            if (value.empty()) continue;
            if (std::find(unique.begin(), unique.end(), value) != unique.end()) continue;
            unique.push_back(value);
        }
        values = std::move(unique);
    }

    std::string id_;
    std::string name_;
    std::string type_;
    std::string allocatableMode_ = "mixed";
    std::vector<std::string> actorIds_;
    std::vector<std::string> propertyIds_;
    std::vector<Alias> aliases_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& type() const noexcept;
    [[nodiscard]] const std::string& allocatableMode() const noexcept;
    [[nodiscard]] const std::vector<std::string>& actorIds() const noexcept;
    [[nodiscard]] const std::vector<std::string>& propertyIds() const noexcept;
    [[nodiscard]] const std::vector<Alias>& aliases() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setAliases(std::vector<Alias> value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
};

} // namespace core::domain
