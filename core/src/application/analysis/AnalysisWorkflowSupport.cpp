/**
 * @file core/src/application/analysis/AnalysisWorkflowSupport.cpp
 * @brief Shared analysis request/config helpers for UI and application layers.
 */

#include "core/application/analysis/AnalysisWorkflowSupport.h"

#include "core/application/analysis/AnalysisKeys.h"
#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "../../utils/StringUtils.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace core::application::analysis {

namespace {

using core::utils::lowerAscii;
using core::utils::trim;

void eraseAll(std::string& value, char ch) {
    value.erase(std::remove(value.begin(), value.end(), ch), value.end());
}

} // namespace

std::string buildAnalysisConfigJson(
    const core::ports::analysis::AnalysisConfigInput& input) {
    nlohmann::json config;
    const std::string normalizedType = lowerAscii(trim(input.type));

    if (normalizedType == core::application::analysis::keys::kTypeCalculation) {
        config[std::string(core::application::analysis::keys::calculation::kStrategyKey)] =
            core::application::analysis::keys::calculation::kStrategyTax;
        config[std::string(core::application::analysis::keys::calculation::kPercentKey)] =
            input.taxPercent;
        return config.dump();
    }

    const std::string plotType = trim(input.plotType);
    config[std::string(core::application::analysis::keys::kPlotTypeKey)] =
        plotType.empty() ? core::application::analysis::keys::plotTypes::kPie
                         : plotType;

    const std::string plotMeasure = trim(input.plotMeasure);
    config[std::string(core::application::analysis::keys::kPlotMeasureKey)] =
        plotMeasure.empty()
            ? core::application::analysis::keys::plotMeasures::kTotalAmount
            : plotMeasure;

    nlohmann::json properties = nlohmann::json::array();
    for (const auto& propertyId : input.propertyIds) {
        const std::string value = trim(propertyId);
        if (!value.empty()) {
            properties.push_back(value);
        }
    }
    config[std::string(core::application::analysis::keys::kPropertiesKey)] =
        std::move(properties);

    nlohmann::json contractTypes = nlohmann::json::array();
    for (auto contractType : input.contractTypes) {
        contractType = lowerAscii(trim(contractType));
        if (!contractType.empty()) {
            contractTypes.push_back(contractType);
        }
    }
    config[std::string(core::application::analysis::keys::kContractTypesKey)] =
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
    selection.dateField = lowerAscii(trim(dateField));
    selection.dateMode = lowerAscii(trim(dateMode));
    selection.year = trim(year);
    selection.dateFrom = trim(dateFrom);
    selection.dateTo = trim(dateTo);
    selection.allocatableMode = lowerAscii(trim(allocatableMode));
    selection.propertyIds = propertyIds;
    selection.contractTypes = contractTypes;
    for (auto& contractType : selection.contractTypes) {
        contractType = lowerAscii(trim(contractType));
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

std::optional<double> parseAnalysisPercentText(const std::string& text) {
    std::string normalized = trim(text);
    if (normalized.empty()) {
        return std::nullopt;
    }

    if (normalized.back() == '%') {
        normalized.pop_back();
        normalized = trim(normalized);
    }
    eraseAll(normalized, ' ');

    try {
        size_t consumed = 0;
        const double direct = std::stod(normalized, &consumed);
        if (consumed == normalized.size() && std::isfinite(direct)) {
            return direct;
        }
    } catch (...) {
    }

    const auto lastComma = normalized.find_last_of(',');
    const auto lastDot = normalized.find_last_of('.');
    if (lastComma != std::string::npos && lastDot != std::string::npos) {
        if (lastComma > lastDot) {
            eraseAll(normalized, '.');
            std::replace(normalized.begin(), normalized.end(), ',', '.');
        } else {
            eraseAll(normalized, ',');
        }
    } else if (lastComma != std::string::npos) {
        std::replace(normalized.begin(), normalized.end(), ',', '.');
    }

    try {
        size_t consumed = 0;
        const double parsed = std::stod(normalized, &consumed);
        if (consumed == normalized.size() && std::isfinite(parsed)) {
            return parsed;
        }
    } catch (...) {
    }
    return std::nullopt;
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
