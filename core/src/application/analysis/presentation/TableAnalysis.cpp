#include "TableAnalysis.h"
#include "../internal/AnalysisFilter.h"
#include "core/application/analysis/AnalysisKeys.h"

namespace core::application::analysis {

core::ports::analysis::AnalysisResult
computeTableAnalysis(const core::domain::Analysis& analysis,
                     const core::domain::catalog::WorkspaceCatalog& state,
                     const AnalysisFilter& filter)
{
    (void)analysis;
    core::ports::analysis::AnalysisResult result;

    for (const auto& transaction : collectAnalysisTransactions(state, filter)) {
        result.table.push_back({transaction->bookingDate(), transaction->name(), std::to_string(transaction->amount())});
    }

    result.metrics[std::string(core::application::analysis::keys::metricKeys::kRowCount)] = static_cast<double>(result.table.size());
    return result;
}

} // namespace core::application::analysis
