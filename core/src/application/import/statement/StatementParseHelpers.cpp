/**
 * @file core/src/application/import/statement/StatementParseHelpers.cpp
 * @brief Implements helper types and routines shared by statement parsing stages.
 */

#include "core/application/import/statement/StatementParseHelpers.h"

#include "core/application/import/transaction/AmountParser.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/application/import/transaction/DefaultTransactionParser.h"
#include "core/application/import/internal/ParserHelpers.h"
#include "../../../utils/UniqId.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <regex>

namespace core::application::importing::statement::internal {
using core::application::importing::statement::DefaultStatementParser;
using core::application::importing::transaction::DefaultTransactionParser;
using core::application::importing::internal::ColumnGuess;
using core::application::importing::internal::RawLineLite;
using core::application::importing::transaction::internal::OcrLine;
using core::application::importing::transaction::internal::TransactionBlock;
using core::application::importing::transaction::internal::TransactionMainRow;
namespace helpers = core::application::importing::internal;
namespace heuristics = core::application::importing::internal;

namespace {

template <typename Action>
bool tryReportParserWarning(const char* origin, Action&& action)
{
    try {
        std::forward<Action>(action)();
        return true;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, origin, std::current_exception());
        return false;
    }
}

std::string fullBlockMetadata(const TransactionBlock& block)
{
    std::vector<std::string> parts;
    auto append = [&](const std::string& text) {
        const auto value = core::utils::trim(core::utils::collapseWhitespace(text));
        if (value.empty()) return;
        if (std::find(parts.begin(), parts.end(), value) != parts.end()) return;
        parts.push_back(value);
    };

    append(block.main.left.line.text);
    for (const auto& line : block.detailLines) {
        append(line.text);
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) out << '\n';
        out << parts[i];
    }
    return out.str();
}

RawLineLite toRawLineLite(const OcrLine& line)
{
    return RawLineLite{line.minX, line.maxX, line.minY, line.maxY, line.wordSpans, line.text};
}

OcrLine rawLiteToOcrLine(const RawLineLite& line)
{
    OcrLine out;
    out.minX = line.minX;
    out.maxX = line.maxX;
    out.minY = line.minY;
    out.maxY = line.maxY;
    out.wordSpans = line.wordSpans;
    out.text = line.text;
    return out;
}

RawLineLite mergeRawLines(const OcrLine& first, const OcrLine& second)
{
    RawLineLite merged = toRawLineLite(first);
    merged.minX = std::min(first.minX, second.minX);
    merged.maxX = std::max(first.maxX, second.maxX);
    merged.minY = std::min(first.minY, second.minY);
    merged.maxY = std::max(first.maxY, second.maxY);
    merged.wordSpans.insert(merged.wordSpans.end(), second.wordSpans.begin(), second.wordSpans.end());
    merged.text += std::string(" ") + second.text;
    return merged;
}

TransactionMainRow splitMergedMainRow(const OcrLine& first,
                                      const OcrLine& second,
                                      const ColumnGuess& cols) noexcept
{
    return helpers::splitMainRowFromRaw(mergeRawLines(first, second), cols.valutaX, cols.debitX, cols.creditX);
}

bool hasVerticalStartEvidence(const OcrLine& descriptiveLine,
                              const OcrLine& amountLine,
                              int valutaX,
                              int bandPx) noexcept
{
    return helpers::hasLeftDescriptiveText(descriptiveLine, valutaX)
        && !helpers::hasAmountLikeTokenInLine(descriptiveLine, valutaX)
        && helpers::hasAmountNearValuta(amountLine, valutaX, bandPx);
}

const std::regex& combinedMainRowRegex()
{
    static const std::regex pattern(R"((\d{2}\.\s*\d{2})\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?\b)", std::regex::ECMAScript);
    return pattern;
}

void appendDebugValue(std::vector<std::string>& debugOut, const char* prefix, const std::string& value)
{
    if (value.empty()) return;
    debugOut.push_back(std::string(prefix) + value);
}

bool looksLikeMetadataIdentifier(const std::string& value) noexcept
{
    try {
        static const std::regex ibanPattern(R"(^[A-Z]{2}\d{2}[A-Z0-9]{10,32}$)", std::regex::icase);
        static const std::regex bicPattern(R"(^[A-Z]{6}[A-Z0-9]{2}(?:[A-Z0-9]{3})?$)", std::regex::icase);
        static const std::regex referencePattern(R"((end-to-end|kundenreferenz|verwendungszweck|vertragskont|iban|bic|dauerauftrag|mandat|referenz|zweck))", std::regex::icase);
        return std::regex_match(value, ibanPattern)
            || std::regex_match(value, bicPattern)
            || std::regex_search(value, referencePattern);
    } catch (...) {
        return false;
    }
}

