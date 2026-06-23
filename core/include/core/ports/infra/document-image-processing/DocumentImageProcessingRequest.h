/**
 * @file core/include/core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h
 * @brief Request types for document image processing operations.
 */
#pragma once

#include "core/ports/infra/document-image-processing/DocumentImageProcessingTypes.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace core::ports::document_image_processing {

struct DenoiseRequest {
    std::filesystem::path imagePath;
    enum class Method { Median, Gaussian, Bilateral } method = Method::Median;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct MaskRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    std::vector<Rect> textElements;
    enum class Mode { Whiteout, Inpaint } mode = Mode::Whiteout;
    std::string tesseractTsv;
    bool usePoppler = true;
    bool useTesseract = false;
    bool useMorphology = true;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct DetectRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    enum class DetectKind { Tables, Cells, TextBlocks } kind = DetectKind::Tables;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct CropRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    Rect bbox;
    enum class OutputFormat { Png, Jpg } outputFormat = OutputFormat::Png;
    int jpegQuality = 92;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

} // namespace core::ports::document_image_processing
