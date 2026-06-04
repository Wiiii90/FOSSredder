/**
 * @file core/src/application/analysis/AnalysisWorkflowSupport.cpp
 * @brief Shared analysis request/config helpers for UI and application layers.
 */

#include "core/application/analysis/AnalysisWorkflowSupport.h"

#include "core/constants/analysis.h"
#include "core/ports/usecases/analysis/AnalysisRequest.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace core::application::analysis {

namespace {

std::string trim(std::string value) {
    const auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(),
                std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(),
                value.end());
    return value;
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
    return value;
}

} // namespace

std::string buildAnalysisConfigJson(const AnalysisConfigInput& input) {
    nlohmann::json config;
    const std::string normalizedType = toLower(trim(input.type));

    if (normalizedType == core::constants::analysis::kTypeCalculation) {
        config[std::string(core::constants::analysis::calculation::kStrategyKey)] =
            core::constants::analysis::calculation::kStrategyTax;
        config[std::string(core::constants::analysis::calculation::kPercentKey)] =
            input.taxPercent;
        return config.dump();
    }

    const std::string plotType = trim(input.plotType);
    config[std::string(core::constants::analysis::kPlotTypeKey)] =
        plotType.empty() ? core::constants::analysis::plotTypes::kPie
                         : plotType;

    const std::string plotMeasure = trim(input.plotMeasure);
    config[std::string(core::constants::analysis::kPlotMeasureKey)] =
        plotMeasure.empty()
            ? core::constants::analysis::plotMeasures::kTotalAmount
            : plotMeasure;

    nlohmann::json properties = nlohmann::json::array();
    for (const auto& propertyId : input.propertyIds) {
        const std::string value = trim(propertyId);
        if (!value.empty()) {
            properties.push_back(value);
        }
    }
    config[std::string(core::constants::analysis::kPropertiesKey)] =
        std::move(properties);

    nlohmann::json contractTypes = nlohmann::json::array();
    for (auto contractType : input.contractTypes) {
        contractType = toLower(trim(contractType));
        if (!contractType.empty()) {
            contractTypes.push_back(contractType);
        }
    }
    config[std::string(core::constants::analysis::kContractTypesKey)] =
        std::move(contractTypes);

    return config.dump();
}

core::ports::analysis::AnalysisFilterSelection filterSelectionFromFields(
    const std::string& dateField, const std::string& dateMode,
    const std::string& year, const std::string& dateFrom,
    const std::string& dateTo, const std::vector<std::string>& propertyIds,
    const std::vector<std::string>& contractTypes,
    const std::string& allocatableMode) {
    core::ports::analysis::AnalysisFilterSelection selection;
    selection.dateField = toLower(trim(dateField));
    selection.dateMode = toLower(trim(dateMode));
    selection.year = trim(year);
    selection.dateFrom = trim(dateFrom);
    selection.dateTo = trim(dateTo);
    selection.allocatableMode = toLower(trim(allocatableMode));
    selection.propertyIds = propertyIds;
    selection.contractTypes = contractTypes;
    for (auto& contractType : selection.contractTypes) {
        contractType = toLower(trim(contractType));
    }
    selection.propertyIds.erase(
        std::remove_if(selection.propertyIds.begin(), selection.propertyIds.end(),
                       [](const std::string& value) { return trim(value).empty(); }),
        selection.propertyIds.end());
    selection.contractTypes.erase(
        std::remove_if(selection.contractTypes.begin(),
                       selection.contractTypes.end(),
                       [](const std::string& value) { return trim(value).empty(); }),
        selection.contractTypes.end());
    return selection;
}

std::vector<std::pair<std::string, double>>
parseAnalysisAdjustmentsJson(const std::string& adjustmentsJson) {
    std::vector<std::pair<std::string, double>> out;
    if (adjustmentsJson.empty()) {
        return out;
    }
    try {
        const auto parsed = nlohmann::json::parse(adjustmentsJson);
        if (!parsed.is_object()) {
            return out;
        }
        out.reserve(parsed.size());
        for (const auto& [key, value] : parsed.items()) {
            if (value.is_number()) {
                out.emplace_back(key, value.get<double>());
            }
        }
    } catch (const nlohmann::json::exception&) {
        return {};
    }
    return out;
}

std::string buildAnalysisAdjustmentsJson(
    const std::vector<AnalysisAdjustmentTransactionInput>& transactions,
    const std::vector<std::string>& selectedTransactionIds, double taxPercent) {
    std::unordered_set<std::string> selected;
    selected.reserve(selectedTransactionIds.size());
    for (const auto& id : selectedTransactionIds) {
        const std::string value = trim(id);
        if (!value.empty()) {
            selected.insert(value);
        }
    }
    if (selected.empty()) {
        return std::string("{}");
    }

    const double factor = 1.0 + (taxPercent / 100.0);
    nlohmann::json adjustments = nlohmann::json::object();
    for (const auto& transaction : transactions) {
        const std::string id = trim(transaction.id);
        if (id.empty() || !selected.contains(id)) {
            continue;
        }
        adjustments[id] = transaction.amount * factor;
    }
    return adjustments.dump();
}

void applyAnalysisPreviewOverrides(
    core::ports::workspace::WorkspaceSnapshot& workspace,
    const std::string& analysisId, bool includeCalculationAdjustments,
    const std::vector<std::pair<std::string, double>>& adjustments) {
    const std::string targetId = trim(analysisId);
    const auto it = std::find_if(
        workspace.analyses.begin(), workspace.analyses.end(),
        [&](const auto& analysis) { return analysis.id == targetId; });
    if (it == workspace.analyses.end()) {
        return;
    }

    it->includeCalculationAdjustments = includeCalculationAdjustments;
    for (const auto& [key, value] : adjustments) {
        const auto existing =
            std::find_if(it->adjustments.begin(), it->adjustments.end(),
                         [&](const auto& item) { return item.first == key; });
        if (existing == it->adjustments.end()) {
            it->adjustments.emplace_back(key, value);
        } else {
            existing->second = value;
        }
    }
}

std::vector<std::string> contractTypesFromSnapshot(
    const core::ports::workspace::WorkspaceSnapshot& workspace) {
    std::unordered_set<std::string> seen;
    std::vector<std::string> values;
    for (const auto& contract : workspace.contracts) {
        if (contract.type.empty() || seen.contains(contract.type)) {
            continue;
        }
        seen.insert(contract.type);
        values.push_back(contract.type);
    }
    std::sort(values.begin(), values.end());
    return values;
}

} // namespace core::application::analysis
