/**
 * @file infra/document-image-processing/include/document-image-processing/CropAdapter.h
 * @brief Declares the OpenCV crop adapter used by the import workflow.
 */

#pragma once

#include "core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingResult.h"
#include <memory>
#include <filesystem>
#include <opencv2/opencv.hpp>

namespace core::ports::diagnostics { class IDiagnostics; }

namespace opencv {

namespace ports = core::ports::document_image_processing;

class CropAdapter {
public:
    /**
     * @brief Crops multiple rectangles from an image file and writes the results to disk.
     * @param imagePath The source image file path.
     * @param rects The crop rectangles in source-image coordinates.
     * @param outputDir The output directory for generated crops.
     * @param fmt The desired output image format.
     * @param jpegQuality The JPEG quality to use when encoding JPG output.
     * @param debugger Optional debugger used for trace output.
     * @param filePrefix Optional prefix for generated filenames.
     * @return The list of written crop file paths.
     */
    static std::vector<std::filesystem::path> cropImages(const std::string& imagePath,
                                                         const std::vector<ports::Rect>& rects,
                                                         const std::filesystem::path& outputDir,
                                                         ports::CropRequest::OutputFormat fmt,
                                                         int jpegQuality = 92,
                                                         std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger = nullptr,
                                                         const std::string& filePrefix = "");

    /**
     * @brief Crops multiple rectangles from an in-memory image.
     * @param img The source image.
     * @param rects The crop rectangles in source-image coordinates.
     * @param outputDir The output directory for generated crops.
     * @param fmt The desired output image format.
     * @param jpegQuality The JPEG quality to use when encoding JPG output.
     * @param outBytes Optional sink for encoded crop bytes.
     * @param debugger Optional debugger used for trace output.
     * @param filePrefix Optional prefix for generated filenames.
     * @return The list of written crop file paths.
     */
    static std::vector<std::filesystem::path> cropImages(const cv::Mat& img,
                                                         const std::vector<ports::Rect>& rects,
                                                         const std::filesystem::path& outputDir,
                                                         ports::CropRequest::OutputFormat fmt,
                                                         int jpegQuality = 92,
                                                         std::vector<std::vector<uint8_t>>* outBytes = nullptr,
                                                         std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger = nullptr,
                                                         const std::string& filePrefix = "");
};

} // namespace opencv
