/**
 * @file core/include/core/ports/infra/image-processing/ImageProcessingResult.h
 * @brief Result types for image processing operations.
 */
#pragma once

#include "core/ports/infra/image-processing/ImageProcessingTypes.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace core::ports::image_processing {

struct DenoiseResult {
    std::filesystem::path denoisedImagePath;
};

struct MaskResult {
    std::filesystem::path maskedImagePath;
    std::filesystem::path maskImagePath;
    std::vector<uint8_t> maskedImageBytes;
    std::vector<uint8_t> maskImageBytes;
};

struct DetectResult {
    Table table;
    bool detected = false;
    std::vector<Rect> textBlocks;
};

struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
    std::vector<std::vector<uint8_t>> croppedImageBytes;
};

} // namespace core::ports::image_processing
