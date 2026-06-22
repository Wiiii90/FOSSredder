/**
 * @file core/src/application/analysis/presentation/PlotAnalysis.cpp
 * @brief Implements plot-oriented analysis aggregation.
 */

#include "PlotAnalysis.h"
#include "../internal/AnalysisFilter.h"
#include "core/application/analysis/AnalysisKeys.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Transaction.h"
#include "../../../utils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace core::application::analysis {

namespace {

using ContractTypeIndex = std::unordered_map<std::string, std::string>;
using ContractPropertyIndex = std::unordered_map<std::string, std::vector<std::string>>;
using PropertyNameIndex = std::unordered_map<std::string, std::string>;

struct PlotConfig {
    std::string plotType = std::string(core::application::analysis::keys::plotTypes::kPie);
    std::string plotMeasure = std::string(core::application::analysis::keys::plotMeasures::kTotalAmount);
    std::vector<std::string> propertyAnalysisFilter;
    std::vector<std::string> contractTypeAnalysisFilter;
};

std::string normalizeValue(const std::string& value);
std::string normalizePlotMeasure(const std::string& value);

PlotConfig parsePlotConfig(const core::domain::Analysis& analysis)
{
    PlotConfig config;
    if (analysis.configJson().empty()) {
        return config;
    }

    try {
        const auto json = nlohmann::json::parse(analysis.configJson());
        if (!json.is_object()) {
            return config;
        }

        const auto plotTypeIt = json.find(core::application::analysis::keys::kPlotTypeKey);
        if (plotTypeIt != json.end() && plotTypeIt->is_string()) {
            config.plotType = normalizeValue(plotTypeIt->get<std::string>());
        }

        const auto plotMeasureIt = json.find(core::application::analysis::keys::kPlotMeasureKey);
        if (plotMeasureIt != json.end() && plotMeasureIt->is_string()) {
            config.plotMeasure = normalizePlotMeasure(plotMeasureIt->get<std::string>());
        }

        const auto propertiesIt = json.find(core::application::analysis::keys::kPropertiesKey);
        if (propertiesIt != json.end() && propertiesIt->is_array()) {
            for (const auto& entry : *propertiesIt) {
                if (entry.is_string()) {
                    config.propertyAnalysisFilter.push_back(entry.get<std::string>());
                }
            }
        }

        const auto contractTypesIt = json.find(core::application::analysis::keys::kContractTypesKey);
        if (contractTypesIt != json.end() && contractTypesIt->is_array()) {
            for (const auto& entry : *contractTypesIt) {
                if (entry.is_string()) {
                    config.contractTypeAnalysisFilter.push_back(entry.get<std::string>());
                }
            }
        }
    } catch (...) {}

    return config;
}

std::string normalizeValue(const std::string& value)
{
    std::string normalized = core::utils::trim(value);
    if (normalized.empty()) {
        return {};
    }

    for (auto& ch : normalized) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    return normalized;
}

std::vector<std::string> normalizeList(const std::vector<std::string>& values)
{
    std::vector<std::string> normalizedValues;
    normalizedValues.reserve(values.size());
    for (const auto& value : values) {
        const auto normalized = normalizeValue(value);
        if (!normalized.empty()) {
            normalizedValues.push_back(normalized);
        }
    }
    return normalizedValues;
}

std::string normalizePlotMeasure(const std::string& value)
{
    const auto normalized = normalizeValue(value);
    if (normalized == core::application::analysis::keys::plotMeasures::kCount) {
        return std::string(core::application::analysis::keys::plotMeasures::kCount);
    }
    if (normalized == "averageamount" || normalized == "average amount") {
        return std::string(core::application::analysis::keys::plotMeasures::kAverageAmount);
    }
    if (normalized == "totalamount" || normalized == "total amount") {
        return std::string(core::application::analysis::keys::plotMeasures::kTotalAmount);
    }
    return normalized;
}

ContractTypeIndex buildContractTypeIndex(const core::domain::catalog::WorkspaceCatalog& state, bool normalize)
{
    ContractTypeIndex index;
    index.reserve(state.contracts().size());
    for (const auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        index.emplace(contract->id(), normalize ? normalizeValue(contract->type()) : contract->type());
    }
    return index;
}

ContractPropertyIndex buildContractPropertyIndex(const core::domain::catalog::WorkspaceCatalog& state)
{
    ContractPropertyIndex index;
    index.reserve(state.contracts().size());
    for (const auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        index.emplace(contract->id(), contract->propertyIds());
    }
    return index;
}

PropertyNameIndex buildPropertyNameIndex(const core::domain::catalog::WorkspaceCatalog& state)
{
    PropertyNameIndex index;
    index.reserve(state.properties().size());
    for (const auto& property : state.properties()) {
        if (!property) {
            continue;
        }
        index.emplace(property->id(), property->name().empty() ? property->id() : property->name());
    }
    return index;
}

std::string propertyLabelForId(const PropertyNameIndex& propertyNameById,
                               const std::string& propertyId)
{
    const auto it = propertyNameById.find(propertyId);
    if (it == propertyNameById.end() || it->second.empty()) {
        return propertyId;
    }
    return it->second;
}

std::vector<std::string> effectivePropertyIds(const core::domain::Transaction& transaction,
                                              const ContractPropertyIndex& propertyIdsByContractId)
{
    std::vector<std::string> ids = transaction.propertyIds();
    if (transaction.contractId().empty()) {
        return ids;
    }

    const auto it = propertyIdsByContractId.find(transaction.contractId());
    if (it == propertyIdsByContractId.end()) {
        return ids;
    }

    for (const auto& propertyId : it->second) {
        if (std::find(ids.begin(), ids.end(), propertyId) == ids.end()) {
            ids.push_back(propertyId);
        }
    }
    return ids;
}

bool matchesPropertyAnalysisFilter(const core::domain::Transaction& transaction,
                                   const std::vector<std::string>& propertyAnalysisFilter,
                                   const ContractPropertyIndex& propertyIdsByContractId)
{
    if (propertyAnalysisFilter.empty()) {
        return true;
    }

    const auto propertyIds = effectivePropertyIds(transaction, propertyIdsByContractId);
    for (const auto& propertyId : propertyIds) {
        if (std::find(propertyAnalysisFilter.begin(), propertyAnalysisFilter.end(), propertyId) != propertyAnalysisFilter.end()) {
            return true;
        }
    }

    return false;
}

bool matchesContractTypeAnalysisFilter(const core::domain::Transaction& transaction,
                                       const std::vector<std::string>& normalizedContractTypeAnalysisFilter,
                                       const ContractTypeIndex& normalizedContractTypeById)
{
    if (normalizedContractTypeAnalysisFilter.empty()) {
        return true;
    }

    if (transaction.contractId().empty()) {
        return std::find(normalizedContractTypeAnalysisFilter.begin(),
                         normalizedContractTypeAnalysisFilter.end(),
                         std::string(core::application::analysis::filterKeys::kUnassigned)) != normalizedContractTypeAnalysisFilter.end();
    }

    const auto it = normalizedContractTypeById.find(transaction.contractId());
    if (it == normalizedContractTypeById.end()) {
        return false;
    }

    return std::find(normalizedContractTypeAnalysisFilter.begin(),
                     normalizedContractTypeAnalysisFilter.end(),
                     it->second) != normalizedContractTypeAnalysisFilter.end();
}

std::vector<std::shared_ptr<core::domain::Transaction>> collectMatchedTransactions(const core::domain::catalog::WorkspaceCatalog& state,
                                                                                   const AnalysisFilter& filter,
                                                                                   const std::vector<std::string>& propertyAnalysisFilter,
                                                                                   const std::vector<std::string>& normalizedContractTypeAnalysisFilter,
                                                                                   const ContractTypeIndex& normalizedContractTypeById,
                                                                                   const ContractPropertyIndex& propertyIdsByContractId)
{
    std::vector<std::shared_ptr<core::domain::Transaction>> matched;
    matched.reserve(state.transactions().size());
    for (const auto& transaction : state.transactions()) {
        if (!transaction) {
            continue;
        }
        if (!filter.empty() && !filter.matches(transaction, state)) {
            continue;
        }
        if (!matchesPropertyAnalysisFilter(*transaction, propertyAnalysisFilter, propertyIdsByContractId)) {
            continue;
        }
        if (!matchesContractTypeAnalysisFilter(*transaction, normalizedContractTypeAnalysisFilter, normalizedContractTypeById)) {
            continue;
        }
        matched.push_back(transaction);
    }

    return matched;
}

std::string extractYearMonth(const std::string& bookingDate)
{
    const std::string value = core::utils::trim(bookingDate);
    if (value.empty()) {
        return {};
    }

    if (value.size() >= 7
        && std::isdigit(static_cast<unsigned char>(value[0]))
        && std::isdigit(static_cast<unsigned char>(value[1]))
        && std::isdigit(static_cast<unsigned char>(value[2]))
        && std::isdigit(static_cast<unsigned char>(value[3]))) {
        const std::string year = value.substr(0, 4);
        if (value[4] == '-') {
            return year + "-" + value.substr(5, 2);
        }

        if (value.size() >= 6) {
            return year + "-" + value.substr(4, 2);
        }
    }

    if (value.find('.') != std::string::npos) {
        std::vector<std::string> parts;
        std::istringstream stream(value);
        std::string token;
        while (std::getline(stream, token, '.')) {
            parts.push_back(core::utils::trim(token));
        }

        if (parts.size() == 3) {
            std::string month = parts[1];
            if (parts[2].size() == 4 && (month.size() == 1 || month.size() == 2)) {
                if (month.size() == 1) {
                    month.insert(month.begin(), '0');
                }
                return parts[2] + "-" + month;
            }
        }
    }

    return value;
}

core::ports::analysis::AnalysisResult buildPieResult(
    const std::vector<std::shared_ptr<core::domain::Transaction>>& matched,
    const ContractTypeIndex& normalizedContractTypeById,
    const std::string& plotMeasure)
{
    core::ports::analysis::AnalysisResult result;
    std::map<std::string, double> aggregatedAmounts;
    std::map<std::string, int> transactionCounts;

    for (const auto& transaction : matched) {
        std::string key = std::string(core::application::analysis::filterKeys::kUnassigned);
        if (!transaction->contractId().empty()) {
            const auto it = normalizedContractTypeById.find(transaction->contractId());
            if (it != normalizedContractTypeById.end() && !it->second.empty()) {
                key = it->second;
            } else {
                key = transaction->contractId();
            }
        }

        aggregatedAmounts[key] += transaction->amount();
        transactionCounts[key]++;
    }

    double totalAmount = 0.0;
    std::size_t totalCount = 0;
    for (const auto& [key, amount] : aggregatedAmounts) {
        totalAmount += std::fabs(amount);
        totalCount += transactionCounts[key];
    }

    for (const auto& [key, amount] : aggregatedAmounts) {
        double value = 0.0;
        if (plotMeasure == core::application::analysis::keys::plotMeasures::kCount) {
            value = static_cast<double>(transactionCounts[key]);
        } else if (plotMeasure == core::application::analysis::keys::plotMeasures::kAverageAmount) {
            value = transactionCounts[key] > 0 ? std::fabs(amount) / transactionCounts[key] : 0.0;
        } else {
            value = std::fabs(amount);
        }

        result.table.push_back({key, std::to_string(value)});
    }

    result.metrics[std::string(core::application::analysis::keys::metricKeys::kTotalAmount)] = totalAmount;
    result.metrics[std::string(core::application::analysis::keys::metricKeys::kRowCount)] = static_cast<double>(totalCount);
    return result;
}

core::ports::analysis::AnalysisResult buildHistogramResult(
    const std::vector<std::shared_ptr<core::domain::Transaction>>& matched,
    const core::domain::catalog::WorkspaceCatalog& state,
    const ContractPropertyIndex& propertyIdsByContractId)
{
    core::ports::analysis::AnalysisResult result;
    if (matched.empty()) {
        return result;
    }

    const auto contractTypeByIdRaw = buildContractTypeIndex(state, false);
    const auto propertyNameById = buildPropertyNameIndex(state);
    std::map<std::string, double> monthTotal;
    std::map<std::string, std::map<std::string, double>> monthByContract;
    std::map<std::string, std::map<std::string, double>> monthByProperty;

    for (const auto& transaction : matched) {
        const std::string month = extractYearMonth(transaction->bookingDate());
        const double amount = std::fabs(transaction->amount());
        monthTotal[month] += amount;

        std::string contractType = std::string(core::application::analysis::filterKeys::kUnassigned);
        if (!transaction->contractId().empty()) {
            const auto it = contractTypeByIdRaw.find(transaction->contractId());
            if (it != contractTypeByIdRaw.end() && !it->second.empty()) {
                contractType = it->second;
            } else {
                contractType = transaction->contractId();
            }
        }
        monthByContract[month][contractType] += amount;

        const auto propertyIds = effectivePropertyIds(*transaction, propertyIdsByContractId);
        if (!propertyIds.empty()) {
            for (const auto& propertyId : propertyIds) {
                monthByProperty[month][propertyLabelForId(propertyNameById, propertyId)] += amount;
            }
        } else {
            monthByProperty[month][std::string(core::application::analysis::keys::labels::kNoProperty)] += amount;
        }
    }

    std::vector<std::string> months;
    months.reserve(monthTotal.size());
    for (const auto& [month, amount] : monthTotal) {
        (void)amount;
        months.push_back(month);
    }
    std::sort(months.begin(), months.end());

    double totalAmount = 0.0;
    for (const auto& month : months) {
        nlohmann::json summary;
        summary[core::application::analysis::keys::resultFields::kMonth] = month;
        summary[core::application::analysis::keys::resultFields::kTotal] = monthTotal[month];
        summary[core::application::analysis::keys::resultFields::kByContract] = nlohmann::json::object();
        for (const auto& [label, amount] : monthByContract[month]) {
            summary[core::application::analysis::keys::resultFields::kByContract][label] = amount;
        }
        summary[core::application::analysis::keys::resultFields::kByProperty] = nlohmann::json::object();
        for (const auto& [label, amount] : monthByProperty[month]) {
            summary[core::application::analysis::keys::resultFields::kByProperty][label] = amount;
        }

        result.table.push_back({month, summary.dump()});
        totalAmount += monthTotal[month];
    }

    result.metrics[std::string(core::application::analysis::keys::metricKeys::kTotalAmount)] = totalAmount;
    result.metrics[std::string(core::application::analysis::keys::metricKeys::kRowCount)] = static_cast<double>(months.size());
    result.metrics[std::string(core::application::analysis::keys::metricKeys::kMatchedTransactions)] = static_cast<double>(matched.size());
    return result;
}

} // namespace

core::ports::analysis::AnalysisResult
computePlotAnalysis(const core::domain::Analysis& analysis,
                    const core::domain::catalog::WorkspaceCatalog& state,
                    const AnalysisFilter& filter)
{
    const PlotConfig config = parsePlotConfig(analysis);
    const ContractTypeIndex normalizedContractTypeById = buildContractTypeIndex(state, true);
    const ContractPropertyIndex propertyIdsByContractId = buildContractPropertyIndex(state);
    const auto normalizedPropertyFilter = normalizeList(config.propertyAnalysisFilter);
    const auto normalizedContractTypeFilter = normalizeList(config.contractTypeAnalysisFilter);
    const auto matched = collectMatchedTransactions(state,
                                                    filter,
                                                    normalizedPropertyFilter,
                                                    normalizedContractTypeFilter,
                                                    normalizedContractTypeById,
                                                    propertyIdsByContractId);

    core::ports::analysis::AnalysisResult out;
    if (config.plotType == core::application::analysis::keys::plotTypes::kHistogram) {
        out = buildHistogramResult(matched, state, propertyIdsByContractId);
    } else {
        out = buildPieResult(matched, normalizedContractTypeById, config.plotMeasure);
    }

    return out;
}

} // namespace core::application::analysis
