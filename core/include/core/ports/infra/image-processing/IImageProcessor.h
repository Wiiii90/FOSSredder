/**
 * @file core/include/core/ports/infra/image-processing/IImageProcessor.h
 * @brief Image processing port for import workflows.
 */
#pragma once

#include "core/ports/infra/image-processing/ImageProcessingRequest.h"
#include "core/ports/infra/image-processing/ImageProcessingResult.h"

namespace core::ports::image_processing {

/**
 * @brief Processes images for the import pipeline.
 */
class IImageProcessor {
public:
    virtual ~IImageProcessor() = default;

    /**
     * @brief Removes noise from an image.
     * @param req Denoising request.
     * @return The denoised image result.
     */
    virtual DenoiseResult denoise(const DenoiseRequest& req) const = 0;

    /**
     * @brief Applies a mask to an image.
     * @param req Masking request.
     * @return The masking result.
     */
    virtual MaskResult mask(const MaskRequest& req) const = 0;

    /**
     * @brief Detects table or text regions in an image.
     * @param req Detection request.
     * @return The detection result.
     */
    virtual DetectResult detect(const DetectRequest& req) const = 0;

    /**
     * @brief Crops an image into one or more regions.
     * @param req Cropping request.
     * @return The cropping result.
     */
    virtual CropResult crop(const CropRequest& req) const = 0;
};

} // namespace core::ports::image_processing
