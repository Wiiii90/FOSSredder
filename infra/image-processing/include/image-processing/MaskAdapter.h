/**
 * @file infra/image-processing/include/image-processing/MaskAdapter.h
 * @brief Declares the OpenCV masking adapter used by the import workflow.
 */

#pragma once

#include "core/ports/infra/image-processing/ImageProcessingRequest.h"
#include "core/ports/infra/image-processing/ImageProcessingResult.h"
#include <memory>
#include <opencv2/opencv.hpp>

namespace core::ports::diagnostics { class IDiagnostics; }

namespace opencv {

namespace ports = core::ports::image_processing;

class MaskAdapter {
public:
    /**
     * @brief Builds a whiteout mask for the requested image region.
     * @param req The mask request containing the source image and crop geometry.
     * @param debugger Optional debugger used for trace output.
     * @return The mask result with binary mask bytes and masked image bytes.
     */
    static ports::MaskResult mask(const ports::MaskRequest& req, std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger = nullptr);
    /**
     * @brief Creates a binary line mask for text or table detection.
     * @param gray The grayscale source image.
     * @param horizontal True for a horizontal line mask, false for a vertical one.
     * @return The generated mask image.
     */
    static cv::Mat makeLineMask(const cv::Mat& gray, bool horizontal);
};

} // namespace opencv
