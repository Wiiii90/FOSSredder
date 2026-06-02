/**
 * @file core/include/core/ports/analysis/IAnalysisRunner.h
 * @brief Analysis use-case port consumed by UI workflows.
 */

#pragma once

#include "core/ports/analysis/AnalysisRequest.h"
#include "core/ports/analysis/AnalysisResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <string>
#include <vector>

namespace core::ports::analysis {

class IAnalysisRunner {
public:
    virtual ~IAnalysisRunner() = default;

    [[nodiscard]] virtual AnalysisResult
    runAnalysis(const core::ports::workspace::WorkspaceSnapshot& workspace,
                const AnalysisRequest& request) const = 0;

    [[nodiscard]] virtual std::vector<AnalysisPreviewTransaction>
    previewTransactions(const core::ports::workspace::WorkspaceSnapshot& workspace,
                        const std::string& filterSpec) const = 0;
};

} // namespace core::ports::analysis
