/**
 * @file core/include/core/application/import/statement/StatementParseHelpers.h
 * @brief Declares helpers for the statement parsing workflow.
 */

#pragma once

#include "core/application/import/statement/DefaultStatementParser.h"
#include "core/application/import/transaction/TransactionParserTypes.h"
#include "core/application/import/statement/StatementParserTypes.h"
#include "core/application/import/internal/ParserHelpers.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h"
#include "core/ports/infra/document-image-processing/IDocumentImageProcessor.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"
#include "core/application/import/draft/TransactionDraft.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace core::application::importing::statement::internal {

/**
 * @brief Converts a raw OCR line into a parser line model.
 * @param line Raw OCR line input.
 * @return Normalized parser line representation.
 */
core::application::importing::transaction::internal::OcrLine rawToOcrLine(const core::application::importing::statement::internal::RawLine& line);
/**
 * @brief Checks whether a raw line likely contains a main transaction row.
 * @param line Raw OCR line input.
 * @param cols Current column model.
 * @return True when the geometry suggests a main row.
 */
bool isLikelyTransactionMainRowGeom(const core::application::importing::statement::internal::RawLine& line,
                                    const core::application::importing::statement::internal::ColumnModel& cols);
/**
 * @brief Checks whether an OCR line likely represents a transaction header row.
 * @param line OCR line input.
 * @param cols Current column model.
 * @return True when the line resembles a transaction header.
 */
bool isLikelyTransactionHeaderLine(const core::application::importing::transaction::internal::OcrLine& line,
                                   const core::application::importing::statement::internal::ColumnModel& cols);
/**
 * @brief Detects the most likely header region in the supplied OCR lines.
 * @param lines OCR lines extracted from the page.
 * @param scanLines Number of leading lines to inspect.
 * @return Header region position and detection flag.
 */
std::pair<int, bool> detectHeaderRegion(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines, size_t scanLines);
/**
 * @brief Finds a default booking date near the top of the parsed page.
 * @param lines OCR lines extracted from the page.
 * @param scanLines Number of leading lines to inspect.
 * @return Booking date when one can be inferred, otherwise an empty optional.
 */
std::optional<std::string> findDefaultBookingDate(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines, size_t scanLines) noexcept;
/**
 * @brief Extracts a booking date from a header line.
 * @param line Header text to inspect.
 * @return Parsed booking date when the line contains one.
 */
std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept;
/**
 * @brief Detects whether a page is effectively empty before parsing continues.
 * @param lines OCR lines extracted from the page.
 * @param outDebug Output debug message for diagnostics.
 * @return True when the page is considered empty.
 */
bool detectEarlyEmptyPage(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines, std::string& outDebug) noexcept;
/**
 * @brief Merges nearby raw lines using the parser's selective grouping heuristic.
 * @param lines Raw line candidates.
 * @param maxGapPx Maximum vertical gap for merging.
 * @param seedCols Seed column model used for the merge.
 * @return Merged raw line spans.
 */
std::vector<core::application::importing::internal::RawLineLite> selectiveGroupMergeLinesRaw(const std::vector<core::application::importing::internal::RawLineLite>& lines,
                                                                      int maxGapPx,
                                                                      const core::application::importing::internal::ColumnGuess& seedCols);
/**
 * @brief Attempts to start a transaction block from a vertically aligned line.
 * @param lines OCR lines extracted from the page.
 * @param li Current line index.
 * @param cols Current column guess.
 * @return Main row and next index when the line can start a block.
 */
std::optional<std::pair<core::application::importing::transaction::internal::TransactionMainRow, int>> tryVerticalStart(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines,
                                                                   size_t li,
                                                                   const core::application::importing::internal::ColumnGuess& cols) noexcept;
/**
 * @brief Attempts to start a transaction block from a combined text pattern.
 * @param lines OCR lines extracted from the page.
 * @param li Current line index.
 * @param cols Current column guess.
 * @return Main row and next index when the line can start a block.
 */
std::optional<std::pair<core::application::importing::transaction::internal::TransactionMainRow, int>> tryCombinedStart(const std::vector<core::application::importing::transaction::internal::OcrLine>& lines,
                                                                   size_t li,
                                                                   const core::application::importing::internal::ColumnGuess& cols) noexcept;
/**
 * @brief Appends a detail line to the current transaction block.
 * @param cur Current block being assembled.
 * @param line Detail OCR line to append.
 * @param cols Current column guess.
 * @param debugOut Optional debug output sink.
 */
