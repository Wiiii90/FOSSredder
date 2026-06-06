/**
 * @file persistence/include/persistence/repositories/SqliteExportLogRepository.h
 * @brief Declares the SQLite-backed export log repository.
 */

#pragma once

#include "core/ports/repositories/IExportLogRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteExportLogRepository : public core::ports::repositories::IExportLogRepository {
public:
    /**
     * @brief Create an export log repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteExportLogRepository(const std::string& dbPath);

    /**
     * @brief Create an export log repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteExportLogRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteExportLogRepository() override;

    /**
     * @brief Insert a new export log entry.
     * @param log Export log to add.
     */
    void addExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) override;

    /**
     * @brief Retrieve all export log entries.
     * @return All stored export logs.
     */
    std::vector<std::shared_ptr<core::application::exporting::ExportLog>> getExportLogs() const override;

    /**
     * @brief Retrieve an export log entry by identifier.
     * @param id Export log identifier.
     * @return Export log with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::application::exporting::ExportLog>> getExportLogById(const std::string& id) const override;

    /**
     * @brief Remove an export log entry by identifier.
     * @param id Export log identifier.
     */
    void removeExportLog(const std::string& id) override;

    /**
     * @brief Update an existing export log entry.
     * @param log Export log to update.
     */
    void updateExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) override;

    /**
     * @brief Insert or update an export log entry.
     * @param log Export log to upsert.
     */
    void upsertExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) override;

    /**
     * @brief Remove all export log entries.
     */
    void clearExportLogs() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
