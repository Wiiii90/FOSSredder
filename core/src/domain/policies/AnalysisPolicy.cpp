/**
 * @file core/src/domain/policies/AnalysisPolicy.cpp
 * @brief Shared analysis validation helpers.
 */

#include "core/domain/policies/AnalysisPolicy.h"

#include <nlohmann/json.hpp>

#include <utility>

namespace core::domain::policies::analysis {

bool supportsResultType(const std::string& type) {
    const auto normalized = core::domain::AnalysisType::normalize(type);
    return !normalized.empty();
}

bool isTabularType(const std::string& type) {
    return core::domain::AnalysisType::normalize(type) == core::constants::analysis::kTypeTab;
}

bool isChartLikeType(const std::string& type) {
    const auto normalized = core::domain::AnalysisType::normalize(type);
    return normalized == core::constants::analysis::plotTypes::kPie ||
           normalized == core::constants::analysis::plotTypes::kHistogram;
}

bool isExportable(const std::string& type, const std::string& exportFormat) {
    return !core::domain::AnalysisType::normalize(type).empty() && !core::domain::ExportFormat::normalize(exportFormat).empty();
}

std::string normalizeKey(std::string key) {
    return core::domain::FilterSpec::normalize(std::move(key));
}

bool isConfigured(const std::string& type,
                  const std::string& configJson,
                  const std::string& filterSpec,
                  const std::string& exportFormat) {
    return !core::domain::AnalysisType::normalize(type).empty() ||
           !configJson.empty() ||
           !core::domain::FilterSpec::normalize(filterSpec).empty() ||
           !core::domain::ExportFormat::normalize(exportFormat).empty();
}

std::string resolveOutputType(const std::string& type, const std::string& configJson) {
    const auto normalizedType = core::domain::AnalysisType::normalize(type);
    if (normalizedType != core::constants::analysis::kTypePlot) {
        return normalizedType;
    }

    if (configJson.empty()) {
        return std::string(core::constants::analysis::plotTypes::kPie);
    }

    try {
        const auto config = nlohmann::json::parse(configJson);
        if (config.contains(core::constants::analysis::kPlotTypeKey) && config[core::constants::analysis::kPlotTypeKey].is_string()) {
            return config[core::constants::analysis::kPlotTypeKey].get<std::string>();
        }
    } catch (...) {
    }

    return std::string(core::constants::analysis::plotTypes::kPie);
}

std::string resolveExecutionType(const std::string& type) {
    const auto normalizedType = core::domain::AnalysisType::normalize(type);
    if (normalizedType.empty()) {
        return std::string(core::constants::analysis::kTypeTab);
    }
    return normalizedType;
}

} // namespace core::domain::policies::analysis
