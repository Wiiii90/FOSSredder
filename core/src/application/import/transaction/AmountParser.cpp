/**
 * @file core/src/application/import/transaction/AmountParser.cpp
 * @brief Implements private helpers for parsing OCR amount strings.
 */

#include "core/application/import/transaction/AmountParser.h"

#include "core/application/import/internal/ParserDateUtils.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>

namespace {

std::optional<double> parseAmountInternal(const std::string& line)
{
    std::string s = line;
    try {
        auto replaceAll = [&](const std::string& from, const std::string& to) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos) {
                s.replace(pos, from.size(), to);
                pos += to.size();
            }
        };
        replaceAll("\xE2\x80\x93", "-");
        replaceAll("\xE2\x80\x94", "-");
        replaceAll("\xE2\x88\x92", "-");
        replaceAll("\xC2\xA0", " ");
        replaceAll("\xE2\x80\xAF", " ");
        replaceAll("\xE2\x80\x89", " ");
        replaceAll("\xE2\x80\xA2", ".");
        replaceAll("\xE2\x80\xB7", ".");
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::normalizeChars", std::current_exception());
    }

    std::string clean;
    clean.reserve(s.size());
    for (unsigned char c : s) {
        if ((c >= '0' && c <= '9') || c == '.' || c == ',' || c == '-' || c == '(' || c == ')' || std::isspace(c)) clean.push_back(static_cast<char>(c));
        else clean.push_back(' ');
    }
    s = std::move(clean);

    try {
        std::string direct = s;
        direct.erase(std::remove_if(direct.begin(), direct.end(), [](unsigned char c) {
            return std::isspace(c);
        }), direct.end());
        if (!direct.empty()) {
            size_t parsedChars = 0;
            const double value = std::stod(direct, &parsedChars);
            if (parsedChars == direct.size() && std::isfinite(value)) {
                return value;
            }
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::direct", std::current_exception());
    }

    try {
        static const std::regex splitDec(R"((\d{1,3}[\.,]\d)\s+(\d{1,2}-?))");
        std::smatch m;
        std::string tmp = s;
        while (std::regex_search(tmp, m, splitDec)) {
            const std::string repl = m.str(1) + m.str(2);
            tmp = m.prefix().str() + repl + m.suffix().str();
        }
        s = std::move(tmp);
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::repairSplit", std::current_exception());
    }

    static const std::regex re(R"((\(?-?(?:\d{1,3}(?:[\.,]\d{3})*|\d+),\d{2}\)?))");
    try {
        std::optional<double> best;
        double bestAbs = 0.0;
        for (std::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it) {
            try {
                std::string token = (*it).str(1);
                if (core::application::importing::internal::hasShortDateToken(token)) continue;

                bool negative = false;
                if (!token.empty() && token.front() == '(' && token.back() == ')') { negative = true; token = token.substr(1, token.size() - 2); }
                if (!token.empty() && token.front() == '-') { negative = true; token = token.substr(1); }
                if (!token.empty() && token.back() == '-') { negative = true; token.pop_back(); }

                std::string normalized = token;
                normalized.erase(std::remove(normalized.begin(), normalized.end(), '.'), normalized.end());
                std::replace(normalized.begin(), normalized.end(), ',', '.');

                try {
                    double value = std::stod(normalized);
                    if (negative) value = -value;
                    const double absoluteValue = std::abs(value);
                    if (!best || absoluteValue > bestAbs) {
                        best = value;
                        bestAbs = absoluteValue;
                    }
                } catch (...) {
                    core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::stodPrimary", std::current_exception());
                }
            } catch (...) {
                core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::tokenPrimary", std::current_exception());
            }
        }
        if (best) return best;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::primary", std::current_exception());
    }

    static const std::regex re2(R"((\(?-?\d+[\.,]\d{2}\)?))");
    try {
        std::optional<double> best;
        double bestAbs = 0.0;
        for (std::sregex_iterator it(s.begin(), s.end(), re2), end; it != end; ++it) {
            try {
                std::string token = (*it).str(1);
                if (core::application::importing::internal::hasShortDateToken(token)) continue;
                bool negative = false;
                if (!token.empty() && token.front() == '(' && token.back() == ')') { negative = true; token = token.substr(1, token.size() - 2); }
                if (!token.empty() && token.front() == '-') { negative = true; token = token.substr(1); }
                token.erase(std::remove(token.begin(), token.end(), '.'), token.end());
                std::replace(token.begin(), token.end(), ',', '.');
                try {
                    double value = std::stod(token);
                    if (negative) value = -value;
                    const double absoluteValue = std::abs(value);
                    if (!best || absoluteValue > bestAbs) {
                        best = value;
                        bestAbs = absoluteValue;
                    }
                } catch (...) {
                    core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::stodFallback", std::current_exception());
                }
            } catch (...) {
                core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::tokenFallback", std::current_exception());
            }
        }
        if (best) return best;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::parseAmountInternal::recovery", std::current_exception());
    }

    return std::nullopt;
}

}

namespace core::application::importing::transaction {

std::optional<double> parseAmountString(const std::string& line)
{
    return parseAmountInternal(line);
}

} // namespace core::application::importing::transaction
