/**
 * @file persistence/include/persistence/repositories/SqliteAnnualRepository.h
 * @brief Declares the SQLite-backed annual repository.
 */

#pragma once

#include "core/ports/repositories/IAnnualRepository.h"
#include "persistence/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteAnnualRepository : public core::ports::repositories::IAnnualRepository, public ISqlConnectionProvider {
public:
    /**
     * @brief Create an annual repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteAnnualRepository(const std::string& dbPath);

    /**
     * @brief Create an annual repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteAnnualRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteAnnualRepository() override;

    /**
     * @brief Insert a new annual entry.
     * @param annual Annual to add.
     */
    void addAnnual(const std::shared_ptr<core::domain::Annual>& annual) override;

    /**
     * @brief Retrieve all annual entries.
     * @return All stored annual entries.
     */
    std::vector<std::shared_ptr<core::domain::Annual>> getAnnuals() const override;

    /**
     * @brief Retrieve an annual entry by identifier.
     * @param id Annual identifier.
     * @return Annual with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Annual>> getAnnualById(const std::string& id) const override;

    /**
     * @brief Remove an annual entry by identifier.
     * @param id Annual identifier.
     */
    void removeAnnual(const std::string& id) override;

    /**
     * @brief Update an existing annual entry.
     * @param annual Annual to update.
     */
    void updateAnnual(const std::shared_ptr<core::domain::Annual>& annual) override;

    /**
     * @brief Insert or update an annual entry.
     * @param annual Annual to upsert.
     */
    void upsertAnnual(const std::shared_ptr<core::domain::Annual>& annual) override;

    /**
     * @brief Remove all annual entries.
     */
    void clearAnnuals() override;

    /**
     * @brief Return the SQLite handle.
     * @return SQLite database handle.
     */
    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