void appendDetailLine(core::application::importing::transaction::internal::TransactionBlock& cur,
                      const core::application::importing::transaction::internal::OcrLine& line,
                      const core::application::importing::internal::ColumnGuess& cols,
                      std::vector<std::string>* debugOut = nullptr) noexcept;
/**
 * @brief Converts one OCR line into a transaction main row.
 * @param line OCR line input.
 * @param cols Current column guess.
 * @param isGeom True when the line was selected by geometry.
 * @param debugOut Debug output sink.
 * @return Parsed main transaction row.
 */
core::application::importing::transaction::internal::TransactionMainRow handleMainRow(const core::application::importing::transaction::internal::OcrLine& line,
                                 const core::application::importing::internal::ColumnGuess& cols,
                                 bool isGeom,
                                 std::vector<std::string>& debugOut) noexcept;
/**
 * @brief Attaches orphan OCR lines to existing transaction blocks.
 * @param blocks Transaction blocks to extend.
 * @param orphans OCR lines not assigned during initial parsing.
 * @param maxGapPx Maximum gap used for attachment.
 * @param valutaX Reference x-coordinate for currency alignment.
 * @param debugOut Optional debug output sink.
 */
void attachOrphansToBlocks(std::vector<core::application::importing::transaction::internal::TransactionBlock>& blocks,
                           const std::vector<core::application::importing::transaction::internal::OcrLine>& orphans,
                           int maxGapPx,
                           int valutaX,
                           std::vector<std::string>* debugOut = nullptr) noexcept;
/**
 * @brief Analyzes the page header window and updates parser output state.
 * @param ocrLines OCR lines extracted from the page.
 * @param lines Raw lines extracted from the page.
 * @param seedCols Seed column model used for the analysis.
 * @param out Parser result that receives the analysis output.
 * @return Header analysis summary.
 */
HeaderAnalysis analyzeHeaderWindow(const std::vector<core::application::importing::transaction::internal::OcrLine>& ocrLines,
                                   const std::vector<core::application::importing::statement::internal::RawLine>& lines,
                                   const core::application::importing::statement::internal::ColumnModel& seedCols,
                                   core::application::importing::statement::DefaultStatementParser::ParseResult& out);
/**
 * @brief Reprocesses orphan transaction blocks against the current booking date and column model.
 * @param blocks Parsed blocks to repair.
 * @param orphanLines Remaining OCR lines.
 * @param cols Current column model.
 * @param currentBookingDate Current booking date seed.
 * @param out Parser result that receives recovered output.
 */
void rescueOrphanBlocks(std::vector<core::application::importing::transaction::internal::TransactionBlock>& blocks,
                        const std::vector<core::application::importing::transaction::internal::OcrLine>& orphanLines,
                        const core::application::importing::statement::internal::ColumnModel& cols,
                        const std::string& currentBookingDate,
                        core::application::importing::statement::DefaultStatementParser::ParseResult& out);
/**
 * @brief Appends a page-level summary to the parser result.
 * @param lines Raw page lines.
 * @param headerBottomY Header boundary y-coordinate.
 * @param blocks Parsed transaction blocks.
 * @param txStartLooseCount Loose transaction count at page start.
 * @param out Parser result that receives the summary.
 */
void appendPageSummary(const std::vector<core::application::importing::statement::internal::RawLine>& lines,
                       int headerBottomY,
                       const std::vector<core::application::importing::transaction::internal::TransactionBlock>& blocks,
                       int txStartLooseCount,
                       core::application::importing::statement::DefaultStatementParser::ParseResult& out);
/**
 * @brief Converts parsed transaction blocks into drafts and extraction artifacts.
 * @param blocks Parsed transaction blocks.
 * @param cols Current column model.
 * @param ocr OCR result for the page.
 * @param documentImageProcessor Document image processing adapter used for artifact generation.
 * @param pageCropImagePath File path to the cropped page image.
 * @param pageCropImageBytes Raw bytes of the cropped page image.
 * @param txIndex Running transaction index counter.
 * @param out Parser result that receives generated drafts and artifacts.
 */
void appendTransactionsFromBlocks(const std::vector<core::application::importing::transaction::internal::TransactionBlock>& blocks,
                                  const core::application::importing::statement::internal::ColumnModel& cols,
                                  const core::ports::text_recognition::ExtractResult& ocr,
                                  const std::shared_ptr<core::ports::document_image_processing::IDocumentImageProcessor>& documentImageProcessor,
                                  const std::string& pageCropImagePath,
                                  const std::vector<uint8_t>& pageCropImageBytes,
                                  int& txIndex,
                                  core::application::importing::statement::DefaultStatementParser::ParseResult& out);

}
