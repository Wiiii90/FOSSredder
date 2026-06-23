/**
 * @file core/include/core/ports/infra/storage/IRegistry.h
 * @brief Registry port used by storage adapters.
 */

#pragma once

#include <optional>
#include <string>

namespace core::ports::storage {

class IRegistry {
public:
    /**
     * @brief Destroy the registry port.
     */
    virtual ~IRegistry() = default;

    /**
     * @brief Retrieve the latest stored path.
     * @return Latest path, if present.
     */
    virtual std::optional<std::string> getLatest() const = 0;

    /**
     * @brief Store the latest path.
     * @param path Path to store.
     */
    virtual void setLatest(const std::string& path) = 0;
};

} // namespace core::ports::storage
