/**
 * @file infra/image-processing/include/image-processing/DenoiseAdapter.h
 * @brief Declares the OpenCV denoise adapter used by the import workflow.
 */

#pragma once

#include "core/ports/infra/image-processing/ImageProcessingRequest.h"
#include "core/ports/infra/image-processing/ImageProcessingResult.h"
#include <memory>

class IDebugger;

namespace opencv {

namespace ports = core::ports::image_processing;

class DenoiseAdapter {
public:
    /**
     * @brief Applies the configured denoise operation to the input image.
     * @param req The denoise request describing the source image and method.
     * @param debugger Optional debugger used for trace output.
     * @return The denoise result containing the output image path when successful.
     */
    static ports::DenoiseResult denoise(const ports::DenoiseRequest& req, std::shared_ptr<IDebugger> debugger = nullptr);
};

} // namespace opencv
