/**
 * @file core/src/domain/entities/Statement.cpp
 * @brief Domain behavior for statements.
 */

#include "core/domain/entities/Statement.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace core::domain {

Statement::Statement() = default;

void Statement::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Statement::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Statement::addTransaction(std::string value) {
    value = policies::statement::normalizeId(std::move(value));
    if (value.empty() || containsTransaction(value)) {
        return;
    }
    transactionIds_.push_back(std::move(value));
}

void Statement::setTransactionIds(std::vector<std::string> value) {
    policies::statement::normalizeIds(value);
    transactionIds_ = std::move(value);
}

void Statement::insertTransaction(std::string value, std::size_t position) {
    value = policies::statement::normalizeId(std::move(value));
    if (value.empty()) {
        return;
    }
    removeTransaction(value);
    if (position >= transactionIds_.size()) {
        transactionIds_.push_back(std::move(value));
        return;
    }
    transactionIds_.insert(transactionIds_.begin() + static_cast<std::vector<std::string>::difference_type>(position), std::move(value));
}

void Statement::removeTransaction(const std::string& value) {
    const auto normalized = policies::statement::normalizeId(value);
    transactionIds_.erase(std::remove(transactionIds_.begin(), transactionIds_.end(), normalized), transactionIds_.end());
}

std::size_t Statement::indexOfTransaction(const std::string& value) const {
    return policies::statement::indexOfTransactionId(transactionIds_, value);
}

bool Statement::hasTransactionAt(std::size_t index) const noexcept {
    return index < transactionIds_.size();
}

void Statement::moveTransaction(const std::string& value, std::size_t newPosition) {
    const auto normalized = policies::statement::normalizeId(value);
    const auto it = std::find(transactionIds_.begin(), transactionIds_.end(), normalized);
    if (it == transactionIds_.end()) {
        return;
    }
    const auto current = static_cast<std::size_t>(std::distance(transactionIds_.begin(), it));
    const auto item = *it;
    transactionIds_.erase(it);
    if (newPosition >= transactionIds_.size()) {
        transactionIds_.push_back(item);
        return;
    }
    if (current < newPosition && newPosition > 0) {
        --newPosition;
    }
    transactionIds_.insert(transactionIds_.begin() + static_cast<std::vector<std::string>::difference_type>(newPosition), item);
}

bool Statement::containsTransaction(const std::string& value) const {
    return policies::statement::containsTransactionId(transactionIds_, value);
}

void Statement::clearTransactions() {
    transactionIds_.clear();
}

std::size_t Statement::transactionCount() const noexcept {
    return transactionIds_.size();
}

bool Statement::empty() const noexcept {
    return transactionIds_.empty();
}

const std::string& Statement::id() const noexcept { return id_; }
const std::string& Statement::name() const noexcept { return name_; }
const std::vector<std::string>& Statement::transactionIds() const noexcept { return transactionIds_; }
const std::string& Statement::createdAt() const noexcept { return createdAt_; }
const std::string& Statement::updatedAt() const noexcept { return updatedAt_; }
void Statement::setId(std::string value) { id_ = std::move(value); }
void Statement::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Statement::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
