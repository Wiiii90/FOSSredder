/**
 * @file core/include/core/application/import/internal/ParserHelpers.h
 * @brief Declares low-level OCR helper routines for statement parsing.
 */

#pragma once

#include "core/application/import/internal/ParserConfig.h"
#include "core/application/import/transaction/TransactionParserTypes.h"
#include "core/application/import/transaction/AmountParser.h"
#include "core/application/import/internal/ParserDateUtils.h"
#include "core/application/import/internal/ParserHeuristics.h"

#include "core/ports/infra/text-recognition/TextRecognitionResult.h"

#include <initializer_list>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace core::application::importing::internal {

/**
 * @brief Finds the indices of amount-like tokens in an OCR line.
 * @param line OCR line to inspect.
 * @param valutaX Optional currency column reference.
 * @param bandPx Matching band in pixels around the reference column.
 * @return Indices of tokens that look like amounts.
 */
std::vector<size_t> findAmountTokenIndices(const core::application::importing::transaction::internal::OcrLine& line, int valutaX = -1, int bandPx = 0) noexcept;
/**
 * @brief Normalizes alphanumeric text to lower-case ASCII for matching.
 * @param s Input string.
 * @return Normalized string.
 */
std::string normalizeAlnumLower(const std::string& s) noexcept;
/**
 * @brief Locates the horizontal center of a token inside an OCR line.
 * @param line OCR line to inspect.
 * @param tokenLower Lower-case token to find.
 * @return Token center x-coordinate when the token is present.
 */
std::optional<int> findTokenCenterX(const core::application::importing::transaction::internal::OcrLine& line, const std::string& tokenLower) noexcept;
/**
 * @brief Locates the horizontal center of a multi-token phrase in an OCR line.
 * @param line OCR line to inspect.
 * @param phraseLower Lower-case phrase tokens to find.
 * @return Phrase center x-coordinate when the phrase is present.
 */
std::optional<int> findPhraseCenterX(const core::application::importing::transaction::internal::OcrLine& line, std::initializer_list<const char*> phraseLower) noexcept;
/**
 * @brief Checks whether any token in the line is close to a horizontal position.
 * @param line OCR line to inspect.
 * @param x Reference x-coordinate.
 * @param bandPx Allowed band around the reference x-coordinate.
 * @return True when a token lies inside the band.
 */
bool hasTokenNearX(const core::application::importing::transaction::internal::OcrLine& line, int x, int bandPx) noexcept;
/**
 * @brief Checks whether the line contains amount-like tokens near the valuta column.
 * @param line OCR line to inspect.
 * @param valutaX Reference currency column.
 * @return True when amount-like content is found near the valuta column.
 */
bool hasAmountLikeTokenInLine(const core::application::importing::transaction::internal::OcrLine& line, int valutaX) noexcept;
/**
 * @brief Checks whether the line contains descriptive text to the left of the valuta column.
 * @param line OCR line to inspect.
 * @param valutaX Reference currency column.
 * @return True when descriptive text is detected on the left side.
 */
bool hasLeftDescriptiveText(const core::application::importing::transaction::internal::OcrLine& line, int valutaX) noexcept;
/**
 * @brief Checks whether an amount appears close to the valuta reference.
 * @param line OCR line to inspect.
 * @param valutaX Reference currency column.
 * @param bandPx Allowed band around the reference x-coordinate.
 * @return True when an amount-like token is near the currency reference.
 */
bool hasAmountNearValuta(const core::application::importing::transaction::internal::OcrLine& line, int valutaX, int bandPx) noexcept;
/**
 * @brief Checks whether the line looks like a loosely formatted transaction row.
 * @param line OCR line to inspect.
 * @param valutaX Reference currency column.
 * @return True when the line resembles a transaction row.
 */
bool isLooseTransactionLine(const core::application::importing::transaction::internal::OcrLine& line, int valutaX) noexcept;
/**
 * @brief Finds and parses an amount token inside an OCR line.
 * @param line OCR line to inspect.
 * @param valutaX Reference currency column.
 * @param debugOut Optional debug output sink.
 * @return Parsed amount when a valid token is found.
 */
std::optional<double> findAndParseAmountInLine(const core::application::importing::transaction::internal::OcrLine& line, int valutaX = -1, std::vector<std::string>* debugOut = nullptr) noexcept;

/**
 * @brief Lightweight raw-line representation used during grouping and splitting.
 */
struct RawLineLite {
    int minX;
    int maxX;
    int minY;
    int maxY;
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

/**
 * @brief Builds OCR lines from raw Tesseract word output.
 * @param words OCR words to convert.
 * @return Constructed OCR lines.
 */
std::vector<core::application::importing::transaction::internal::OcrLine> buildOcrLinesFromWords(const std::vector<core::ports::text_recognition::Word>& words);

/**
 * @brief Stores the inferred column positions for an OCR page.
 */
struct ColumnGuess {
    int valutaX = -1;
    int debitX = -1;
    int creditX = -1;
};

/**
 * @brief Infers a column layout from a set of OCR lines.
 * @param lines OCR lines to scan.
 * @param scanN Maximum number of leading lines to inspect.
 * @return Inferred column guess.
 */
ColumnGuess inferColumnModelFromLines(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines, size_t scanN = 16) noexcept;
/**
 * @brief Merges raw lines using a gap-based heuristic.
 * @param lines Raw lines to merge.
 * @param maxGapPx Maximum gap between lines that should still be merged.
 * @return Merged raw lines.
 */
std::vector<RawLineLite> groupMergeLinesRaw(const std::vector<RawLineLite>& lines, int maxGapPx);
/**
 * @brief Converts a range of raw words into one OCR line.
 * @param src Raw line container.
 * @param i0 Start word index.
 * @param i1 End word index.
 * @return OCR line built from the selected words.
 */
core::application::importing::transaction::internal::OcrLine toOcrLineFromRawWords(const RawLineLite& src, size_t i0, size_t i1) noexcept;
/**
 * @brief Splits a raw line into the main transaction row representation.
 * @param src Raw line container.
 * @param valutaX Reference currency column.
 * @param debitX Reference debit column.
 * @param creditX Reference credit column.
 * @return Main transaction row extracted from the raw line.
 */
core::application::importing::transaction::internal::TransactionMainRow splitMainRowFromRaw(const RawLineLite& src, int valutaX, int debitX, int creditX) noexcept;
/**
 * @brief Splits an OCR line into the main transaction row representation.
 * @param src OCR line input.
 * @param valutaX Reference currency column.
 * @param debitX Reference debit column.
 * @param creditX Reference credit column.
 * @return Main transaction row extracted from the OCR line.
 */
core::application::importing::transaction::internal::TransactionMainRow splitMainRowFromOcrLine(const core::application::importing::transaction::internal::OcrLine& src, int valutaX, int debitX, int creditX) noexcept;

}

// Intentionally no legacy core::parser aliases here; parser sources now use the current namespaces directly.
