/**
 * @file core/src/application/import/internal/ParserDateUtils.cpp
 * @brief Implements private helpers for short/full date detection in parser text.
 */

#include "core/application/import/internal/ParserDateUtils.h"

#include "core/errors/ErrorReporterRegistry.h"

#include <regex>

namespace {

const std::regex gShortDateRegex(R"(\b\d{2}\.\s*\d{2}\b)");
const std::regex gFullDateRegex(R"((\d{2}\.\d{2}\.\d{4}))");

}

namespace core::application::importing::internal {

bool hasShortDateToken(const std::string& text) noexcept
{
    try {
        return std::regex_search(text, gShortDateRegex);
    } catch (...) {
        return false;
    }
}

bool containsShortDate(const std::string& text) noexcept
{
    return hasShortDateToken(text);
}

std::optional<std::string> findFirstFullDate(const std::string& text) noexcept
{
    try {
        std::smatch match;
        if (std::regex_search(text, match, gFullDateRegex)) return match.str(1);
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findFirstFullDate", std::current_exception());
    }
    return std::nullopt;
}

}
