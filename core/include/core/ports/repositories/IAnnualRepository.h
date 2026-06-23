/**
 * @file core/include/core/ports/repositories/IAnnualRepository.h
 * @brief Repository port for core::domain::Annual persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Annual;
}

namespace core::ports::repositories {

class IAnnualRepository {
public:
    /**
     * @brief Destroy the annual repository interface.
     */
    virtual ~IAnnualRepository() = default;

    /**
     * @brief Add a new annual entry.
     * @param annual Annual to add.
     */
    virtual void addAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Retrieve all annual entries.
     * @return All stored annual entries.
     */
    virtual std::vector<std::shared_ptr<core::domain::Annual>> getAnnuals() const = 0;

    /**
     * @brief Retrieve an annual entry by identifier.
     * @param id Annual identifier.
     * @return Annual with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Annual>> getAnnualById(const std::string& id) const = 0;

    /**
     * @brief Remove an annual entry by identifier.
     * @param id Annual identifier.
     */
    virtual void removeAnnual(const std::string& id) = 0;

    /**
     * @brief Update an existing annual entry.
     * @param annual Annual to update.
     */
    virtual void updateAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Insert or update an annual entry.
     * @param annual Annual to upsert.
     */
    virtual void upsertAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Remove all annual entries.
     */
    virtual void clearAnnuals() = 0;
};

} // namespace core::ports::repositories