bool looksLikeStandaloneMetadataLine(const std::string& value) noexcept
{
    try {
        const auto trimmed = core::utils::trim(value);
        if (trimmed.empty()) return false;
        if (looksLikeMetadataIdentifier(trimmed)) return true;

        static const std::regex dateOnlyPattern(R"(^\d{1,2}[\./-]\d{1,2}(?:[\./-]\d{2,4})?$)", std::regex::icase);
        static const std::regex labelPattern(R"((end-to-end|kundenreferenz|verwendungszweck|vertragskont|iban|bic|dauerauftrag|mandat|referenz|zweck))", std::regex::icase);
        return std::regex_match(trimmed, dateOnlyPattern) || std::regex_search(trimmed, labelPattern);
    } catch (...) {
        return false;
    }
}

bool isCurrencyToken(const std::string& token) noexcept
{
    const auto normalized = helpers::normalizeAlnumLower(token);
    return normalized == "eur"
        || normalized == "usd"
        || normalized == "chf"
        || normalized == "gbp";
}

bool looksLikePureAmountColumnTail(const OcrLine& line) noexcept
{
    try {
        const auto tokens = core::utils::splitWhitespace(line.text);
        if (tokens.empty()) return false;

        bool sawAmount = false;
        for (const auto& token : tokens) {
            if (!helpers::findAmountTokenIndices(OcrLine{0, 0, 0, 0, {}, token}, -1, 0).empty()) {
                sawAmount = true;
                continue;
            }
            if (isCurrencyToken(token)) continue;

            for (const unsigned char ch : token) {
                if (std::isalpha(ch)) return false;
            }
            return false;
        }

        return sawAmount;
    } catch (...) {
        return false;
    }
}

}

OcrLine rawToOcrLine(const RawLine& line) {
    OcrLine out;
    out.minX = line.minX;
    out.maxX = line.maxX;
    out.minY = line.minY;
    out.maxY = line.maxY;
    out.wordSpans = line.wordSpans;
    out.text = line.text;
    return out;
}

bool isLikelyTransactionMainRowGeom(const RawLine& line, const ColumnModel& cols) {
    if (!cols.hasValuta() || (!cols.hasDebit() && !cols.hasCredit())) return false;
    const auto tokens = core::utils::splitWhitespace(line.text);
    if (tokens.size() != line.wordSpans.size() || tokens.size() < 3) return false;

    const int band = helpers::parserConfig.tokenNearBandForMainRow;
    const bool hasValuta = helpers::hasTokenNearX(rawToOcrLine(line), cols.valutaX, band);
    bool hasDebit = cols.hasDebit() ? helpers::hasTokenNearX(rawToOcrLine(line), cols.debitX, band) : false;
    bool hasCredit = cols.hasCredit() ? helpers::hasTokenNearX(rawToOcrLine(line), cols.creditX, band) : false;

    if (!hasDebit && !hasCredit) {
        tryReportParserWarning("core::parser::DefaultStatementParser::isLikelyTransactionMainRowGeom", [&] {
            auto amountIndices = helpers::findAmountTokenIndices(rawToOcrLine(line), cols.valutaX, helpers::parserConfig.amountNearValutaBandPx);
            if (!amountIndices.empty()) hasDebit = true;
        });
    }

    bool hasLeft = false;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& span = line.wordSpans[i];
        const int centerX = (span.first + span.second) / 2;
        if (centerX < cols.valutaX - helpers::parserConfig.leftDescriptiveOffsetPx) {
            hasLeft = true;
            break;
        }
    }

    return hasLeft && hasValuta && (hasDebit || hasCredit);
}

bool isLikelyTransactionHeaderLine(const OcrLine& line, const ColumnModel& cols) {
    return cols.hasValuta() && helpers::hasTokenNearX(line, cols.valutaX, 100);
}

