/**
 * @file core/src/domain/policies/AliasPolicy.cpp
 * @brief Shared alias normalization and matching helpers.
 */

#include "core/domain/policies/AliasPolicy.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace core::domain::policies::alias {

std::string trimCopy(std::string value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c) != 0; }).base();
    if (begin >= end) return {};
    return std::string(begin, end);
}

std::string canonicalAliasValue(std::string value) { return trimCopy(std::move(value)); }

void normalizeAlias(Alias& alias) {
    const std::string now = core::utils::currentTimestampUtc();
    const std::string normalizedValue = canonicalAliasValue(alias.value());
    const std::string normalizedSource = canonicalAliasValue(alias.source());
    const std::string normalizedKind = trimCopy(alias.kind());
    alias.setValue(normalizedValue);
    alias.setKind(normalizedKind);
    alias.setSource(normalizedSource);
    if (alias.value().empty()) alias.setValue(alias.source());
    if (alias.source().empty()) alias.setSource(alias.value());
    if (alias.createdAt().empty()) alias.setCreatedAt(now);
    if (alias.updatedAt().empty()) alias.setUpdatedAt(now);
}

void normalizeAliases(std::vector<Alias>& aliases) {
    for (auto& alias : aliases) normalizeAlias(alias);
    std::vector<Alias> unique;
    unique.reserve(aliases.size());
    for (auto& alias : aliases) {
        const auto alreadyPresent = std::any_of(unique.begin(), unique.end(), [&](const Alias& existing) { return existing.value() == alias.value(); });
        if (!alreadyPresent && !alias.value().empty()) unique.push_back(std::move(alias));
    }
    aliases = std::move(unique);
}

bool containsAliasValue(const std::vector<Alias>& aliases, const std::string& value) {
    const auto needle = canonicalAliasValue(value);
    return std::any_of(aliases.begin(), aliases.end(), [&](const Alias& alias) { return alias.value() == needle; });
}

void removeAliasValue(std::vector<Alias>& aliases, const std::string& value) {
    const auto needle = canonicalAliasValue(value);
    aliases.erase(std::remove_if(aliases.begin(), aliases.end(), [&](const Alias& alias) { return alias.value() == needle; }), aliases.end());
}

void normalizeAliasUsage(Alias& alias) {
    normalizeAlias(alias);
    const std::string now = core::utils::currentTimestampUtc();
    if (alias.hitCount() < 1) alias.setHitCount(1);
    if (alias.lastUsedAt().empty()) alias.setLastUsedAt(now);
    if (alias.createdAt().empty()) alias.setCreatedAt(now);
    if (alias.updatedAt().empty()) alias.setUpdatedAt(now);
}

void recordAliasHit(std::vector<Alias>& aliases, const std::string& value, const std::string& timestamp) {
    const auto needle = canonicalAliasValue(value);
    if (needle.empty()) return;
    const std::string now = timestamp.empty() ? core::utils::currentTimestampUtc() : timestamp;
    for (auto& alias : aliases) {
        if (alias.value() != needle) continue;
        normalizeAliasUsage(alias);
        alias.setHitCount(alias.hitCount() + 1);
        alias.setLastUsedAt(now);
        alias.setUpdatedAt(now);
        return;
    }

    Alias alias;
    alias.setValue(needle);
    alias.setSource(needle);
    alias.setHitCount(1);
    alias.setLastUsedAt(now);
    alias.setCreatedAt(now);
    alias.setUpdatedAt(now);
    aliases.push_back(std::move(alias));
}

} // namespace core::domain::policies::alias
