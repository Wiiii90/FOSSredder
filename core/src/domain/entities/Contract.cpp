/**
 * @file core/src/domain/entities/Contract.cpp
 * @brief Domain behavior for contracts.
 */

#include "core/domain/entities/Contract.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain {

Contract::Contract() = default;

void Contract::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Contract::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Contract::setType(ContractType value) {
    type_ = std::move(value.value());
}

void Contract::setType(std::string value) {
    setType(ContractType(std::move(value)));
}

void Contract::setAllocatableMode(std::string value) {
    auto normalized = policies::alias::trimCopy(std::move(value));
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (normalized == "all")
        normalized = "mixed";
    if (normalized != "mixed" && normalized != "allocatable" && normalized != "non-allocatable")
        normalized = "mixed";
    allocatableMode_ = std::move(normalized);
}

void Contract::setActorIds(std::vector<std::string> value) {
    normalizeIds(value);
    actorIds_ = std::move(value);
}

void Contract::addActorId(std::string value) {
    value = policies::alias::trimCopy(std::move(value));
    if (value.empty() || containsActorId(value)) {
        return;
    }
    actorIds_.push_back(std::move(value));
}

void Contract::removeActorId(const std::string& value) {
    const auto normalized = policies::alias::trimCopy(value);
    actorIds_.erase(std::remove(actorIds_.begin(), actorIds_.end(), normalized), actorIds_.end());
}

void Contract::clearActorIds() {
    actorIds_.clear();
}

void Contract::setPropertyIds(std::vector<std::string> value) {
    normalizeIds(value);
    propertyIds_ = std::move(value);
}

void Contract::addPropertyId(std::string value) {
    value = policies::alias::trimCopy(std::move(value));
    if (value.empty() || containsPropertyId(value)) {
        return;
    }
    propertyIds_.push_back(std::move(value));
}

void Contract::removePropertyId(const std::string& value) {
    const auto normalized = policies::alias::trimCopy(value);
    propertyIds_.erase(std::remove(propertyIds_.begin(), propertyIds_.end(), normalized), propertyIds_.end());
}

void Contract::clearPropertyIds() {
    propertyIds_.clear();
}

void Contract::setAliases(std::vector<Alias> value) {
    policies::alias::normalizeAliases(value);
    aliases_ = std::move(value);
}

void Contract::addAlias(Alias value) {
    policies::alias::normalizeAlias(value);
    if (value.value().empty() || policies::alias::containsAliasValue(aliases_, value.value())) {
        return;
    }
    aliases_.push_back(std::move(value));
}

void Contract::removeAlias(std::string value) {
    const auto normalized = policies::alias::canonicalAliasValue(value);
    policies::alias::removeAliasValue(aliases_, normalized);
}

void Contract::recordAliasHit(std::string value, std::string timestamp) {
    policies::alias::recordAliasHit(aliases_, std::move(value), std::move(timestamp));
}

bool Contract::hasName() const noexcept {
    return !name_.empty();
}

bool Contract::hasAlias(const std::string& value) const {
    return policies::alias::containsAliasValue(aliases_, value);
}

std::size_t Contract::aliasCount() const noexcept {
    return aliases_.size();
}

bool Contract::hasAliases() const noexcept {
    return !aliases_.empty();
}

void Contract::clearAliases() {
    aliases_.clear();
}

bool Contract::containsActorId(const std::string& value) const {
    const auto normalized = policies::alias::trimCopy(value);
    return std::find(actorIds_.begin(), actorIds_.end(), normalized) != actorIds_.end();
}

bool Contract::containsPropertyId(const std::string& value) const {
    const auto normalized = policies::alias::trimCopy(value);
    return std::find(propertyIds_.begin(), propertyIds_.end(), normalized) != propertyIds_.end();
}

bool Contract::hasSingleActor() const noexcept {
    return actorIds_.size() == 1u;
}

bool Contract::hasActorRelations() const noexcept {
    return !actorIds_.empty();
}

bool Contract::hasPropertyRelations() const noexcept {
    return !propertyIds_.empty();
}

bool Contract::hasRelations() const noexcept {
    return !actorIds_.empty() || !propertyIds_.empty();
}

std::size_t Contract::relationCount() const noexcept {
    return actorIds_.size() + propertyIds_.size();
}

std::size_t Contract::actorCount() const noexcept {
    return actorIds_.size();
}

std::size_t Contract::propertyCount() const noexcept {
    return propertyIds_.size();
}

bool Contract::hasType() const noexcept {
    return !type_.empty();
}

bool Contract::isConfigured() const noexcept {
    return hasName() && hasType();
}

bool Contract::isMatchingReady() const noexcept {
    return isConfigured() && hasAliases();
}

bool Contract::isStandalone() const noexcept {
    return !hasRelations();
}

const std::string& Contract::id() const noexcept { return id_; }
const std::string& Contract::name() const noexcept { return name_; }
const std::string& Contract::type() const noexcept { return type_; }
const std::string& Contract::allocatableMode() const noexcept { return allocatableMode_; }
const std::vector<std::string>& Contract::actorIds() const noexcept { return actorIds_; }
const std::vector<std::string>& Contract::propertyIds() const noexcept { return propertyIds_; }
const std::vector<Alias>& Contract::aliases() const noexcept { return aliases_; }
const std::string& Contract::createdAt() const noexcept { return createdAt_; }
const std::string& Contract::updatedAt() const noexcept { return updatedAt_; }
void Contract::setId(std::string value) { id_ = std::move(value); }
void Contract::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Contract::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
