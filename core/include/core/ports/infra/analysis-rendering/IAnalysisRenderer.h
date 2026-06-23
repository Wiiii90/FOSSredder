/**
 * @file core/include/core/ports/infra/analysis-rendering/IAnalysisRenderer.h
 * @brief Declares the analysis rendering port used by the export layer.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisResult.h"

#include <filesystem>
#include <string>

namespace core::ports::analysis_rendering {

/**
 * @brief Renders analysis results to image artifacts.
 */
class IAnalysisRenderer {
public:
    /**
     * @brief Destroys the analysis renderer port.
     */
    virtual ~IAnalysisRenderer() = default;

    /**
     * @brief Renders one analysis artifact to an image file.
     * @param outputPath Target image path.
     * @param title Display title for the rendered analysis.
     * @param result Analysis result to render.
     * @return True when the image was written successfully.
     */
    virtual bool renderToImage(const std::filesystem::path& outputPath,
                               const std::string& title,
                               const core::ports::analysis::AnalysisResult& result) const = 0;
};

} // namespace core::ports::analysis_rendering
