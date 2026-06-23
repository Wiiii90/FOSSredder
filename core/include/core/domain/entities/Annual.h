/**
 * @file core/include/core/domain/entities/Annual.h
 * @brief Domain model for annual aggregates.
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <utility>

#include "core/domain/policies/AnnualPolicy.h"
#include "core/domain/values/EntityName.h"
#include "core/domain/values/Year.h"

namespace core::domain {

class Annual {
public:
    Annual();

    Annual(const Annual&) = delete;
    Annual& operator=(const Annual&) = delete;
    Annual(Annual&&) = delete;
    Annual& operator=(Annual&&) = delete;

    /**
     * @brief Renames the annual aggregate with normalized entity text.
     * @param value Normalized entity name wrapper.
     */
    void rename(EntityName value);
    /**
     * @brief Renames the annual aggregate with raw text.
     * @param value Raw entity name text.
     */
    void rename(std::string value);

    /**
     * @brief Sets the annual year from a value object.
     * @param value Normalized year wrapper.
     */
    void setYear(Year value);
    /**
     * @brief Sets the annual year from raw input.
     * @param value Raw year value.
     */
    void setYear(int value);

    /**
     * @brief Applies the complete annual definition in one normalized domain step.
     * @param nameValue Raw annual name text.
     * @param yearValue Raw year value.
     * @param analysisIdsValue Analysis id list to store.
     */
    void apply(std::string nameValue, int yearValue, std::vector<std::string> analysisIdsValue);

    /**
     * @brief Replaces the linked analysis ids with normalized values.
     * @param value Analysis id list to store.
     */
    void setAnalysisIds(std::vector<std::string> value);

    /**
     * @brief Adds an analysis id when it is not already present.
     * @param value Analysis id to add.
     */
    void addAnalysisId(std::string value);

    /**
     * @brief Inserts an analysis id at a requested position.
     * @param value Analysis id to insert.
     * @param position Target insertion position.
     */
    void insertAnalysisId(std::string value, std::size_t position);

    /**
     * @brief Removes an analysis id from the annual aggregate.
     * @param value Analysis id to remove.
     */
    void removeAnalysisId(const std::string& value);

    /**
     * @brief Clears all linked analysis ids.
     */
    void clearAnalysisIds();

    /**
     * @brief Checks whether an analysis id exists.
     * @param value Analysis id to search for.
     * @return `true` when the analysis id exists.
     */
    [[nodiscard]] bool containsAnalysisId(const std::string& value) const;

    /**
     * @brief Returns the index of an analysis id.
     * @param value Analysis id to search for.
     * @return Matching index or the list size when not found.
     */
    [[nodiscard]] std::size_t indexOfAnalysisId(const std::string& value) const;

    /**
     * @brief Moves an analysis id to a new position.
     * @param value Analysis id to move.
     * @param newPosition New insertion position.
     */
    void moveAnalysisId(const std::string& value, std::size_t newPosition);

    /**
     * @brief Returns the number of linked analysis ids.
     * @return Analysis relation count.
     */
    [[nodiscard]] std::size_t analysisCount() const noexcept;

    /**
     * @brief Checks whether the annual year is valid.
     * @return `true` when the year passes value-object validation.
     */
    [[nodiscard]] bool hasYear() const noexcept;

    /**
     * @brief Checks whether the annual aggregate is empty.
     * @return `true` when name, year and analysis ids are empty.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

private:
    std::string id_;
    std::string name_;
    int year_ = 0;
    std::vector<std::string> analysisIds_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const int& year() const noexcept;
    [[nodiscard]] const std::vector<std::string>& analysisIds() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
};

} // namespace core::domain
