/**
 * @file core/include/core/ports/usecases/analysis/IAnalysisRunner.h
 * @brief Analysis use-case port consumed by UI workflows.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <string>
#include <vector>

namespace core::ports::analysis {

class IAnalysisRunner {
public:
    /**
     * @brief Destroys the analysis runner port.
     */
    virtual ~IAnalysisRunner() = default;

    /**
     * @brief Computes an analysis result from a workspace snapshot and request.
     * @param workspace Workspace data used as the analysis input.
     * @param request Analysis configuration and target selection.
     * @return Computed analysis result.
     */
    [[nodiscard]] virtual AnalysisResult
    runAnalysis(const core::ports::workspace::WorkspaceSnapshot& workspace,
                const AnalysisRequest& request) const = 0;

    /**
     * @brief Builds the transactions that would participate in an analysis.
     * @param workspace Workspace data used for filtering.
     * @param filterSpec Serialized filter specification.
     * @return Preview transactions and aggregate metrics for the filter.
     */
    [[nodiscard]] virtual AnalysisPreviewResult
    previewTransactions(const core::ports::workspace::WorkspaceSnapshot& workspace,
                        const std::string& filterSpec) const = 0;

    [[nodiscard]] virtual AnalysisFilterSelection filterSelectionFromFields(
        const std::string& dateField,
        const std::string& dateMode,
        const std::string& year,
        const std::string& dateFrom,
        const std::string& dateTo,
        const std::vector<std::string>& propertyIds,
        const std::vector<std::string>& contractTypes,
        const std::string& allocatableMode) const = 0;

    [[nodiscard]] virtual std::string
    buildAnalysisConfigJson(const AnalysisConfigInput& input) const = 0;

    [[nodiscard]] virtual std::string buildAnalysisAdjustmentsJson(
        const std::vector<AnalysisAdjustmentTransactionInput>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent) const = 0;

    virtual void applyAnalysisPreviewOverrides(
        core::ports::workspace::WorkspaceSnapshot& workspace,
        const std::string& analysisId,
        bool includeCalculationAdjustments,
        const std::string& adjustmentsJson) const = 0;

    [[nodiscard]] virtual AnalysisTableState projectTableState(
        const AnalysisResult& result,
        const std::string& adjustmentsJson,
        bool includeCalculationAdjustments,
        const std::string& unassignedLabel) const = 0;

    [[nodiscard]] virtual std::vector<std::string> contractTypes(
        const core::ports::workspace::WorkspaceSnapshot& workspace) const = 0;
};

} // namespace core::ports::analysis
