/**
 * @file infra/text-recognition/include/text-recognition/TesseractCore.h
 * @brief Declares the Tesseract implementation helpers used by the import workflow.
 */

#pragma once

#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "core/ports/infra/text-recognition/TextRecognitionTypes.h"
#include <string>
#include <vector>
#include <memory>

namespace core::ports::diagnostics { class IDiagnostics; }

class TesseractCore {
public:
    /**
     * @brief Extracts recognized text and word-level OCR data from image bytes.
     * @param data The encoded image bytes.
     * @param tessdataPath The tessdata directory path or a fallback location hint.
     * @param recognition The OCR recognition settings.
     * @param debugger Optional debugger used for trace output.
     * @return A pair containing recognized text and the extracted word list.
     */
    static std::pair<core::ports::text_recognition::Text, std::vector<core::ports::text_recognition::Word>> extractFromBytes(
        const std::vector<uint8_t>& data,
        const std::string& tessdataPath,
        const core::ports::text_recognition::Settings& recognition,
        std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger = nullptr);
};
