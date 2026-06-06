/**
 * @file core/include/core/domain/policies/AliasPolicy.h
 * @brief Shared alias normalization and ranking helpers for core entities.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "core/domain/values/Alias.h"
#include "core/utils/Time.h"

namespace core::domain::policies::alias {

/**
 * @brief Trims leading and trailing whitespace from text.
 * @param value Raw text.
 * @return Trimmed text or an empty string.
 */
std::string trimCopy(std::string value);

/**
 * @brief Canonicalizes an alias string for comparison.
 * @param value Raw alias text.
 * @return Canonical alias text.
 */
std::string canonicalAliasValue(std::string value);

/**
 * @brief Normalizes a single alias in place.
 * @param alias Alias instance to normalize.
 */
void normalizeAlias(Alias& alias);

/**
 * @brief Normalizes and deduplicates a list of aliases.
 * @param aliases Alias list to normalize in place.
 */
void normalizeAliases(std::vector<Alias>& aliases);

/**
 * @brief Checks whether an alias list already contains a value.
 * @param aliases Alias list to inspect.
 * @param value Alias text to search for.
 * @return `true` when a matching alias exists.
 */
bool containsAliasValue(const std::vector<Alias>& aliases, const std::string& value);

/**
 * @brief Removes aliases with a given value.
 * @param aliases Alias list to mutate.
 * @param value Alias text to remove.
 */
void removeAliasValue(std::vector<Alias>& aliases, const std::string& value);

/**
 * @brief Normalizes alias usage metadata in place.
 * @param alias Alias to normalize.
 */
void normalizeAliasUsage(Alias& alias);

/**
 * @brief Records or updates a hit for an alias value.
 * @param aliases Alias collection to mutate.
 * @param value Alias text that was hit.
 * @param timestamp Optional timestamp override.
 */
void recordAliasHit(std::vector<Alias>& aliases, const std::string& value, const std::string& timestamp = {});

} // namespace core::domain::policies::alias
