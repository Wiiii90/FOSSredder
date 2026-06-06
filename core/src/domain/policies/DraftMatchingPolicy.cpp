/**
 * @file core/src/domain/policies/DraftMatchingPolicy.cpp
 * @brief Implements draft matching policy helpers for import-time inference.
 */

#include "core/pch.h"
/**
 * @file core/src/domain/policies/DraftMatchingPolicy.cpp
 * @brief Shared draft matching and parsing helpers.
 */

#include "core/domain/policies/DraftMatchingPolicy.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <regex>
#include <sstream>

namespace core::domain::policies::matching {

namespace {

char lowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

template <typename TEntity>
std::vector<core::domain::Alias> aliasUsages(const TEntity& entity)
{
    std::vector<core::domain::Alias> out;
    out.reserve(entity.aliases().size());
    for (const auto& alias : entity.aliases()) {
        if (alias.value().empty()) continue;
        out.push_back(alias);
    }
    return out;
}

bool containsId(const std::vector<std::string>& ids, const std::string& id)
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

bool appendUnique(std::vector<std::string>& values, const std::string& value)
{
    if (value.empty() || containsId(values, value)) return false;
    values.push_back(value);
    return true;
}

} // namespace

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool matchesDraftText(const std::string& left, const std::string& right)
{
    return normalizeText(left) == normalizeText(right);
}

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\r') continue;
        if (c == '\n') {
            const auto line = trim(current);
            if (!line.empty()) lines.push_back(line);
            current.clear();
            continue;
        }
        current.push_back(c);
    }
    const auto tail = trim(current);
    if (!tail.empty()) lines.push_back(tail);
    return lines;
}

std::string joinNonEmptyLines(const std::vector<std::string>& values, const std::string& separator)
{
    std::ostringstream out;
    bool first = true;
    for (const auto& value : values) {
        const auto trimmedValue = trim(value);
        if (trimmedValue.empty()) continue;
        if (!first) out << separator;
        first = false;
        out << trimmedValue;
    }
    return out.str();
}

bool lineLooksMetadataBoundary(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return true;
    if (value.find(':') != std::string::npos) return true;

    static const std::regex amountPattern("(eur|usd|chf|gbp)\\s*[-+]?\\d|\\b\\d+(?:[\\.,]\\d{2})\\b", std::regex::icase);
    return std::regex_search(value, amountPattern);
}

bool lineLooksActorStopSignal(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return true;
    if (lineLooksMetadataBoundary(value)) return true;

    static const std::regex ibanPattern("^[A-Z]{2}\\d{2}[A-Z0-9]{10,32}$", std::regex::icase);
    static const std::regex bicPattern("^[A-Z]{6}[A-Z0-9]{2}(?:[A-Z0-9]{3})?$", std::regex::icase);
    static const std::regex dateOnlyPattern("^\\d{1,2}[\\./-]\\d{1,2}[\\./-]\\d{2,4}$", std::regex::icase);
    static const std::regex bankOrReferencePattern("(end-to-end|kundenreferenz|verwendungszweck|vertragskont|iban|bic|dauerauftrag|mandat|referenz|zweck)", std::regex::icase);
    return std::regex_match(value, ibanPattern)
        || std::regex_match(value, bicPattern)
        || std::regex_match(value, dateOnlyPattern)
        || std::regex_search(value, bankOrReferencePattern);
}

bool lineLooksActorContinuation(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return false;
    if (lineLooksActorStopSignal(value)) return false;

    int digitCount = 0;
    int alphaCount = 0;
    for (const unsigned char ch : value) {
        if (std::isdigit(ch)) ++digitCount;
        if (std::isalpha(ch)) ++alphaCount;
    }

    if (alphaCount == 0) return false;
    if (digitCount == 0) return true;
    if (digitCount <= 2 && alphaCount >= digitCount * 2) return true;

    static const std::regex compactReferencePattern("^[A-Z]{1,6}(?:[ -]?[A-Z0-9]{2,8}){1,5}$", std::regex::icase);
    return value.size() <= 36 && std::regex_match(value, compactReferencePattern);
}

std::string firstMeaningfulLine(const std::string& text)
{
    for (const auto& line : splitLines(text)) {
        if (!line.empty()) return line;
    }
    return {};
}

std::string leadingPhrase(const std::string& text, std::size_t maxTokens)
{
    std::istringstream in(text);
    std::ostringstream out;
    std::string token;
    std::size_t count = 0;
    while (in >> token) {
        if (count++ > 0) out << ' ';
        out << token;
        if (count >= maxTokens) break;
    }
    return trim(out.str());
}

std::string normalizeText(std::string text)
{
    for (auto& ch : text) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (byte < 128 && std::isalpha(byte)) {
            ch = static_cast<char>(std::tolower(byte));
        } else if (byte < 128 && std::isalnum(byte)) {
            ch = static_cast<char>(byte);
        } else {
            ch = ' ';
        }
    }
    return trim(text);
}

std::vector<std::string> tokens(const std::string& text)
{
    std::vector<std::string> out;
    std::istringstream in(normalizeText(text));
    for (std::string token; in >> token;) out.push_back(token);
    return out;
}

std::string leadingText(const std::string& text, std::size_t maxTokens)
{
    const auto parts = tokens(text);
    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size() && i < maxTokens; ++i) {
        if (i > 0) out << ' ';
        out << parts[i];
    }
    return trim(out.str());
}

