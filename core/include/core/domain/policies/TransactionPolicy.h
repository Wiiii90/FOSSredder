/**
 * @file core/include/core/domain/policies/TransactionPolicy.h
 * @brief Shared transaction transition and normalization helpers.
 */

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <utility>

#include "core/domain/values/BookingDate.h"
#include "core/domain/values/MoneyAmount.h"

namespace core::domain::policies::transaction {

/**
 * @brief Trims leading and trailing whitespace from text.
 * @param value Raw text.
 * @return Trimmed text or an empty string.
 */
std::string trimCopy(std::string value);

/**
 * @brief Checks whether a booking date value is valid.
 * @param value Raw booking date.
 * @return `true` when the booking date passes value-object validation.
 */
bool hasValidBookingDate(const std::string& value);

/**
 * @brief Checks whether a transaction amount value is valid.
 * @param value Raw amount.
 * @return `true` when the amount is finite.
 */
bool hasValidAmount(double value);

/**
 * @brief Checks whether a statement id is assigned.
 * @param statementId Raw statement id.
 * @return `true` when the normalized statement id is non-empty.
 */
bool hasStatementAssignment(const std::string& statementId);

/**
 * @brief Normalizes free-form transaction text.
 * @param value Raw transaction text.
 * @return Normalized text.
 */
std::string normalizeText(std::string value);

/**
 * @brief Normalizes a relation id.
 * @param value Raw id text.
 * @return Normalized id.
 */
std::string normalizeId(std::string value);

/**
 * @brief Normalizes and deduplicates relation ids.
 * @param values Relation id list to mutate.
 */
void normalizeIds(std::vector<std::string>& values);

/**
 * @brief Checks whether a draft transaction can be finalized.
 * @param bookingDate Raw booking date.
 * @param amount Raw amount.
 * @param statementId Raw statement id.
 * @return `true` when all finalization prerequisites are met.
 */
bool canFinalizeFromDraft(const std::string& bookingDate,
                         double amount,
                         const std::string& statementId);

/**
 * @brief Checks whether a transaction status may advance.
 * @param current Current numeric status.
 * @param next Requested numeric status.
 * @return `true` when the requested status is not a regression.
 */
bool statusCanAdvance(int current, int next);

/**
 * @brief Checks whether a transaction has any property ids.
 * @param propertyIds Property id list to inspect.
 * @return `true` when the list is not empty.
 */
bool hasPropertyIds(const std::vector<std::string>& propertyIds);

} // namespace core::domain::policies::transaction
