/**
 * @file core/src/domain/values/Alias.cpp
 * @brief Domain value semantics for aliases.
 */

#include "core/domain/values/Alias.h"

#include <utility>

namespace core::domain {

Alias::Alias(std::string value,
             std::string kind,
             std::string source,
             std::string createdAt,
             std::string updatedAt,
             int hitCount,
             std::string lastUsedAt)
    : value_(std::move(value)),
      kind_(std::move(kind)),
      source_(std::move(source)),
      createdAt_(std::move(createdAt)),
      updatedAt_(std::move(updatedAt)),
      hitCount_(hitCount),
      lastUsedAt_(std::move(lastUsedAt)) {}

const std::string& Alias::value() const noexcept { return value_; }
void Alias::setValue(std::string value) { value_ = std::move(value); }
void Alias::setKind(std::string value) { kind_ = std::move(value); }
const std::string& Alias::kind() const noexcept { return kind_; }
void Alias::setSource(std::string value) { source_ = std::move(value); }
const std::string& Alias::source() const noexcept { return source_; }
void Alias::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
const std::string& Alias::createdAt() const noexcept { return createdAt_; }
void Alias::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }
const std::string& Alias::updatedAt() const noexcept { return updatedAt_; }
int Alias::hitCount() const noexcept { return hitCount_; }
const std::string& Alias::lastUsedAt() const noexcept { return lastUsedAt_; }
void Alias::setHitCount(int value) { hitCount_ = value; }
void Alias::setLastUsedAt(std::string value) { lastUsedAt_ = std::move(value); }

} // namespace core::domain
