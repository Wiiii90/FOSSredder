/**
 * @file core/include/core/application/import/transaction/ParserTypes.h
 * @brief Declares the OCR transaction parser model types.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace core::application::importing::transaction::internal {

/**
 * @brief Represents one OCR line with its bounding box and recognized text.
 */
struct OcrLine {
    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

/**
 * @brief Represents one OCR cell within a transaction table row.
 */
struct OcrCell {
    OcrLine line;
    bool empty() const { return line.text.empty(); }
};

/**
 * @brief Represents the parsed main row of a transaction block.
 */
struct TransactionMainRow {
    OcrCell left;
    OcrCell valuta;
    OcrCell debit;
    OcrCell credit;
};

/**
 * @brief Groups the main row and detail rows of one parsed transaction block.
 */
struct TransactionBlock {
    std::string bookingDateGroup;
    TransactionMainRow main;
    std::vector<OcrLine> detailLines;
};

}

namespace core {
namespace parser {
using OcrCell = application::importing::transaction::internal::OcrCell;
using OcrLine = application::importing::transaction::internal::OcrLine;
using TransactionBlock = application::importing::transaction::internal::TransactionBlock;
using TransactionMainRow = application::importing::transaction::internal::TransactionMainRow;
}
}
