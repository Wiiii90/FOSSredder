/**
 * @file core/include/core/ports/repositories/IActorRepository.h
 * @brief Repository port for core::domain::Actor persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Actor;
}

namespace core::ports::repositories {

class IActorRepository {
public:
    /**
     * @brief Destroy the actor repository interface.
     */
    virtual ~IActorRepository() = default;

    /**
     * @brief Add a new actor.
     * @param actor Actor to add.
     */
    virtual void addActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Retrieve all actors.
     * @return All stored actors.
     */
    virtual std::vector<std::shared_ptr<core::domain::Actor>> getActors() const = 0;

    /**
     * @brief Retrieve an actor by identifier.
     * @param id Actor identifier.
     * @return Actor with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Actor>> getActorById(const std::string& id) const = 0;

    /**
     * @brief Remove an actor by identifier.
     * @param id Actor identifier.
     */
    virtual void removeActor(const std::string& id) = 0;

    /**
     * @brief Update an existing actor.
     * @param actor Actor to update.
     */
    virtual void updateActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Insert or update an actor.
     * @param actor Actor to upsert.
     */
    virtual void upsertActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Remove all actors.
     */
    virtual void clearActors() = 0;
};

} // namespace core::ports::repositories
