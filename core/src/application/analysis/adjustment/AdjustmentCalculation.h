#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"

namespace core::application::analysis {
class AnalysisFilter;
}

namespace core::application::analysis {

core::ports::analysis::AnalysisResult
computeAdjustmentAnalysis(const core::domain::Analysis &analysis,
                          const core::domain::catalog::WorkspaceCatalog &state,
                          const AnalysisFilter &filter);

} // namespace core::application::analysis
