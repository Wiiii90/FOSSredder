/**
 * @file core/include/core/ports/analysis-image-renderer/IAnalysisImageRenderer.h
 * @brief Declares the analysis image rendering port used by the export layer.
 */

#pragma once

#include "core/ports/analysis/AnalysisResult.h"

#include <filesystem>
#include <string>

namespace core::ports::analysis_image_renderer {

class IAnalysisImageRenderer {
public:
    virtual ~IAnalysisImageRenderer() = default;

    virtual bool writeAnalysisImage(const std::filesystem::path& outputPath,
                                    const std::string& title,
                                    const core::ports::analysis::AnalysisResult& result) const = 0;
};

} // namespace core::ports::analysis_image_renderer
