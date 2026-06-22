/**
 * @file infra/analysis-rendering/include/analysis-rendering/OpenCvAnalysisRendererAdapter.h
 * @brief Declares the OpenCV-based analysis image renderer adapter.
 */

#pragma once

#include "core/ports/infra/analysis-rendering/IAnalysisRenderer.h"

#include <memory>

namespace core::ports::diagnostics {
class IErrorReporter;
}

namespace infra::analysis_rendering {

class OpenCvAnalysisRendererAdapter final : public core::ports::analysis_rendering::IAnalysisRenderer {
public:
    explicit OpenCvAnalysisRendererAdapter(std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);

    bool renderToImage(const std::filesystem::path& outputPath,
                            const std::string& title,
                            const core::ports::analysis::AnalysisResult& result) const override;

private:
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

} // namespace infra::analysis_rendering
