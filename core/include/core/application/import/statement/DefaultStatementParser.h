/**
 * @file core/include/core/application/import/statement/DefaultStatementParser.h
 * @brief Declares the public default statement parser used by the import pipeline.
 */

#pragma once

#include "core/ports/infra/image-processing/ImageProcessingResult.h"
#include "core/ports/infra/image-processing/IImageProcessor.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"
#include "core/application/import/draft/TransactionDraft.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::application::importing::statement {

class DefaultStatementParser {
public:
    /**
     * @brief Holds the parser output for one imported statement page.
     */
    struct ParseResult {
        std::vector<core::application::importing::draft::TransactionDraft> transactions;
        std::vector<std::string> debugLines;
        std::unordered_map<std::string, std::vector<uint8_t>> artifacts;
        std::string lastBookingDate;
        int nextTransactionIndex = 1;
    };

    /**
     * @brief Parses an OCR-backed table into transaction drafts and artifacts.
     * @param table Detected table structure from the page image.
     * @param ocr OCR result for the page or table region.
     * @param pageCropImagePath File path to the cropped page image.
     * @param opencv Image processing adapter used for image extraction helpers.
     * @param pageCropImageBytes Raw bytes of the cropped page image.
     * @param initialBookingDate Booking date seed carried into parsing.
     * @param initialTransactionIndex Initial transaction counter value.
     * @return Parse result containing generated drafts, artifacts, and parser metadata.
     */
    static ParseResult parse(const core::ports::image_processing::Table& table,
                             const core::ports::text_recognition::ExtractResult& ocr,
                             const std::string& pageCropImagePath,
                             std::shared_ptr<core::ports::image_processing::IImageProcessor> opencv,
                             const std::vector<uint8_t>& pageCropImageBytes,
                             std::string initialBookingDate = {},
                             int initialTransactionIndex = 1);
};

}
