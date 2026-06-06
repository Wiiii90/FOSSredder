/**
 * @file core/include/core/ports/infra/text-recognition/TextRecognitionRequest.h
 * @brief Request types for text recognition operations.
 */
#pragma once

#include "core/ports/infra/text-recognition/TextRecognitionTypes.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace core::ports::text_recognition {

struct ExtractRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path tessdataPath;
    Settings recognition;
    std::vector<Cell> cells;
    enum class Kind { PlainText, Table } kind = Kind::PlainText;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

} // namespace core::ports::text_recognition
