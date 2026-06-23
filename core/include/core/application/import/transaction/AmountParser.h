/**
 * @file core/include/core/application/import/transaction/AmountParser.h
 * @brief Declares helpers for parsing imported transaction amount strings.
 */

#pragma once

#include <optional>
#include <string>

namespace core::application::importing::transaction {

[[nodiscard]] std::optional<double> parseAmountString(const std::string& s);

} // namespace core::application::importing::transaction
