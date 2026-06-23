/**
 * @file core/include/core/application/import/statement/StatementParserTypes.h
 * @brief Declares helper types used by statement parsing heuristics.
 */

#pragma once

#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace core::application::importing::statement::internal {

/**
 * @brief Represents a merged raw OCR line before it is turned into parser lines.
 */
struct RawLine {
    int cy = 0;
    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

/**
 * @brief Represents the inferred transaction column layout for a page.
 */
struct ColumnModel {
    int valutaX = -1;
    int debitX = -1;
    int creditX = -1;
    int valutaCol = -1;
    int debitCol = -1;
    int creditCol = -1;
    bool hasValuta() const { return valutaX >= 0; }
    bool hasDebit() const { return debitX >= 0; }
    bool hasCredit() const { return creditX >= 0; }
};

/**
 * @brief Holds the current header-detection state for one statement page.
 */
struct HeaderAnalysis {
    int preHeaderBottomY = -1;
    bool headerFound = false;
    int headerBottomY = -1;
    int headerMarginPx = 8;
    int pageMaxY = -1;
};

}
