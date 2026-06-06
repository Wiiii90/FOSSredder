/**
 * @file core/include/core/domain/values/Alias.h
 * @brief Domain model for entity aliases.
 */

#pragma once

#include <string>
#include <utility>

namespace core::domain {

class Alias {
public:
    Alias() = default;
    /**
     * @brief Creates an alias record from raw values.
     * @param value Alias text.
     * @param kind Alias kind.
     * @param source Alias source.
     * @param createdAt Creation timestamp.
     * @param updatedAt Update timestamp.
     * @param hitCount Alias usage hit count.
     * @param lastUsedAt Last usage timestamp.
     */
    Alias(std::string value,
          std::string kind = {},
          std::string source = {},
          std::string createdAt = {},
          std::string updatedAt = {},
          int hitCount = 0,
          std::string lastUsedAt = {});

    [[nodiscard]] const std::string& value() const noexcept;
    [[nodiscard]] const std::string& kind() const noexcept;
    [[nodiscard]] const std::string& source() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;
    [[nodiscard]] int hitCount() const noexcept;
    [[nodiscard]] const std::string& lastUsedAt() const noexcept;
    void setValue(std::string value);
    void setKind(std::string value);
    void setSource(std::string value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
    void setHitCount(int value);
    void setLastUsedAt(std::string value);

private:
    std::string value_;
    std::string kind_;
    std::string source_;
    std::string createdAt_;
    std::string updatedAt_;
    int hitCount_ = 0;
    std::string lastUsedAt_;
};

} // namespace core::domain