std::pair<int, bool> detectHeaderRegion(const std::vector<OcrLine>& lines, size_t scanLines)
{
    int lastBlockY = -1;
    size_t idx = 0;
    int noiseCount = 0;
    for (const auto& line : lines) {
        ++idx;
        if (idx > scanLines) break;
        const auto& text = line.text;
        if (text.empty()) continue;

        bool isStrongHeaderLike = false;
        bool isNoise = false;
        try {
            if (heuristics::isTransactionsSectionHeader(text)) isStrongHeaderLike = true;
            if (helpers::normalizeAlnumLower(text).find("valuta") != std::string::npos) isStrongHeaderLike = true;
            if (heuristics::isDebitCreditHeaderLine(text)) isStrongHeaderLike = true;
            if (helpers::findFirstFullDate(text).has_value()) isStrongHeaderLike = true;
            if (heuristics::isHeaderNoiseLine(text)) isNoise = true;
        } catch (...) {
            core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::detectHeaderRegion", std::current_exception());
        }

        if (isStrongHeaderLike) {
            lastBlockY = std::max(lastBlockY, line.maxY);
        } else if (isNoise) {
            ++noiseCount;
            if (noiseCount >= 2) lastBlockY = std::max(lastBlockY, line.maxY);
        } else if (lastBlockY >= 0) {
            break;
        }
    }

    return {lastBlockY, lastBlockY >= 0};
}

