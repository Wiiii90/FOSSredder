/**
 * @file core/include/core/ports/infra/text-recognition/TextRecognitionResult.h
 * @brief Result types for text recognition operations.
 */
#pragma once

#include "core/ports/infra/text-recognition/TextRecognitionTypes.h"

#include <string>
#include <vector>

namespace core::ports::text_recognition {

struct ExtractResult {
    std::string text;
    std::string tsv;
    std::vector<Word> words;
    std::vector<Table> tables;
};

} // namespace core::ports::text_recognition
