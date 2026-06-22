/**
 * @file core/src/application/analysis/presentation/PlotAnalysis.h
 * @brief Declares plot-oriented analysis aggregation helpers.
 */

#pragma once

#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"

namespace core::application::analysis {
class AnalysisFilter;
}

namespace core::application::analysis {

/**
 * @brief Computes plot metrics and table rows for one analysis definition.
 */
core::ports::analysis::AnalysisResult
computePlotAnalysis(const core::domain::Analysis& analysis,
                    const core::domain::catalog::WorkspaceCatalog& state,
                    const AnalysisFilter& filter);

} // namespace core::application::analysis
