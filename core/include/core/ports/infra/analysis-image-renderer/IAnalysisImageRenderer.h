/**
 * @file core/include/core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h
 * @brief Declares the analysis image rendering port used by the export layer.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisResult.h"

#include <filesystem>
#include <string>

namespace core::ports::analysis_image_renderer {

/**
 * @brief Renders analysis results to image artifacts.
 */
class IAnalysisImageRenderer {
public:
    /**
     * @brief Destroys the analysis image renderer port.
     */
    virtual ~IAnalysisImageRenderer() = default;

    /**
     * @brief Writes one rendered analysis image.
     * @param outputPath Target image path.
     * @param title Display title for the rendered analysis.
     * @param result Analysis result to render.
     * @return True when the image was written successfully.
     */
    virtual bool writeAnalysisImage(const std::filesystem::path& outputPath,
                                    const std::string& title,
                                    const core::ports::analysis::AnalysisResult& result) const = 0;
};

} // namespace core::ports::analysis_image_renderer
