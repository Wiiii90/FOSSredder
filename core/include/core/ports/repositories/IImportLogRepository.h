/**
 * @file core/include/core/ports/repositories/IImportLogRepository.h
 * @brief Repository port for core::application::importing::ImportLog persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::application::importing {
class ImportLog;
}

namespace core::ports::repositories {

class IImportLogRepository {
public:
    /**
     * @brief Destroy the import log repository interface.
     */
    virtual ~IImportLogRepository() = default;

    /**
     * @brief Add a new import log.
     * @param log Import log to add.
     */
    virtual void addImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) = 0;

    /**
     * @brief Retrieve all import logs.
     * @return All stored import logs.
     */
    virtual std::vector<std::shared_ptr<core::application::importing::ImportLog>> getImportLogs() const = 0;

    /**
     * @brief Retrieve an import log by identifier.
     * @param id Import log identifier.
     * @return Import log with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::application::importing::ImportLog>> getImportLogById(const std::string& id) const = 0;

    /**
     * @brief Remove an import log by identifier.
     * @param id Import log identifier.
     */
    virtual void removeImportLog(const std::string& id) = 0;

    /**
     * @brief Update an existing import log.
     * @param log Import log to update.
     */
    virtual void updateImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) = 0;

    /**
     * @brief Insert or update an import log.
     * @param log Import log to upsert.
     */
    virtual void upsertImportLog(const std::shared_ptr<core::application::importing::ImportLog>& log) = 0;

    /**
     * @brief Remove all import logs.
     */
    virtual void clearImportLogs() = 0;
};

} // namespace core::ports::repositories
