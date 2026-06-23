/**
 * @file core/include/core/application/import/internal/ParserHeuristics.h
 * @brief Declares heuristics used to classify statement parser lines.
 */

#pragma once

#include <string>

namespace core::application::importing::internal {

/**
 * @brief Detects whether a line looks like header noise.
 * @param line Input line to inspect.
 * @return True when the line should be treated as header noise.
 */
bool isHeaderNoiseLine(const std::string& line);
/**
 * @brief Detects whether a line marks a footer section.
 * @param line Input line to inspect.
 * @return True when the line is treated as a footer.
 */
bool isFooterLine(const std::string& line);
/**
 * @brief Detects whether a line belongs to the post-transaction footnote area.
 * @param line Input line to inspect.
 * @return True when the line should be treated as a post-transaction footnote.
 */
bool isPostTransactionFootnote(const std::string& line);
/**
 * @brief Detects whether a line marks the start of the transactions section.
 * @param line Input line to inspect.
 * @return True when the line is considered a transactions-section header.
 */
bool isTransactionsSectionHeader(const std::string& line);
/**
 * @brief Detects whether a line looks like a debit/credit table header.
 * @param line Input line to inspect.
 * @return True when the line resembles a debit/credit header row.
 */
bool isDebitCreditHeaderLine(const std::string& line);

}

namespace core {
namespace parser {
namespace heuristics = application::importing::internal;
}
}
