/**
 * @file infra/image-processing/include/image-processing/DetectAdapter.h
 * @brief Declares the OpenCV detection adapter used by the import workflow.
 */

#pragma once

#include "core/ports/infra/image-processing/ImageProcessingTypes.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

class IDebugger;

namespace opencv {

namespace ports = core::ports::image_processing;

class DetectAdapter {
public:
    /**
     * @brief Detects table structures in a source image.
     * @param img The source image.
     * @param imagePath The source image path used for debug output and tracing.
     * @param debugger Optional debugger used for trace output.
     * @return The detected tables.
     */
    static std::vector<ports::Table> detectTables(const cv::Mat& img, const std::string& imagePath, std::shared_ptr<IDebugger> debugger = nullptr);
    /**
     * @brief Detects text blocks in a source image.
     * @param img The source image.
     * @param debugger Optional debugger used for trace output.
     * @return The detected text block rectangles.
     */
    static std::vector<cv::Rect> detectTextBlocks(const cv::Mat& img, std::shared_ptr<IDebugger> debugger = nullptr);
};

} // namespace opencv
