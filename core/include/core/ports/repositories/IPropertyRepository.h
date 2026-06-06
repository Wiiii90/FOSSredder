/**
 * @file core/include/core/ports/repositories/IPropertyRepository.h
 * @brief Repository port for core::domain::Property persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Property;
}

namespace core::ports::repositories {

class IPropertyRepository {
public:
    /**
     * @brief Destroy the property repository interface.
     */
    virtual ~IPropertyRepository() = default;

    /**
     * @brief Add a new property.
     * @param property Property to add.
     */
    virtual void addProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Retrieve all properties.
     * @return All stored properties.
     */
    virtual std::vector<std::shared_ptr<core::domain::Property>> getProperties() const = 0;

    /**
     * @brief Retrieve a property by identifier.
     * @param id Property identifier.
     * @return Property with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Property>> getPropertyById(const std::string& id) const = 0;

    /**
     * @brief Remove a property by identifier.
     * @param id Property identifier.
     */
    virtual void removeProperty(const std::string& id) = 0;

    /**
     * @brief Update an existing property.
     * @param property Property to update.
     */
    virtual void updateProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Insert or update a property.
     * @param property Property to upsert.
     */
    virtual void upsertProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Remove all properties.
     */
    virtual void clearProperties() = 0;
};

} // namespace core::ports::repositories
