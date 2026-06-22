/**
 * @file infra/analysis-image-renderer/include/analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h
 * @brief Declares the OpenCV-based analysis image renderer adapter.
 */

#pragma once

#include "core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h"

#include <memory>

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace infra::analysis_image_renderer {

class OpenCvAnalysisImageRendererAdapter final : public core::ports::analysis_image_renderer::IAnalysisImageRenderer {
public:
    explicit OpenCvAnalysisImageRendererAdapter(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);

    bool writeAnalysisImage(const std::filesystem::path& outputPath,
                            const std::string& title,
                            const core::ports::analysis::AnalysisResult& result) const override;

private:
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

} // namespace infra::analysis_image_renderer
