/**
 * @file persistence/include/persistence/repositories/SqlitePropertyRepository.h
 * @brief Declares the SQLite-backed property repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/ports/repositories/IPropertyRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqlitePropertyRepository : public core::ports::repositories::IPropertyRepository {
public:
    /**
     * @brief Create a property repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqlitePropertyRepository(const std::string& dbPath);

    /**
     * @brief Create a property repository for the database at the given path.
     * @param dbPath SQLite database path.
     * @param errorReporter Error reporter used by the repository.
     */
    SqlitePropertyRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Create a property repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqlitePropertyRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Create a property repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     * @param errorReporter Error reporter used by the repository.
     */
    SqlitePropertyRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Destroy the repository.
     */
    ~SqlitePropertyRepository() override;

    /**
     * @brief Insert a new property.
     * @param property Property to add.
     */
    void addProperty(const std::shared_ptr<core::domain::Property>& property) override;

    /**
     * @brief Retrieve all properties.
     * @return All stored properties.
     */
    std::vector<std::shared_ptr<core::domain::Property>> getProperties() const override;

    /**
     * @brief Retrieve a property by identifier.
     * @param id Property identifier.
     * @return Property with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Property>> getPropertyById(const std::string& id) const override;

    /**
     * @brief Remove a property by identifier.
     * @param id Property identifier.
     */
    void removeProperty(const std::string& id) override;

    /**
     * @brief Update an existing property.
     * @param property Property to update.
     */
    void updateProperty(const std::shared_ptr<core::domain::Property>& property) override;

    /**
     * @brief Insert or update a property.
     * @param property Property to upsert.
     */
    void upsertProperty(const std::shared_ptr<core::domain::Property>& property) override;

    /**
     * @brief Remove all properties.
     */
    void clearProperties() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
