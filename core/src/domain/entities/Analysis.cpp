/**
 * @file core/src/domain/entities/Analysis.cpp
 * @brief Domain behavior for analysis definitions.
 */

#include "core/domain/entities/Analysis.h"

#include <utility>

#include <nlohmann/json.hpp>

namespace core::domain {

Analysis::Analysis() = default;

void Analysis::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Analysis::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Analysis::setType(AnalysisType value) {
    type_ = std::move(value.value());
}

void Analysis::setType(std::string value) {
    setType(AnalysisType(std::move(value)));
}

void Analysis::setConfigJson(std::string value) {
    configJson_ = std::move(value);
}

void Analysis::setFilterSpec(FilterSpec value) {
    filterSpec_ = std::move(value.value());
}

void Analysis::setFilterSpec(std::string value) {
    setFilterSpec(FilterSpec(std::move(value)));
}

void Analysis::setExportFormat(ExportFormat value) {
    exportFormat_ = std::move(value.value());
}

void Analysis::setExportFormat(std::string value) {
    setExportFormat(ExportFormat(std::move(value)));
}

void Analysis::setIncludeCalculationAdjustments(bool value) {
    includeCalculationAdjustments_ = value;
}

void Analysis::setExportStateJson(std::string value) {
    exportStateJson_ = std::move(value);
}

void Analysis::setSnapshotTransactionsJson(std::string value) {
    try {
        auto parsed = nlohmann::json::parse(value);
        if (parsed.is_array()) {
            bool allStrings = true;
            for (const auto& item : parsed) {
                if (!item.is_string()) {
                    allStrings = false;
                    break;
                }
            }
            if (allStrings) {
                nlohmann::json normalized = nlohmann::json::array();
                for (const auto& item : parsed) {
                    nlohmann::json row;
                    row["transactionId"] = item.get<std::string>();
                    normalized.push_back(std::move(row));
                }
                value = normalized.dump();
            }
        } else if (parsed.is_object()) {
            auto it = parsed.find("transactions");
            if (it != parsed.end() && it->is_array()) {
                bool allStrings = true;
                for (const auto& item : *it) {
                    if (!item.is_string()) {
                        allStrings = false;
                        break;
                    }
                }
                if (allStrings) {
                    nlohmann::json normalizedRows = nlohmann::json::array();
                    for (const auto& item : *it) {
                        nlohmann::json row;
                        row["transactionId"] = item.get<std::string>();
                        normalizedRows.push_back(std::move(row));
                    }
                    parsed["transactions"] = std::move(normalizedRows);
                    value = parsed.dump();
                }
            }
        }
    } catch (...) {
    }
    snapshotTransactionsJson_ = std::move(value);
}

void Analysis::clearAdjustments() {
    adjustments_.clear();
}

void Analysis::setAdjustment(std::string key, double value) {
    key = core::domain::policies::analysis::normalizeKey(std::move(key));
    if (key.empty()) {
        return;
    }
    adjustments_[std::move(key)] = value;
}

void Analysis::removeAdjustment(const std::string& key) {
    const auto normalized = core::domain::policies::analysis::normalizeKey(key);
    adjustments_.erase(normalized);
}

bool Analysis::hasAdjustment(const std::string& key) const {
    const auto normalized = core::domain::policies::analysis::normalizeKey(key);
    return adjustments_.find(normalized) != adjustments_.end();
}

std::size_t Analysis::adjustmentCount() const noexcept {
    return adjustments_.size();
}

bool Analysis::hasType() const noexcept {
    return policies::analysis::supportsResultType(type_);
}

bool Analysis::hasExportFormat() const noexcept {
    return !core::domain::ExportFormat::normalize(exportFormat_).empty();
}

bool Analysis::hasConfig() const noexcept {
    return !configJson_.empty();
}

bool Analysis::hasFilterSpec() const noexcept {
    return !FilterSpec::normalize(filterSpec_).empty();
}

bool Analysis::hasSnapshotTransactions() const noexcept {
    return !snapshotTransactionsJson_.empty();
}

bool Analysis::isReadyForExport() const noexcept {
    return policies::analysis::isExportable(type_, exportFormat_);
}

bool Analysis::isConfigured() const noexcept {
    return policies::analysis::isConfigured(type_, configJson_, filterSpec_, exportFormat_);
}

bool Analysis::isTabular() const noexcept {
    return policies::analysis::isTabularType(type_);
}

bool Analysis::isChartLike() const noexcept {
    return policies::analysis::isChartLikeType(type_);
}

bool Analysis::supportsExportFormat(const std::string& candidate) const {
    const auto normalized = ExportFormat::normalize(candidate);
    if (normalized.empty()) {
        return false;
    }
    return exportFormat_.empty() || exportFormat_ == normalized;
}

bool Analysis::hasAdjustments() const noexcept {
    return !adjustments_.empty();
}

bool Analysis::isResultReady() const noexcept {
    return hasSnapshotTransactions() || hasConfig() || hasType();
}

std::string Analysis::executionType() const {
    return policies::analysis::resolveExecutionType(type_);
}

std::string Analysis::outputType() const {
    return policies::analysis::resolveOutputType(type_, configJson_);
}

const std::string& Analysis::id() const noexcept { return id_; }
const std::string& Analysis::name() const noexcept { return name_; }
const std::string& Analysis::type() const noexcept { return type_; }
const std::string& Analysis::configJson() const noexcept { return configJson_; }
const std::string& Analysis::filterSpec() const noexcept { return filterSpec_; }
const std::string& Analysis::exportFormat() const noexcept { return exportFormat_; }
const bool& Analysis::includeCalculationAdjustments() const noexcept { return includeCalculationAdjustments_; }
const std::string& Analysis::exportStateJson() const noexcept { return exportStateJson_; }
const std::string& Analysis::snapshotTransactionsJson() const noexcept { return snapshotTransactionsJson_; }
const std::unordered_map<std::string, double>& Analysis::adjustments() const noexcept { return adjustments_; }
const std::string& Analysis::createdAt() const noexcept { return createdAt_; }
const std::string& Analysis::updatedAt() const noexcept { return updatedAt_; }
void Analysis::setId(std::string value) { id_ = std::move(value); }
void Analysis::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Analysis::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
