/**
 * @file core/include/core/domain/entities/Analysis.h
 * @brief Domain model for persisted analysis definitions.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include "core/domain/values/AnalysisType.h"
#include "core/domain/values/ExportFormat.h"
#include "core/domain/values/EntityName.h"
#include "core/domain/values/FilterSpec.h"
#include "core/domain/policies/AnalysisPolicy.h"

namespace core::domain {

class Analysis {
public:
    Analysis();

    Analysis(const Analysis&) = delete;
    Analysis& operator=(const Analysis&) = delete;
    Analysis(Analysis&&) = delete;
    Analysis& operator=(Analysis&&) = delete;

    /**
     * @brief Renames the analysis with normalized entity text.
     * @param value Normalized entity name wrapper.
     */
    void rename(EntityName value);
    /**
     * @brief Renames the analysis with raw text.
     * @param value Raw entity name text.
     */
    void rename(std::string value);

    /**
     * @brief Sets the analysis type from a value object.
     * @param value Normalized analysis type wrapper.
     */
    void setType(AnalysisType value);
    /**
     * @brief Sets the analysis type from raw text.
     * @param value Raw analysis type text.
     */
    void setType(std::string value);
    /**
     * @brief Stores the raw configuration JSON.
     * @param value Configuration JSON text.
     */
    void setConfigJson(std::string value);
    /**
     * @brief Sets the filter specification from a value object.
     * @param value Normalized filter specification wrapper.
     */
    void setFilterSpec(FilterSpec value);
    /**
     * @brief Sets the filter specification from raw text.
     * @param value Raw filter specification text.
     */
    void setFilterSpec(std::string value);
    /**
     * @brief Sets the export format from a value object.
     * @param value Normalized export format wrapper.
     */
    void setExportFormat(ExportFormat value);
    /**
     * @brief Sets the export format from raw text.
     * @param value Raw export format text.
     */
    void setExportFormat(std::string value);
    /**
     * @brief Enables or disables calculation adjustments.
     * @param value Adjustment flag.
     */
    void setIncludeCalculationAdjustments(bool value);
    /**
     * @brief Stores the export state JSON snapshot.
     * @param value Export state JSON text.
     */
    void setExportStateJson(std::string value);
    /**
     * @brief Stores the snapshot transactions JSON.
     * @param value Snapshot transactions JSON text.
     */
    void setSnapshotTransactionsJson(std::string value);
    /**
     * @brief Clears all adjustment entries.
     */
    void clearAdjustments();

    /**
     * @brief Sets or updates a normalized adjustment entry.
     * @param key Raw adjustment key.
     * @param value Adjustment amount.
     */
    void setAdjustment(std::string key, double value);

    /**
     * @brief Removes a normalized adjustment entry.
     * @param key Raw adjustment key.
     */
    void removeAdjustment(const std::string& key);

    /**
     * @brief Checks whether an adjustment exists.
     * @param key Raw adjustment key.
     * @return `true` when the adjustment exists.
     */
    [[nodiscard]] bool hasAdjustment(const std::string& key) const;

    /**
     * @brief Returns the number of adjustments.
     * @return Adjustment count.
     */
    [[nodiscard]] std::size_t adjustmentCount() const noexcept;

    /**
     * @brief Checks whether the analysis has a valid type.
     * @return `true` when the type passes policy validation.
     */
    [[nodiscard]] bool hasType() const noexcept;
    /**
     * @brief Checks whether the analysis has a valid export format.
     * @return `true` when the export format is not empty.
     */
    [[nodiscard]] bool hasExportFormat() const noexcept;
    /**
     * @brief Checks whether configuration JSON exists.
     * @return `true` when configuration JSON is non-empty.
     */
    [[nodiscard]] bool hasConfig() const noexcept;
    /**
     * @brief Checks whether a filter specification exists.
     * @return `true` when the filter spec is not empty.
     */
    [[nodiscard]] bool hasFilterSpec() const noexcept;
    /**
     * @brief Checks whether snapshot transactions are present.
     * @return `true` when snapshot transactions JSON is non-empty.
     */
    [[nodiscard]] bool hasSnapshotTransactions() const noexcept;
    /**
     * @brief Checks whether the analysis is ready to export.
     * @return `true` when type and export format are valid.
     */
    [[nodiscard]] bool isReadyForExport() const noexcept;
    /**
     * @brief Checks whether the analysis is configured sufficiently for processing.
     * @return `true` when policy validation accepts the current state.
     */
    [[nodiscard]] bool isConfigured() const noexcept;
    /**
     * @brief Checks whether the analysis type is tabular.
     * @return `true` when the analysis produces tabular output.
     */
    [[nodiscard]] bool isTabular() const noexcept;
    /**
     * @brief Checks whether the analysis type is chart-like.
     * @return `true` when the analysis produces chart-like output.
     */
    [[nodiscard]] bool isChartLike() const noexcept;
    /**
     * @brief Checks whether a candidate export format matches this analysis.
     * @param candidate Export format to check.
     * @return `true` when the candidate matches the stored export format.
     */
    [[nodiscard]] bool supportsExportFormat(const std::string& candidate) const;
    /**
     * @brief Checks whether at least one adjustment exists.
     * @return `true` when the adjustment map is not empty.
     */
    [[nodiscard]] bool hasAdjustments() const noexcept;
    /**
     * @brief Checks whether the analysis has enough data to represent a result.
     * @return `true` when any major result field is present.
     */
    [[nodiscard]] bool isResultReady() const noexcept;

    /**
     * @brief Resolves the effective execution type for the analysis.
     * @return Effective execution type key.
     */
    [[nodiscard]] std::string executionType() const;

    /**
     * @brief Resolves the effective output type for the analysis.
     * @return Resolved output type key.
     */
    [[nodiscard]] std::string outputType() const;

private:
    std::string id_;
    std::string name_;
    std::string type_;
    std::string configJson_;
    std::string filterSpec_;
    std::string exportFormat_;
    bool includeCalculationAdjustments_ = true;
    std::string exportStateJson_;
    std::string snapshotTransactionsJson_;
    std::unordered_map<std::string, double> adjustments_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& type() const noexcept;
    [[nodiscard]] const std::string& configJson() const noexcept;
    [[nodiscard]] const std::string& filterSpec() const noexcept;
    [[nodiscard]] const std::string& exportFormat() const noexcept;
    [[nodiscard]] const bool& includeCalculationAdjustments() const noexcept;
    [[nodiscard]] const std::string& exportStateJson() const noexcept;
    [[nodiscard]] const std::string& snapshotTransactionsJson() const noexcept;
    [[nodiscard]] const std::unordered_map<std::string, double>& adjustments() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
};

} // namespace core::domain
