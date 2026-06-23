/**
 * @file core/include/core/ports/repositories/IExportLogRepository.h
 * @brief Repository port for core::application::exporting::ExportLog persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::application::exporting {
class ExportLog;
}

namespace core::ports::repositories {

class IExportLogRepository {
public:
    /**
     * @brief Destroy the export log repository interface.
     */
    virtual ~IExportLogRepository() = default;

    /**
     * @brief Add a new export log.
     * @param log Export log to add.
     */
    virtual void addExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) = 0;

    /**
     * @brief Retrieve all export logs.
     * @return All stored export logs.
     */
    virtual std::vector<std::shared_ptr<core::application::exporting::ExportLog>> getExportLogs() const = 0;

    /**
     * @brief Retrieve an export log by identifier.
     * @param id Export log identifier.
     * @return Export log with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::application::exporting::ExportLog>> getExportLogById(const std::string& id) const = 0;

    /**
     * @brief Remove an export log by identifier.
     * @param id Export log identifier.
     */
    virtual void removeExportLog(const std::string& id) = 0;

    /**
     * @brief Update an existing export log.
     * @param log Export log to update.
     */
    virtual void updateExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) = 0;

    /**
     * @brief Insert or update an export log.
     * @param log Export log to upsert.
     */
    virtual void upsertExportLog(const std::shared_ptr<core::application::exporting::ExportLog>& log) = 0;

    /**
     * @brief Remove all export logs.
     */
    virtual void clearExportLogs() = 0;
};

} // namespace core::ports::repositories
