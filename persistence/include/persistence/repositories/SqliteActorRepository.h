/**
 * @file persistence/include/persistence/repositories/SqliteActorRepository.h
 * @brief Declares the SQLite-backed actor repository.
 */

#pragma once

#include "core/ports/repositories/IActorRepository.h"
#include "persistence/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteActorRepository : public core::ports::repositories::IActorRepository, public ISqlConnectionProvider {
public:
    /**
     * @brief Create an actor repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteActorRepository(const std::string& dbPath);

    /**
     * @brief Create an actor repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteActorRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteActorRepository() override;

    /**
     * @brief Insert a new actor.
     * @param actor Actor to add.
     */
    void addActor(const std::shared_ptr<core::domain::Actor>& actor) override;

    /**
     * @brief Retrieve all actors.
     * @return All stored actors.
     */
    std::vector<std::shared_ptr<core::domain::Actor>> getActors() const override;

    /**
     * @brief Retrieve an actor by identifier.
     * @param id Actor identifier.
     * @return Actor with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Actor>> getActorById(const std::string& id) const override;

    /**
     * @brief Remove an actor by identifier.
     * @param id Actor identifier.
     */
    void removeActor(const std::string& id) override;

    /**
     * @brief Update an existing actor.
     * @param actor Actor to update.
     */
    void updateActor(const std::shared_ptr<core::domain::Actor>& actor) override;

    /**
     * @brief Insert or update an actor.
     * @param actor Actor to upsert.
     */
    void upsertActor(const std::shared_ptr<core::domain::Actor>& actor) override;

    /**
     * @brief Remove all actors.
     */
    void clearActors() override;

    /**
     * @brief Return the SQLite handle.
     * @return SQLite database handle.
     */
    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
