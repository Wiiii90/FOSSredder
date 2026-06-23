/**
 * @file core/include/core/domain/policies/DraftMatchingPolicy.h
 * @brief Declares draft matching policy helpers for import-time bank statement inference.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/domain/catalog/WorkspaceCatalog.h"

namespace core::domain::policies::matching {

/**
 * @brief Trims whitespace from raw text.
 * @param value Raw text.
 * @return Trimmed text or an empty string.
 */
std::string trim(std::string value);
/**
 * @brief Compares two pieces of draft text after normalization.
 * @param left Left text.
 * @param right Right text.
 * @return `true` when the normalized texts are equal.
 */
bool matchesDraftText(const std::string& left, const std::string& right);
/**
 * @brief Splits text into non-empty normalized lines.
 * @param text Raw multiline text.
 * @return Normalized lines.
 */
std::vector<std::string> splitLines(const std::string& text);
/**
 * @brief Joins non-empty lines with a separator.
 * @param values Input lines.
 * @param separator Separator string.
 * @return Joined text.
 */
std::string joinNonEmptyLines(const std::vector<std::string>& values, const std::string& separator);
/**
 * @brief Checks whether a line looks like a metadata boundary.
 * @param line Raw line text.
 * @return `true` when the line resembles metadata.
 */
bool lineLooksMetadataBoundary(const std::string& line);
/**
 * @brief Checks whether a line should stop actor extraction.
 * @param line Raw line text.
 * @return `true` when the line indicates that actor text ended.
 */
bool lineLooksActorStopSignal(const std::string& line);
/**
 * @brief Checks whether a line can continue actor extraction.
 * @param line Raw line text.
 * @return `true` when the line looks like actor continuation text.
 */
bool lineLooksActorContinuation(const std::string& line);
/**
 * @brief Returns the first meaningful line of text.
 * @param text Raw multiline text.
 * @return The first non-empty normalized line.
 */
std::string firstMeaningfulLine(const std::string& text);
/**
 * @brief Extracts a leading phrase from text.
 * @param text Raw text.
 * @param maxTokens Maximum number of tokens to keep.
 * @return Leading phrase text.
 */
std::string leadingPhrase(const std::string& text, std::size_t maxTokens);
/**
 * @brief Normalizes draft text for matching.
 * @param text Raw text.
 * @return Normalized comparison text.
 */
std::string normalizeText(std::string text);
/**
 * @brief Splits normalized text into tokens.
 * @param text Raw text.
 * @return Tokens extracted from the text.
 */
std::vector<std::string> tokens(const std::string& text);
/**
 * @brief Returns a leading slice of the normalized text.
 * @param text Raw text.
 * @param maxTokens Maximum number of tokens to keep.
 * @return Leading normalized text.
 */
std::string leadingText(const std::string& text, std::size_t maxTokens);
/**
 * @brief Checks whether a token is useful for matching.
 * @param token Raw token.
 * @return `true` when the token can contribute to matching.
 */
bool tokenIsMatchable(const std::string& token);
/**
 * @brief Scores token overlap between two token lists.
 * @param sourceTokens Candidate tokens.
 * @param targetTokens Query tokens.
 * @return Overlap score.
 */
int tokenOverlapScore(const std::vector<std::string>& sourceTokens,
                      const std::vector<std::string>& targetTokens);
/**
 * @brief Computes a score weight from alias hit count.
 * @param hitCount Alias hit count.
 * @return Weight factor.
 */
double aliasHitWeight(int hitCount);
/**
 * @brief Computes a score weight from alias recency.
 * @param lastUsedAt Last-used timestamp.
 * @return Weight factor.
 */
double aliasRecencyWeight(const std::string& lastUsedAt);
/**
 * @brief Normalizes metadata into signal text.
 * @param metadata Raw metadata text.
 * @return Signal text derived from metadata.
 */
std::string metadataSignalText(const std::string& metadata);
/**
 * @brief Extracts reference-like aliases from metadata.
 * @param metadata Raw metadata text.
 * @return Candidate reference aliases.
 */
std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata);
/**
 * @brief Collects known contract types from the workspace state.
 * @param state Workspace snapshot.
 * @return Known contract type labels.
 */
std::vector<std::string> knownContractTypes(const core::domain::catalog::WorkspaceCatalog& state);
/**
 * @brief Extracts a contract type hint from draft text.
 * @param state Workspace snapshot.
 * @param signalText Combined signal text.
 * @param lines Draft lines.
 * @return Best matching contract type or an empty string.
 */
std::string extractTypeText(const core::domain::catalog::WorkspaceCatalog& state,
                            const std::string& signalText,
                            const std::vector<std::string>& lines);
/**
 * @brief Extracts an actor hint from draft text.
 * @param lines Draft lines.
 * @param typeText Already resolved type text.
 * @return Best matching actor text or an empty string.
 */
std::string extractActorText(const std::vector<std::string>& lines, const std::string& typeText);

} // namespace core::domain::policies::matching
