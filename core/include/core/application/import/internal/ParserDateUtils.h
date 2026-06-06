/**
 * @file core/include/core/application/import/internal/ParserDateUtils.h
 * @brief Declares date-detection helpers for the import parser.
 */

#pragma once

#include <optional>
#include <string>

namespace core::application::importing::internal {

/**
 * @brief Checks whether the text contains a short date token.
 * @param text Input text to inspect.
 * @return True when a short date token is present.
 */
bool hasShortDateToken(const std::string& text) noexcept;
/**
 * @brief Checks whether the text contains any short date pattern.
 * @param text Input text to inspect.
 * @return True when a short date is present.
 */
bool containsShortDate(const std::string& text) noexcept;
/**
 * @brief Finds the first full date in a text fragment.
 * @param text Input text to inspect.
 * @return First detected full date, or an empty optional when none is present.
 */
std::optional<std::string> findFirstFullDate(const std::string& text) noexcept;

}

namespace core {
namespace parser {
namespace helpers = application::importing::internal;
}
}
