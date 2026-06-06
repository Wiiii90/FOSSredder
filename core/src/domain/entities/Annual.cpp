/**
 * @file core/src/domain/entities/Annual.cpp
 * @brief Domain behavior for annual aggregates.
 */

#include "core/domain/entities/Annual.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace core::domain {

Annual::Annual() = default;

void Annual::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Annual::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Annual::setYear(Year value) {
    year_ = value.value();
}

void Annual::setYear(int value) {
    setYear(Year(value));
}

void Annual::apply(std::string nameValue, int yearValue, std::vector<std::string> analysisIdsValue) {
    rename(std::move(nameValue));
    setYear(yearValue);
    setAnalysisIds(std::move(analysisIdsValue));
}

void Annual::setAnalysisIds(std::vector<std::string> value) {
    policies::annual::normalizeIds(value);
    analysisIds_ = std::move(value);
}

void Annual::addAnalysisId(std::string value) {
    value = policies::annual::normalizeId(std::move(value));
    if (value.empty() || containsAnalysisId(value)) {
        return;
    }
    analysisIds_.push_back(std::move(value));
}

void Annual::insertAnalysisId(std::string value, std::size_t position) {
    value = policies::annual::normalizeId(std::move(value));
    if (value.empty()) {
        return;
    }
    removeAnalysisId(value);
    if (position >= analysisIds_.size()) {
        analysisIds_.push_back(std::move(value));
        return;
    }
    analysisIds_.insert(analysisIds_.begin() + static_cast<std::vector<std::string>::difference_type>(position), std::move(value));
}

void Annual::removeAnalysisId(const std::string& value) {
    const auto normalized = policies::annual::normalizeId(value);
    analysisIds_.erase(std::remove(analysisIds_.begin(), analysisIds_.end(), normalized), analysisIds_.end());
}

void Annual::clearAnalysisIds() {
    analysisIds_.clear();
}

bool Annual::containsAnalysisId(const std::string& value) const {
    return policies::annual::containsAnalysisId(analysisIds_, value);
}

std::size_t Annual::indexOfAnalysisId(const std::string& value) const {
    const auto normalized = policies::annual::normalizeId(value);
    const auto it = std::find(analysisIds_.begin(), analysisIds_.end(), normalized);
    if (it == analysisIds_.end()) {
        return analysisIds_.size();
    }
    return static_cast<std::size_t>(std::distance(analysisIds_.begin(), it));
}

void Annual::moveAnalysisId(const std::string& value, std::size_t newPosition) {
    const auto normalized = policies::annual::normalizeId(value);
    const auto it = std::find(analysisIds_.begin(), analysisIds_.end(), normalized);
    if (it == analysisIds_.end()) {
        return;
    }
    const auto current = static_cast<std::size_t>(std::distance(analysisIds_.begin(), it));
    const auto item = *it;
    analysisIds_.erase(it);
    if (newPosition >= analysisIds_.size()) {
        analysisIds_.push_back(item);
        return;
    }
    if (current < newPosition && newPosition > 0) {
        --newPosition;
    }
    analysisIds_.insert(analysisIds_.begin() + static_cast<std::vector<std::string>::difference_type>(newPosition), item);
}

std::size_t Annual::analysisCount() const noexcept {
    return analysisIds_.size();
}

bool Annual::hasYear() const noexcept {
    return policies::annual::isValidYear(year_);
}

bool Annual::isEmpty() const noexcept {
    return name_.empty() && !hasYear() && analysisIds_.empty();
}

const std::string& Annual::id() const noexcept { return id_; }
const std::string& Annual::name() const noexcept { return name_; }
const int& Annual::year() const noexcept { return year_; }
const std::vector<std::string>& Annual::analysisIds() const noexcept { return analysisIds_; }
const std::string& Annual::createdAt() const noexcept { return createdAt_; }
const std::string& Annual::updatedAt() const noexcept { return updatedAt_; }
void Annual::setId(std::string value) { id_ = std::move(value); }
void Annual::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Annual::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
