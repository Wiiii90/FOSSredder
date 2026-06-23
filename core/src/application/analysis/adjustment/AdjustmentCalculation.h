/**
 * @file core/src/application/analysis/adjustment/AdjustmentCalculation.h
 * @brief Declares adjustment analysis calculation helpers.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"

namespace core::application::analysis {
class AnalysisFilter;
}

namespace core::application::analysis {

/**
 * @brief Computes adjusted transaction rows for one analysis definition.
 */
core::ports::analysis::AnalysisResult
computeAdjustmentAnalysis(const core::domain::Analysis &analysis,
                          const core::domain::catalog::WorkspaceCatalog &state,
                          const AnalysisFilter &filter);

} // namespace core::application::analysis
