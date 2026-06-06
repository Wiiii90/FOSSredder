/**
 * @file persistence/include/persistence/repositories/SqliteImportLogRepository.h
 * @brief Declares the SQLite-backed import log repository.
 */

#pragma once

#include "core/ports/repositories/IImportLogRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteImportLogRepository : public core::ports::repositories::IImportLogRepository {
public:
    /**
     * @brief Create an import log repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteImportLogRepository(const std::string& dbPath);

    /**
     * @brief Create an import log repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteImportLogRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteImportLogRepository() override;

    /**
     * @brief Insert a new import log entry.
     * @param log Import log to add.
     */
    void addImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) override;

    /**
     * @brief Retrieve all import log entries.
     * @return All stored import logs.
     */
    std::vector<std::shared_ptr<core::application::importing::ImportLog>> getImportLogs() const override;

    /**
     * @brief Retrieve an import log entry by identifier.
     * @param id Import log identifier.
     * @return Import log with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::application::importing::ImportLog>> getImportLogById(const std::string& id) const override;

    /**
     * @brief Remove an import log entry by identifier.
     * @param id Import log identifier.
     */
    void removeImportLog(const std::string& id) override;

    /**
     * @brief Update an existing import log entry.
     * @param log Import log to update.
     */
    void updateImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) override;

    /**
     * @brief Insert or update an import log entry.
     * @param log Import log to upsert.
     */
    void upsertImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) override;

    /**
     * @brief Remove all import log entries.
     */
    void clearImportLogs() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
