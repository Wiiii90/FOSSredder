/**
 * @file infra/analysis-image-renderer/include/analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h
 * @brief Declares the OpenCV-based analysis image renderer adapter.
 */

#pragma once

#include "core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h"

namespace infra::analysis_image_renderer {

class OpenCvAnalysisImageRendererAdapter final : public core::ports::analysis_image_renderer::IAnalysisImageRenderer {
public:
    bool writeAnalysisImage(const std::filesystem::path& outputPath,
                            const std::string& title,
                            const core::ports::analysis::AnalysisResult& result) const override;
};

} // namespace infra::analysis_image_renderer
