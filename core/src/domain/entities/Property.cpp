/**
 * @file core/src/domain/entities/Property.cpp
 * @brief Domain behavior for properties.
 */

#include "core/domain/entities/Property.h"

#include <algorithm>
#include <utility>

namespace core::domain {

Property::Property() = default;

void Property::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Property::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Property::setAliases(std::vector<Alias> value) {
    policies::alias::normalizeAliases(value);
    aliases_ = std::move(value);
}

void Property::setContractIds(std::vector<std::string> value) {
    std::sort(value.begin(), value.end());
    value.erase(std::unique(value.begin(), value.end()), value.end());
    contractIds_ = std::move(value);
}

void Property::addAlias(Alias value) {
    policies::alias::normalizeAlias(value);
    if (value.value().empty() || policies::alias::containsAliasValue(aliases_, value.value())) {
        return;
    }
    aliases_.push_back(std::move(value));
}

void Property::removeAlias(std::string value) {
    const auto normalized = policies::alias::canonicalAliasValue(value);
    policies::alias::removeAliasValue(aliases_, normalized);
}

void Property::clearAliases() {
    aliases_.clear();
}

void Property::recordAliasHit(std::string value, std::string timestamp) {
    policies::alias::recordAliasHit(aliases_, std::move(value), std::move(timestamp));
}

bool Property::hasAlias(const std::string& value) const {
    return policies::alias::containsAliasValue(aliases_, value);
}

std::size_t Property::aliasCount() const noexcept {
    return aliases_.size();
}

void Property::addContractId(std::string value) {
    value = policies::alias::trimCopy(std::move(value));
    if (value.empty() || std::find(contractIds_.begin(), contractIds_.end(), value) != contractIds_.end()) {
        return;
    }
    contractIds_.push_back(std::move(value));
}

void Property::removeContractId(std::string value) {
    const auto normalized = policies::alias::trimCopy(value);
    contractIds_.erase(std::remove(contractIds_.begin(), contractIds_.end(), normalized), contractIds_.end());
}

void Property::clearContractIds() {
    contractIds_.clear();
}

bool Property::hasContractRelations() const noexcept {
    return !contractIds_.empty();
}

std::size_t Property::contractCount() const noexcept {
    return contractIds_.size();
}

const std::string& Property::id() const noexcept { return id_; }
const std::string& Property::name() const noexcept { return name_; }
const std::vector<Alias>& Property::aliases() const noexcept { return aliases_; }
const std::vector<std::string>& Property::contractIds() const noexcept { return contractIds_; }
const std::string& Property::createdAt() const noexcept { return createdAt_; }
const std::string& Property::updatedAt() const noexcept { return updatedAt_; }
void Property::setId(std::string value) { id_ = std::move(value); }
void Property::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Property::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
