/**
 * @file infra/document-image-processing/include/document-image-processing/DenoiseAdapter.h
 * @brief Declares the OpenCV denoise adapter used by the import workflow.
 */

#pragma once

#include "core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingResult.h"
#include <memory>

namespace core::ports::diagnostics { class IDiagnostics; }

namespace opencv {

namespace ports = core::ports::document_image_processing;

class DenoiseAdapter {
public:
    /**
     * @brief Applies the configured denoise operation to the input image.
     * @param req The denoise request describing the source image and method.
     * @param debugger Optional debugger used for trace output.
     * @return The denoise result containing the output image path when successful.
     */
    static ports::DenoiseResult denoise(const ports::DenoiseRequest& req, std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger = nullptr);
};

} // namespace opencv