std::optional<std::string> findDefaultBookingDate(const std::vector<OcrLine>& lines, size_t scanLines) noexcept
{
    try {
        static const std::regex reDate(R"((\d{2}\.\d{2}\.\d{4}))");
        const size_t n = std::min(lines.size(), scanLines);
        for (size_t i = 0; i < n; ++i) {
            const auto& text = lines[i].text;
            if (text.empty()) continue;
            if (heuristics::isPostTransactionFootnote(text)) continue;

            std::smatch match;
            const auto combinedPrev = (i > 0) ? (lines[i - 1].text + std::string(" ") + text) : text;
            const auto combinedNext = (i + 1 < lines.size()) ? (text + std::string(" ") + lines[i + 1].text) : text;
            if (std::regex_search(text, match, reDate)
                || std::regex_search(combinedPrev, match, reDate)
                || std::regex_search(combinedNext, match, reDate)) {
                return match.str(1);
            }
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::findDefaultBookingDate", std::current_exception());
    }
    return std::nullopt;
}

std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept
{
    try {
        const auto trimmed = core::utils::trim(line);
        const auto normalized = helpers::normalizeAlnumLower(trimmed);
        if (normalized.find("buchungsdat") == std::string::npos && normalized.find("buchungsdatum") == std::string::npos) return std::nullopt;

        if (auto date = helpers::findFirstFullDate(trimmed)) return date;

        std::string compact = trimmed;
        compact.erase(std::remove_if(compact.begin(), compact.end(), [](char c) {
            return !(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || std::isspace(static_cast<unsigned char>(c)));
        }), compact.end());
        return helpers::findFirstFullDate(compact);
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::findBookingDateInHeader", std::current_exception());
    }
    return std::nullopt;
}

bool detectEarlyEmptyPage(const std::vector<OcrLine>& lines, std::string& outDebug) noexcept
{
    try {
        const size_t checkLines = std::min(lines.size(), static_cast<size_t>(helpers::parserConfig.headerScanLines));
        int footLike = 0;
        int amountLikeCount = 0;
        const size_t start = lines.size() > checkLines ? lines.size() - checkLines : 0;
        for (size_t i = start; i < lines.size(); ++i) {
            const auto& text = lines[i].text;
            if (text.empty()) continue;
            if (heuristics::isPostTransactionFootnote(text)) ++footLike;

            const auto tokens = core::utils::splitWhitespace(text);
            for (const auto& token : tokens) {
                if (helpers::findAmountTokenIndices(OcrLine{0, 0, 0, 0, {}, token}, -1, 0).empty()) continue;
                ++amountLikeCount;
                break;
            }
        }

        std::ostringstream summary;
        summary << "page.earlyEmptyCheckBottom\tfootLike=" << footLike << "\tamts=" << amountLikeCount;
        outDebug = summary.str();
        return footLike >= static_cast<int>(checkLines * 2 / 3) && amountLikeCount == 0;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::detectEarlyEmptyPage", std::current_exception());
    }
    return false;
}

std::vector<helpers::RawLineLite> selectiveGroupMergeLinesRaw(const std::vector<helpers::RawLineLite>& lines,
                                                                      int maxGapPx,
                                                                      const helpers::ColumnGuess& seedCols)
{
    if (lines.size() <= 1) return lines;

    std::vector<helpers::RawLineLite> merged;
    merged.reserve(lines.size());
    auto current = lines.front();
    for (size_t i = 1; i < lines.size(); ++i) {
        const auto& next = lines[i];
        if (next.minY - current.maxY <= maxGapPx) {
            bool evidence = false;
            try {
                const auto currentLine = rawLiteToOcrLine(current);
                const auto nextLine = rawLiteToOcrLine(next);
                const bool currentStandaloneMetadata = looksLikeStandaloneMetadataLine(currentLine.text);
                const bool nextStandaloneMetadata = looksLikeStandaloneMetadataLine(nextLine.text);
                const bool currentHasAmount = helpers::hasAmountLikeTokenInLine(currentLine, seedCols.valutaX);
                const bool nextHasAmount = helpers::hasAmountLikeTokenInLine(nextLine, seedCols.valutaX);
                const bool currentNearValuta = helpers::hasTokenNearX(currentLine, seedCols.valutaX, helpers::parserConfig.tokenNearMergeBandPx);
                const bool nextNearValuta = helpers::hasTokenNearX(nextLine, seedCols.valutaX, helpers::parserConfig.tokenNearMergeBandPx);

                if (!currentStandaloneMetadata && !nextStandaloneMetadata) {
                    if (currentHasAmount && nextHasAmount) evidence = true;
                    if (!evidence && helpers::hasLeftDescriptiveText(currentLine, seedCols.valutaX) && nextNearValuta && nextHasAmount) evidence = true;
                }
                if (!evidence && !currentStandaloneMetadata && !nextStandaloneMetadata && helpers::isLooseTransactionLine(currentLine, seedCols.valutaX) && nextNearValuta && nextHasAmount) evidence = true;
                if (!evidence && !currentStandaloneMetadata && !nextStandaloneMetadata && helpers::isLooseTransactionLine(nextLine, seedCols.valutaX) && currentNearValuta && currentHasAmount) evidence = true;
            } catch (...) {
                core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::selectiveGroupMergeLinesRaw", std::current_exception());
            }

            if (evidence) {
                current.maxX = std::max(current.maxX, next.maxX);
                current.maxY = next.maxY;
                current.wordSpans.insert(current.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end());
                current.text += " " + next.text;
                continue;
            }
        }

        merged.push_back(std::move(current));
        current = next;
    }

    merged.push_back(std::move(current));
    return merged;
}

HeaderAnalysis analyzeHeaderWindow(const std::vector<OcrLine>& ocrLines,
                                   const std::vector<RawLine>& lines,
                                   const ColumnModel& seedCols,
                                   DefaultStatementParser::ParseResult& out)
{
    HeaderAnalysis result;
    static constexpr size_t headerScanLines = 12;

    auto [preHeaderBottomY, headerFound] = detectHeaderRegion(ocrLines, headerScanLines);
    result.preHeaderBottomY = preHeaderBottomY;
    result.headerFound = headerFound;
    result.headerBottomY = preHeaderBottomY;

    out.debugLines.push_back(std::string("header.prebottomY\t") + std::to_string(preHeaderBottomY));
    out.debugLines.push_back(std::string("header.found\t") + (headerFound ? "1" : "0"));

    try {
        const size_t debugCount = std::min(ocrLines.size(), headerScanLines);
        for (size_t headerIndex = 0; headerIndex < debugCount; ++headerIndex) {
            const auto& line = ocrLines[headerIndex];
            const auto& text = line.text;
            try {
                const bool isTxSection = heuristics::isTransactionsSectionHeader(text);
                const bool isDebitCredit = heuristics::isDebitCreditHeaderLine(text);
                const bool isNoise = heuristics::isHeaderNoiseLine(text);
                const bool hasFullDate = helpers::findFirstFullDate(text).has_value();
                bool hasAmount = helpers::hasAmountLikeTokenInLine(line, seedCols.valutaX);
                bool hasValutaTokenNear = false;
                tryReportParserWarning("core::parser::DefaultStatementParser::headerCandidateValutaNear", [&] {
                    if (seedCols.valutaX >= 0) hasValutaTokenNear = helpers::hasTokenNearX(line, seedCols.valutaX, helpers::parserConfig.tokenNearBandForMainRow);
                });
                std::ostringstream summary;
                summary << "header.candidate\tline=" << headerIndex << "\ttext=" << text << "\ttxSection=" << (isTxSection ? "1" : "0") << "\tdebitcredit=" << (isDebitCredit ? "1" : "0") << "\tnoise=" << (isNoise ? "1" : "0") << "\tfullDate=" << (hasFullDate ? "1" : "0") << "\thasAmt=" << (hasAmount ? "1" : "0") << "\tvalutaNear=" << (hasValutaTokenNear ? "1" : "0");
                out.debugLines.push_back(summary.str());
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::txt", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerCandidates", std::current_exception()); }

    try {
        int earliestAmountY = std::numeric_limits<int>::max();
        bool foundAmountLine = false;
        for (const auto& line : lines) {
            try {
                OcrLine ocrLine = rawToOcrLine(line);
                const bool hasAmount = helpers::hasAmountLikeTokenInLine(ocrLine, seedCols.valutaX);
                const bool dateLeft = helpers::hasShortDateToken(ocrLine.text) && helpers::hasLeftDescriptiveText(ocrLine, seedCols.valutaX);
                if (!hasAmount && !dateLeft) continue;
                earliestAmountY = std::min(earliestAmountY, line.maxY);
                foundAmountLine = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::combined", std::current_exception()); }
        }
        if (foundAmountLine && earliestAmountY != std::numeric_limits<int>::max()) {
            result.headerBottomY = std::min(result.headerBottomY, earliestAmountY - 2);
            out.debugLines.push_back(std::string("header.adjustedForEarliestAmount\t") + std::to_string(result.headerBottomY));
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerAdjustEarliestAmount", std::current_exception()); }

    try {
        for (const auto& line : lines) result.pageMaxY = std::max(result.pageMaxY, line.maxY);
    } catch (...) { result.pageMaxY = -1; }

    return result;
}

static bool looksLikeRescuedMainLine(const OcrLine& line, const ColumnModel& cols)
{
    bool looksMain = false;
    try { if (helpers::hasAmountLikeTokenInLine(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::hasAmount", std::current_exception()); }
    try { if (!looksMain && helpers::isLooseTransactionLine(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::isLoose", std::current_exception()); }
    try { if (!looksMain && helpers::hasShortDateToken(line.text) && helpers::hasLeftDescriptiveText(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::dateAndLeft", std::current_exception()); }
    return looksMain;
}

void rescueOrphanBlocks(std::vector<TransactionBlock>& blocks,
                        const std::vector<OcrLine>& orphanLines,
                        const ColumnModel& cols,
                        const std::string& currentBookingDate,
                        DefaultStatementParser::ParseResult& out)
{
    try {
        for (const auto& orphanLine : orphanLines) {
            try {
                if (!looksLikeRescuedMainLine(orphanLine, cols)) continue;
                TransactionBlock block;
                block.bookingDateGroup = currentBookingDate;
                helpers::ColumnGuess guess{ cols.valutaX, cols.debitX, cols.creditX };
                block.main = handleMainRow(orphanLine, guess, false, out.debugLines);
                blocks.push_back(std::move(block));
                out.debugLines.push_back(std::string("tx.start.rescued\ttext=") + orphanLine.text);
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::loop", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue", std::current_exception()); }
}

std::optional<std::pair<TransactionMainRow, int>> tryVerticalStart(const std::vector<OcrLine>& lines,
                                                                   size_t li,
                                                                   const helpers::ColumnGuess& cols) noexcept
{
    try {
        if (li >= lines.size()) return std::nullopt;

        const int band = std::max(helpers::parserConfig.amountNearValutaBandPx, helpers::parserConfig.tokenNearMergeBandPx);
        const auto& current = lines[li];

        if (li > 0) {
            const auto& previous = lines[li - 1];
            if (hasVerticalStartEvidence(previous, current, cols.valutaX, band)) {
                return std::make_optional(std::make_pair(splitMergedMainRow(previous, current, cols), 0));
            }
            if (helpers::hasShortDateToken(previous.text) && helpers::hasAmountNearValuta(current, cols.valutaX, band)) {
                return std::make_optional(std::make_pair(splitMergedMainRow(previous, current, cols), 0));
            }
        }

        if (li + 1 < lines.size()) {
            const auto& next = lines[li + 1];
            if (hasVerticalStartEvidence(current, next, cols.valutaX, band)) {
                return std::make_optional(std::make_pair(splitMergedMainRow(current, next, cols), 1));
            }
            if (helpers::hasShortDateToken(current.text) && helpers::hasAmountNearValuta(next, cols.valutaX, band)) {
                return std::make_optional(std::make_pair(splitMergedMainRow(current, next, cols), 1));
            }
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::tryVerticalStart", std::current_exception());
    }
    return std::nullopt;
}

std::optional<std::pair<TransactionMainRow, int>> tryCombinedStart(const std::vector<OcrLine>& lines,
                                                                   size_t li,
                                                                   const helpers::ColumnGuess& cols) noexcept
{
    try {
        if (li >= lines.size()) return std::nullopt;

        const auto& current = lines[li];
        const auto combinedPrev = (li > 0) ? (lines[li - 1].text + std::string(" ") + current.text) : current.text;
        const auto combinedNext = (li + 1 < lines.size()) ? (current.text + std::string(" ") + lines[li + 1].text) : current.text;

        if (!combinedPrev.empty() && std::regex_search(combinedPrev, combinedMainRowRegex())) {
            return std::make_optional(std::make_pair(helpers::splitMainRowFromOcrLine(current, cols.valutaX, cols.debitX, cols.creditX), 0));
        }
        if (!combinedNext.empty() && std::regex_search(combinedNext, combinedMainRowRegex())) {
            return std::make_optional(std::make_pair(helpers::splitMainRowFromOcrLine(current, cols.valutaX, cols.debitX, cols.creditX), 1));
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::tryCombinedStart", std::current_exception());
    }
    return std::nullopt;
}

void appendDetailLine(TransactionBlock& cur,
                      const OcrLine& line,
                      const helpers::ColumnGuess& cols,
                      std::vector<std::string>* debugOut) noexcept
{
    try {
        if (cols.valutaX >= 0 && helpers::hasLeftDescriptiveText(line, cols.valutaX)) {
            cur.detailLines.push_back(line);
            if (debugOut) debugOut->push_back(std::string("detail.append.helper.whole.left\ttext=") + line.text);
            return;
        }

        if (cols.valutaX >= 0) {
            const auto tokens = core::utils::splitWhitespace(line.text);
            if (tokens.size() == line.wordSpans.size()) {
                size_t cut = tokens.size();
                for (size_t i = 0; i < tokens.size(); ++i) {
                    const auto& span = line.wordSpans[i];
                    const int centerX = (span.first + span.second) / 2;
                    if (centerX >= cols.valutaX) {
                        cut = i;
                        break;
                    }
                }

                const auto rawLine = toRawLineLite(line);
                const auto leftPart = helpers::toOcrLineFromRawWords(rawLine, 0, cut);
                const auto rightPart = helpers::toOcrLineFromRawWords(rawLine, cut, tokens.size());
                const auto fullText = core::utils::trim(line.text);
                const auto leftText = core::utils::trim(leftPart.text);
                const auto rightText = core::utils::trim(rightPart.text);

                bool keepWholeLine = leftText.empty() || looksLikeMetadataIdentifier(fullText);
                if (!keepWholeLine && !rightText.empty()) {
                    keepWholeLine = !looksLikePureAmountColumnTail(rightPart);
                }

                if (keepWholeLine) {
                    cur.detailLines.push_back(line);
                    if (debugOut) debugOut->push_back(std::string("detail.append.helper.whole\ttext=") + line.text + std::string("\tcut=") + std::to_string(cut));
                    return;
                }

                if (!leftText.empty()) cur.detailLines.push_back(leftPart);
                if (debugOut) debugOut->push_back(std::string("detail.append.helper\ttext=") + line.text + std::string("\tcut=") + std::to_string(cut));
                return;
            }
        }

        cur.detailLines.push_back(line);
        if (debugOut) debugOut->push_back(std::string("detail.append.helper.whole\t") + line.text);
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::appendDetailLine", std::current_exception());
    }
}

TransactionMainRow handleMainRow(const OcrLine& line,
                                 const helpers::ColumnGuess& cols,
                                 bool isGeom,
                                 std::vector<std::string>& debugOut) noexcept
{
    try {
        auto mainRow = helpers::splitMainRowFromOcrLine(line, cols.valutaX, cols.debitX, cols.creditX);
        std::ostringstream selection;
        selection << "tx.main.split\tline?\tvalutaX=" << cols.valutaX << "\tdebitX=" << cols.debitX << "\tcreditX=" << cols.creditX;
        debugOut.push_back(selection.str());
        if (isGeom) debugOut.push_back(std::string("tx.main.geom\t") + line.text);
        debugOut.push_back(std::string("tx.main\t") + line.text);
        appendDebugValue(debugOut, "tx.main.left\t", mainRow.left.line.text);
        appendDebugValue(debugOut, "tx.main.valuta\t", mainRow.valuta.line.text);
        appendDebugValue(debugOut, "tx.main.debit\t", mainRow.debit.line.text);
        appendDebugValue(debugOut, "tx.main.credit\t", mainRow.credit.line.text);
        return mainRow;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::handleMainRow", std::current_exception());
    }
    return TransactionMainRow{};
}

void attachOrphansToBlocks(std::vector<TransactionBlock>& blocks,
                           const std::vector<OcrLine>& orphans,
                           int maxGapPx,
                           int valutaX,
                           std::vector<std::string>* debugOut) noexcept
{
    try {
        for (const auto& orphan : orphans) {
            int bestBlock = -1;
            int bestGap = std::numeric_limits<int>::max();
            for (size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
                const auto& block = blocks[blockIndex];
                if (block.main.left.empty()) continue;
                const int mainBottom = block.main.left.line.maxY;
                if (orphan.minY < mainBottom) continue;
                const int gap = orphan.minY - mainBottom;
                if (gap <= maxGapPx && gap < bestGap) {
                    bestGap = gap;
                    bestBlock = static_cast<int>(blockIndex);
                }
            }

            if (bestBlock < 0) {
                if (debugOut) debugOut->push_back(std::string("orphan.unmatched\ttext=") + orphan.text);
                continue;
            }

            const bool crossesValuta = valutaX >= 0 && orphan.maxX >= valutaX + 40;
            const bool hasAmountLike = crossesValuta && helpers::hasAmountLikeTokenInLine(orphan, valutaX);
            if (!crossesValuta || !hasAmountLike) {
                appendDetailLine(blocks[static_cast<size_t>(bestBlock)], orphan, {valutaX, -1, -1}, debugOut);
                if (debugOut) debugOut->push_back(std::string("orphan.attach\tblock=") + std::to_string(bestBlock) + std::string("\tgap=") + std::to_string(bestGap) + std::string("\ttext=") + orphan.text);
            } else if (debugOut) {
                debugOut->push_back(std::string("orphan.drop\treason=amount-column-overlap\ttext=") + orphan.text);
            }
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::attachOrphansToBlocks", std::current_exception());
    }
}

void appendPageSummary(const std::vector<RawLine>& lines,
                       int headerBottomY,
                       const std::vector<TransactionBlock>& blocks,
                       int txStartLooseCount,
                       DefaultStatementParser::ParseResult& out)
{
    try {
        std::ostringstream summary;
        summary << "page.summary\tocr.lines=" << lines.size() << "\theader.prebottomY=" << headerBottomY << "\tblocks=" << blocks.size() << "\ttxStartLoose=" << txStartLooseCount;
        out.debugLines.push_back(summary.str());
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::pageSummary", std::current_exception()); }
}

static void appendBlockDebug(const TransactionBlock& block,
                             const ColumnModel& cols,
                             DefaultStatementParser::ParseResult& out)
{
    try {
        out.debugLines.push_back(std::string("block.debug\tbookingDateGroup=") + block.bookingDateGroup);
        out.debugLines.push_back(std::string("block.main.left.text\t") + block.main.left.line.text);
        out.debugLines.push_back(std::string("block.main.valuta.empty\t") + (block.main.valuta.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.valuta.text\t") + (block.main.valuta.empty() ? std::string("(none)") : block.main.valuta.line.text));
        out.debugLines.push_back(std::string("block.main.debit.empty\t") + (block.main.debit.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.debit.text\t") + (block.main.debit.empty() ? std::string("(none)") : block.main.debit.line.text));
        out.debugLines.push_back(std::string("block.main.credit.empty\t") + (block.main.credit.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.credit.text\t") + (block.main.credit.empty() ? std::string("(none)") : block.main.credit.line.text));
        out.debugLines.push_back(std::string("cols.state\tvalutaX=") + std::to_string(cols.valutaX) + std::string("\tdebitX=") + std::to_string(cols.debitX) + std::string("\tcreditX=") + std::to_string(cols.creditX));
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::blockDebug", std::current_exception()); }
}

static void applyCellAmountOverride(core::application::importing::draft::TransactionDraft& tx,
                                    const TransactionBlock& block,
                                    const ColumnModel& cols,
                                    const core::ports::text_recognition::ExtractResult& ocr,
                                    DefaultStatementParser::ParseResult& out)
{
    try {
        if (ocr.tables.empty() || ocr.tables[0].cells.empty()) return;

        const auto& table = ocr.tables[0];
        auto overlapsLine = [&](const core::ports::text_recognition::Cell& cell) -> bool {
            const int cellTop = cell.bbox.y;
            const int cellBottom = cell.bbox.y + cell.bbox.height;
            return !(cellTop > block.main.left.line.maxY || cellBottom < block.main.left.line.minY);
        };

        bool usedCell = false;
        if (cols.hasCredit() && cols.creditCol >= 0) {
            for (const auto& cell : table.cells) {
                if (cell.col != cols.creditCol || !overlapsLine(cell)) continue;
                out.debugLines.push_back(std::string("cell.amount_used\t") + cell.text);
                if (auto value = core::application::importing::transaction::parseAmountString(cell.text)) {
                    tx.amount = *value;
                    usedCell = true;
                    out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount));
                }
                break;
            }
        }

        if (!usedCell && cols.hasDebit() && cols.debitCol >= 0) {
            for (const auto& cell : table.cells) {
                if (cell.col != cols.debitCol || !overlapsLine(cell)) continue;
                out.debugLines.push_back(std::string("cell.amount_used\t") + cell.text);
                if (auto value = core::application::importing::transaction::parseAmountString(cell.text)) {
                    tx.amount = -std::abs(*value);
                    out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount));
                }
                break;
            }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::cellOverride", std::current_exception()); }
}

static void attachProofCrop(core::application::importing::draft::TransactionDraft& tx,
                            const TransactionBlock& block,
                            int txIndex,
                            const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                            const std::string& pageCropImagePath,
                            const std::vector<uint8_t>& pageCropImageBytes,
                            DefaultStatementParser::ParseResult& out)
{
    try {
        if (!opencv || (pageCropImagePath.empty() && pageCropImageBytes.empty())) return;

        int minX = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int minY = std::numeric_limits<int>::max();
        int maxY = std::numeric_limits<int>::min();

        auto accBounds = [&](const OcrLine& line) {
            minX = std::min(minX, line.minX);
            maxX = std::max(maxX, line.maxX);
            minY = std::min(minY, line.minY);
            maxY = std::max(maxY, line.maxY);
        };

        if (!block.main.left.empty()) accBounds(block.main.left.line);
        for (const auto& line : block.detailLines) accBounds(line);
        if (minX == std::numeric_limits<int>::max()) return;

        core::ports::image_processing::CropRequest request;
        if (!pageCropImagePath.empty()) request.imagePath = std::filesystem::path(pageCropImagePath);
        request.imageBytes = pageCropImageBytes;
        request.uniqIdPrefix = std::string(core::utils::makeUniqId());
        request.filePrefix = std::string("opencv_proof_tx") + std::to_string(txIndex);
        request.outputFormat = core::ports::image_processing::CropRequest::OutputFormat::Jpg;
        request.jpegQuality = 92;
        request.bbox.x = 0;
        request.bbox.y = std::max(0, minY - 20);
        request.bbox.width = 1 << 30;
        request.bbox.height = std::max(1, (maxY - minY) + 24);

        try {
            out.debugLines.push_back(std::string("crop.request\t") + request.imagePath.string() + std::string("\tfilePrefix=") + request.filePrefix);
            const auto response = opencv->crop(request);
            out.debugLines.push_back(std::string("crop.result.count\t") + std::to_string(response.croppedImagePaths.size()));
            if (!response.croppedImageBytes.empty() && !response.croppedImageBytes.front().empty()) {
                tx.proofImageData = response.croppedImageBytes.front();
            }
        } catch (const std::exception& ex) {
            out.debugLines.push_back(std::string("crop.exception\t") + ex.what());
        } catch (...) {
            out.debugLines.push_back(std::string("crop.exception\tunknown"));
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::proofCrop", std::current_exception()); }
}

void appendTransactionsFromBlocks(const std::vector<TransactionBlock>& blocks,
                                  const ColumnModel& cols,
                                  const core::ports::text_recognition::ExtractResult& ocr,
                                  const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                                  const std::string& pageCropImagePath,
                                  const std::vector<uint8_t>& pageCropImageBytes,
                                  int& txIndex,
                                  DefaultStatementParser::ParseResult& out)
{
    for (const auto& block : blocks) {
        if (block.main.left.empty()) continue;

        appendBlockDebug(block, cols, out);

        std::vector<std::string> txDebug;
        const auto parsed = DefaultTransactionParser::parseTransaction(block, &txDebug);
        for (const auto& line : txDebug) out.debugLines.push_back(std::string("txdbg\t") + line);

        core::application::importing::draft::TransactionDraft tx;
        tx.name = parsed.name;
        tx.bookingDate = parsed.bookingDate;
        tx.valuta = parsed.valuta;
        tx.amount = parsed.amount;
        const auto blockMetadata = fullBlockMetadata(block);
        tx.metadata = !blockMetadata.empty() ? blockMetadata : parsed.metadata;

        applyCellAmountOverride(tx, block, cols, ocr, out);
        attachProofCrop(tx, block, txIndex, opencv, pageCropImagePath, pageCropImageBytes, out);

        ++txIndex;
        out.transactions.push_back(std::move(tx));
    }
}

}
