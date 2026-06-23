/**
 * @file core/src/application/analysis/presentation/TableAnalysis.h
 * @brief Declares tabular analysis result projection helpers.
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
 * @brief Computes table rows for transactions matched by one analysis.
 */
core::ports::analysis::AnalysisResult
computeTableAnalysis(const core::domain::Analysis& analysis,
                     const core::domain::catalog::WorkspaceCatalog& state,
                     const AnalysisFilter& filter);

} // namespace core::application::analysis
