/**
 * @file core/include/core/application/import/internal/ParserConfig.h
 * @brief Declares runtime configuration values for the import parsers.
 */

#pragma once

namespace core::application::importing::internal {

/**
 * @brief Runtime configuration for parser heuristics and thresholds.
 */
struct ParserConfig {
    int amountNearValutaBandPx = 120;
    int valutaNeighborExpandPx = 140;
    int leftDescriptiveOffsetPx = 200;
    int tokenNearMergeBandPx = 220;
    int maxPhraseTokens = 6;
    int tokenNearBandForMainRow = 100;
    int groupMergeMaxGapPx = 8;
    int headerScanLines = 12;
    int headerMarginPx = 8;
    int orphanAttachMaxGapPx = 80;
    int orphanAcceptXOffsetPx = 40;
};

/**
 * @brief Shared parser configuration instance used by import parsing helpers.
 */
extern ParserConfig parserConfig;

}
