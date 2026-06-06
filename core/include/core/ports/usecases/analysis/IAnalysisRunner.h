/**
 * @file core/include/core/ports/usecases/analysis/IAnalysisRunner.h
 * @brief Analysis use-case port consumed by UI workflows.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <optional>
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
     * @param filter Filter selection.
     * @return Preview transactions and aggregate metrics for the filter.
     */
    [[nodiscard]] virtual AnalysisPreviewResult
    previewTransactions(const core::ports::workspace::WorkspaceSnapshot& workspace,
                        const AnalysisFilterSelection& filter) const = 0;

    /**
     * @brief Builds a normalized filter selection from raw field values.
     * @param dateField Date field key.
     * @param dateMode Date mode key.
     * @param year Year text.
     * @param dateFrom Start date text.
     * @param dateTo End date text.
     * @param propertyIds Selected property ids.
     * @param contractTypes Selected contract types.
     * @param allocatableMode Allocatable mode key.
     * @return Normalized filter selection.
     */
    [[nodiscard]] virtual AnalysisFilterSelection filterSelectionFromFields(
        const std::string& dateField,
        const std::string& dateMode,
        const std::string& year,
        const std::string& dateFrom,
        const std::string& dateTo,
        const std::vector<std::string>& propertyIds,
        const std::vector<std::string>& contractTypes,
        const std::string& allocatableMode) const = 0;

    /**
     * @brief Parses a serialized analysis filter specification.
     * @param filterSpec Serialized filter specification.
     * @return Parsed filter selection.
     */
    [[nodiscard]] virtual AnalysisFilterSelection
    parseFilterSpec(const std::string& filterSpec) const = 0;

    /**
     * @brief Serializes a normalized analysis filter selection.
     * @param selection Filter selection.
     * @return Serialized filter specification.
     */
    [[nodiscard]] virtual std::string
    buildFilterSpec(const AnalysisFilterSelection& selection) const = 0;

    /**
     * @brief Builds serialized analysis configuration from typed input.
     * @param input Analysis config input.
     * @return Serialized analysis configuration.
     */
    [[nodiscard]] virtual std::string
    buildAnalysisConfigJson(const AnalysisConfigInput& input) const = 0;

    /**
     * @brief Builds adjustment amounts for selected transactions.
     * @param transactions Transaction inputs containing id and base amount.
     * @param selectedTransactionIds Selected transaction ids.
     * @param taxPercent Percentage used to compute adjusted amounts.
     * @return Adjustment amount pairs keyed by transaction id.
     */
    [[nodiscard]] virtual AnalysisAdjustmentAmounts buildAnalysisAdjustments(
        const std::vector<AnalysisAdjustmentTransactionInput>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent) const = 0;

    /**
     * @brief Parses user-entered analysis percentage text.
     * @param text Percent text, optionally with percent sign or comma decimal.
     * @return Parsed percent value when valid.
     */
    [[nodiscard]] virtual std::optional<double>
    parseAnalysisPercentText(const std::string& text) const = 0;

    /**
     * @brief Applies preview-only analysis fields to a workspace snapshot.
     * @param workspace Workspace snapshot to mutate for preview.
     * @param analysisId Target analysis id.
     * @param includeCalculationAdjustments Whether adjustments are included.
     * @param adjustments Adjustment amount pairs keyed by transaction id.
     */
    virtual void applyAnalysisPreviewOverrides(
        core::ports::workspace::WorkspaceSnapshot& workspace,
        const std::string& analysisId,
        bool includeCalculationAdjustments,
        const AnalysisAdjustmentAmounts& adjustments) const = 0;

    /**
     * @brief Projects analysis result table state for UI consumption.
     * @param result Analysis result.
     * @param adjustments Adjustment amount pairs keyed by transaction id.
     * @param includeCalculationAdjustments Whether adjustments are included.
     * @param unassignedLabel Label used for unassigned contract type.
     * @return Projected table state.
     */
    [[nodiscard]] virtual AnalysisTableState projectTableState(
        const AnalysisResult& result,
        const AnalysisAdjustmentAmounts& adjustments,
        bool includeCalculationAdjustments,
        const std::string& unassignedLabel) const = 0;

    /**
     * @brief Returns available contract types for analysis filters.
     * @param workspace Workspace snapshot used as source.
     * @return Contract type values.
     */
    [[nodiscard]] virtual std::vector<std::string> contractTypes(
        const core::ports::workspace::WorkspaceSnapshot& workspace) const = 0;
};

} // namespace core::ports::analysis
