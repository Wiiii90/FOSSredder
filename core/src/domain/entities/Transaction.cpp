/**
 * @file core/src/domain/entities/Transaction.cpp
 * @brief Domain behavior for transactions.
 */

#include "core/domain/entities/Transaction.h"

#include <algorithm>
#include <utility>

namespace core::domain {

Transaction::Transaction() noexcept = default;

Transaction::Transaction(std::string nameValue,
                         std::string bookingDateValue,
                         std::string valutaValue,
                         double amountValue,
                         bool allocatableValue)
    : amount_(amountValue),
      allocatable_(allocatableValue) {
    setName(std::move(nameValue));
    setBookingDate(std::move(bookingDateValue));
    setValuta(std::move(valutaValue));
}

void Transaction::setName(std::string value) {
    name_ = policies::transaction::normalizeText(std::move(value));
}

void Transaction::setBookingDate(BookingDate value) {
    bookingDate_ = std::move(value.value());
}

void Transaction::setBookingDate(std::string value) {
    setBookingDate(BookingDate(std::move(value)));
}

void Transaction::setValuta(std::string value) {
    valuta_ = policies::transaction::normalizeText(std::move(value));
}

void Transaction::setAmount(MoneyAmount value) {
    amount_ = value.value();
}

void Transaction::setAmount(double value) {
    setAmount(MoneyAmount(value));
}

void Transaction::setStatus(Status value) {
    status_ = value;
}

void Transaction::setContractId(std::string value) {
    contractId_ = policies::transaction::normalizeId(std::move(value));
}

void Transaction::setActorId(std::string value) {
    actorId_ = policies::transaction::normalizeId(std::move(value));
}

void Transaction::setStatementId(std::string value) {
    statementId_ = policies::transaction::normalizeId(std::move(value));
}

void Transaction::setAllocatable(bool value) {
    allocatable_ = value;
}

void Transaction::clearContract() {
    contractId_.clear();
}

void Transaction::clearActor() {
    actorId_.clear();
}

void Transaction::clearStatement() {
    statementId_.clear();
}

void Transaction::clearProperties() {
    propertyIds_.clear();
}

bool Transaction::hasStatus(Status value) const noexcept {
    return status_ == value;
}

bool Transaction::hasStatement() const noexcept {
    return !statementId_.empty();
}

bool Transaction::hasContract() const noexcept {
    return !contractId_.empty();
}

bool Transaction::hasActor() const noexcept {
    return !actorId_.empty();
}

bool Transaction::hasProperties() const noexcept {
    return !propertyIds_.empty();
}

std::size_t Transaction::propertyCount() const noexcept {
    return propertyIds_.size();
}

bool Transaction::belongsToStatement(const std::string& value) const {
    return hasStatement() && statementId_ == policies::transaction::normalizeId(value);
}

bool Transaction::isAllocatable() const noexcept {
    return allocatable_;
}

void Transaction::markUnverified() {
    if (core::domain::policies::transaction::statusCanAdvance(static_cast<int>(status_), static_cast<int>(Status::Unverified))) {
        setStatus(Status::Unverified);
    }
}

void Transaction::markVerified() {
    if (core::domain::policies::transaction::statusCanAdvance(static_cast<int>(status_), static_cast<int>(Status::Verified))) {
        setStatus(Status::Verified);
    }
}

void Transaction::markCompleted() {
    if (core::domain::policies::transaction::statusCanAdvance(static_cast<int>(status_), static_cast<int>(Status::Completed))) {
        setStatus(Status::Completed);
    }
}

bool Transaction::hasRelations() const noexcept {
    return hasStatement() || hasContract() || hasActor() || hasProperties();
}

bool Transaction::isStandalone() const noexcept {
    return !hasRelations();
}

void Transaction::setPropertyIds(std::vector<std::string> value) {
    policies::transaction::normalizeIds(value);
    propertyIds_ = std::move(value);
}

void Transaction::addPropertyId(std::string value) {
    value = policies::transaction::normalizeId(std::move(value));
    if (value.empty() || containsPropertyId(value)) {
        return;
    }
    propertyIds_.push_back(std::move(value));
}

void Transaction::removePropertyId(const std::string& value) {
    const auto normalized = policies::transaction::normalizeId(value);
    propertyIds_.erase(std::remove(propertyIds_.begin(), propertyIds_.end(), normalized), propertyIds_.end());
}

bool Transaction::containsPropertyId(const std::string& value) const {
    const auto normalized = policies::transaction::normalizeId(value);
    return std::find(propertyIds_.begin(), propertyIds_.end(), normalized) != propertyIds_.end();
}

const std::string& Transaction::id() const noexcept { return id_; }
const std::string& Transaction::name() const noexcept { return name_; }
const std::string& Transaction::bookingDate() const noexcept { return bookingDate_; }
const std::string& Transaction::valuta() const noexcept { return valuta_; }
const double& Transaction::amount() const noexcept { return amount_; }
const Transaction::Status& Transaction::status() const noexcept { return status_; }
const std::string& Transaction::contractId() const noexcept { return contractId_; }
const std::string& Transaction::actorId() const noexcept { return actorId_; }
const std::string& Transaction::statementId() const noexcept { return statementId_; }
const std::vector<std::string>& Transaction::propertyIds() const noexcept { return propertyIds_; }
const std::string& Transaction::createdAt() const noexcept { return createdAt_; }
const std::string& Transaction::updatedAt() const noexcept { return updatedAt_; }
void Transaction::setId(std::string value) { id_ = std::move(value); }
void Transaction::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Transaction::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
