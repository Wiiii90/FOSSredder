/**
 * @file core/include/core/ports/infra/text-recognition/ITextRecognizer.h
 * @brief Text recognition port.
 */
#pragma once

#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"

namespace core::ports::text_recognition {

/**
 * @brief Extracts recognized text from image data.
 */
class ITextRecognizer {
public:
    /**
     * @brief Destroys the text recognition port.
     */
    virtual ~ITextRecognizer() = default;

    /**
     * @brief Extracts text and geometry from an image.
     * @param req Text recognition request.
     * @return Recognized text extraction result.
     */
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

} // namespace core::ports::text_recognition
