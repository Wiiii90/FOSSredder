/**
 * @file core/src/application/analysis/adjustment/AdjustmentCalculation.cpp
 * @brief Implements adjusted-amount analysis calculations.
 */

#include "AdjustmentCalculation.h"
#include "../internal/AnalysisFilter.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"

#include "core/application/analysis/AnalysisKeys.h"

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>

namespace core::application::analysis {
namespace {

struct TaxConfig {
  bool enabled = false;
  double factor = 1.0;
};

TaxConfig parseTax(const core::domain::Analysis &analysis) {
  TaxConfig config;
  if (analysis.configJson().empty()) {
    return config;
  }

  try {
    const auto json = nlohmann::json::parse(analysis.configJson());
    if (!json.is_object()) {
      return config;
    }

    const auto strategyIt =
        json.find(core::application::analysis::keys::calculation::kStrategyKey);
    if (strategyIt == json.end() || !strategyIt->is_string()) {
      return config;
    }
    if (strategyIt->get<std::string>() !=
        core::application::analysis::keys::calculation::kStrategyTax) {
      return config;
    }

    config.enabled = true;

    const auto percentIt =
        json.find(core::application::analysis::keys::calculation::kPercentKey);
    if (percentIt == json.end() || !percentIt->is_number()) {
      return config;
    }

    const double percent = percentIt->get<double>();
    config.factor = 1.0 + (percent / 100.0);
    return config;
  } catch (...) {}

  return {};
}

} // namespace

core::ports::analysis::AnalysisResult
computeAdjustmentAnalysis(const core::domain::Analysis &analysis,
                          const core::domain::catalog::WorkspaceCatalog &state,
                          const AnalysisFilter &filter) {
  core::ports::analysis::AnalysisResult out;
  const std::unordered_map<std::string, double> adjustments =
      analysis.adjustments();
  const TaxConfig taxConfig = parseTax(analysis);

  for (const auto &transaction : collectAnalysisTransactions(state, filter)) {
    const std::string label = transaction->bookingDate().empty()
                                  ? transaction->name()
                                  : transaction->bookingDate();

    double adjustedAmount = transaction->amount();
    const auto adjustmentIt = adjustments.find(transaction->id());
    if (adjustmentIt != adjustments.end()) {
      adjustedAmount = adjustmentIt->second;
    } else if (taxConfig.enabled) {
      adjustedAmount *= taxConfig.factor;
    }

    nlohmann::json summary;
    summary[core::application::analysis::keys::resultFields::kAmountOriginal] =
        transaction->amount();
    summary[core::application::analysis::keys::resultFields::kAmountAdjusted] =
        adjustedAmount;
    if (taxConfig.enabled) {
      summary[core::application::analysis::keys::resultFields::kTaxPercent] =
          (taxConfig.factor - 1.0) * 100.0;
      summary[core::application::analysis::keys::resultFields::kTaxFactor] =
          taxConfig.factor;
    }
    summary[core::application::analysis::keys::resultFields::kTransactionId] =
        transaction->id();

    out.table.push_back({label, summary.dump()});
  }

  out.metrics[std::string(core::application::analysis::keys::metricKeys::kRows)] =
      static_cast<double>(out.table.size());
  return out;
}

} // namespace core::application::analysis
