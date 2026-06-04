/**
 * @file core/include/core/application/analysis/AnalysisWorkflowSupport.h
 * @brief Shared analysis request/config helpers for UI and application layers.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <string>
#include <utility>
#include <vector>

namespace core::application::analysis {

struct AnalysisConfigInput {
    std::string type;
    std::string plotType;
    std::string plotMeasure;
    std::vector<std::string> propertyIds;
    std::vector<std::string> contractTypes;
    double taxPercent = 0.0;
};

struct AnalysisAdjustmentTransactionInput {
    std::string id;
    double amount = 0.0;
};

[[nodiscard]] std::string buildAnalysisConfigJson(const AnalysisConfigInput& input);

[[nodiscard]] core::ports::analysis::AnalysisFilterSelection
filterSelectionFromFields(const std::string& dateField,
                          const std::string& dateMode,
                          const std::string& year,
                          const std::string& dateFrom,
                          const std::string& dateTo,
                          const std::vector<std::string>& propertyIds,
                          const std::vector<std::string>& contractTypes,
                          const std::string& allocatableMode);

[[nodiscard]] std::vector<std::pair<std::string, double>>
parseAnalysisAdjustmentsJson(const std::string& adjustmentsJson);

[[nodiscard]] std::string
buildAnalysisAdjustmentsJson(
    const std::vector<AnalysisAdjustmentTransactionInput>& transactions,
    const std::vector<std::string>& selectedTransactionIds,
    double taxPercent);

void applyAnalysisPreviewOverrides(
    core::ports::workspace::WorkspaceSnapshot& workspace,
    const std::string& analysisId,
    bool includeCalculationAdjustments,
    const std::vector<std::pair<std::string, double>>& adjustments);

[[nodiscard]] std::vector<std::string>
contractTypesFromSnapshot(const core::ports::workspace::WorkspaceSnapshot& workspace);

} // namespace core::application::analysis