bool tokenIsMatchable(const std::string& token)
{
    if (token.size() >= 3) return true;
    return std::all_of(token.begin(), token.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

int tokenOverlapScore(const std::vector<std::string>& sourceTokens, const std::vector<std::string>& targetTokens)
{
    if (sourceTokens.empty() || targetTokens.empty()) return 0;
    int matches = 0;
    for (const auto& token : targetTokens) {
        if (!tokenIsMatchable(token)) continue;
        if (std::find(sourceTokens.begin(), sourceTokens.end(), token) != sourceTokens.end()) ++matches;
    }
    return matches;
}

double aliasHitWeight(int hitCount)
{
    return std::min(0.7, std::log1p(std::max(1, hitCount)) / 5.0);
}

double aliasRecencyWeight(const std::string& lastUsedAt)
{
    (void)lastUsedAt;
    return 0.0;
}

std::string metadataSignalText(const std::string& metadata)
{
    std::ostringstream out;
    bool first = true;
    const auto lines = splitLines(metadata);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        const auto colon = line.find(':');
        if (colon != std::string::npos) {
            const auto key = trim(line.substr(0, colon));
            const auto value = trim(line.substr(colon + 1));
            if (!key.empty()) {
                if (!first) out << ' ';
                first = false;
                out << key;
            }
            if (!value.empty()) {
                if (!first) out << ' ';
                first = false;
                out << value;
            } else if (i + 1 < lines.size() && !trim(lines[i + 1]).empty()) {
                if (!first) out << ' ';
                first = false;
                out << trim(lines[i + 1]);
                ++i;
            }
            continue;
        }

        if (!first) out << ' ';
        first = false;
        out << line;
    }
    return trim(out.str());
}

std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata)
{
    std::vector<std::string> out;
    const std::regex refKeyword("(kund(?:en)?ref|kundenreferenz|referenz|verweis|vertragsnr|vertragsnummer|nr|nummer|ref)", std::regex::icase);
    const std::regex digits("\\b\\d{3,}\\b");

    for (const auto& line : splitLines(metadata)) {
        if (line.empty()) continue;
        if (std::regex_search(line, refKeyword)) appendUnique(out, line);

        for (std::sregex_iterator it(line.begin(), line.end(), digits), end; it != end; ++it) {
            appendUnique(out, it->str());
        }
    }

    return out;
}

std::vector<std::string> knownContractTypes(const core::domain::catalog::WorkspaceCatalog& state)
{
    std::vector<std::string> types;
    std::vector<std::string> normalizedSeen;
    types.reserve(state.contracts().size());
    normalizedSeen.reserve(state.contracts().size());
    for (const auto& contract : state.contracts()) {
        if (!contract) continue;
        const auto type = trim(contract->type());
        const auto normalizedType = normalizeText(type);
        if (normalizedType.empty()) continue;
        if (std::find(normalizedSeen.begin(), normalizedSeen.end(), normalizedType) != normalizedSeen.end()) continue;
        normalizedSeen.push_back(normalizedType);
        types.push_back(type);
    }
    return types;
}

std::string extractTypeText(const core::domain::catalog::WorkspaceCatalog& state,
                            const std::string& signalText,
                            const std::vector<std::string>& lines)
{
    const auto contractTypes = knownContractTypes(state);
    if (contractTypes.empty()) return {};

    for (const auto& line : lines) {
        const auto value = trim(line);
        const auto separatorPos = value.find('-');
        if (value.empty() || separatorPos == std::string::npos) continue;

        const auto left = trim(value.substr(0, separatorPos));
        if (left.empty()) continue;

        const auto normalizedLeft = normalizeText(left);
        for (const auto& type : contractTypes) {
            if (normalizeText(type) == normalizedLeft) return type;
        }
    }

    const auto fullText = normalizeText(signalText);
    const auto headText = normalizeText(joinNonEmptyLines(lines, " "));

    std::string bestType;
    int bestScore = std::numeric_limits<int>::min();
    for (const auto& type : contractTypes) {
        const auto normalizedType = normalizeText(type);
        if (normalizedType.empty()) continue;

        int score = 0;
        const auto fullPos = fullText.find(normalizedType);
        if (fullPos != std::string::npos) {
            score += 500;
            score += std::max(0, 120 - static_cast<int>(fullPos));
        }

        const auto headPos = headText.find(normalizedType);
        if (headPos != std::string::npos) {
            score += 220;
            score += std::max(0, 80 - static_cast<int>(headPos));
        }

        if (score <= 0) continue;
        score += static_cast<int>(normalizedType.size());
        if (score > bestScore) {
            bestScore = score;
            bestType = type;
        }
    }

    return bestType;
}

std::string extractActorText(const std::vector<std::string>& lines, const std::string& typeText)
{
    std::vector<std::string> actorLines;
    actorLines.reserve(lines.size());

    const auto normalizedType = normalizeText(typeText);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const auto& line = lines[index];
        const auto value = trim(line);
        if (value.empty()) continue;

        if (actorLines.empty()) {
            actorLines.push_back(value);
            continue;
        }

        if (lineLooksActorStopSignal(value)) break;
        if (!lineLooksActorContinuation(value)) break;
        actorLines.push_back(value);
        if (index >= 1) break;
    }

    if (actorLines.empty()) return {};

    auto actorText = joinNonEmptyLines(actorLines, " ");
    if (!normalizedType.empty() && normalizeText(actorText) == normalizedType && actorLines.size() > 1) {
        actorText = joinNonEmptyLines({actorLines.front(), actorLines[1]}, " ");
    }
    return trim(actorText);
}

} // namespace core::domain::policies::matching
