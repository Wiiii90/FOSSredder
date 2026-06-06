/**
 * @file core/include/core/domain/entities/Transaction.h
 * @brief Domain model for transactions.
 */

#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "core/domain/policies/TransactionPolicy.h"
#include "core/domain/values/BookingDate.h"
#include "core/domain/values/MoneyAmount.h"

namespace core::domain {

class Transaction {
public:
    enum class Status : int {
        Neutral = 0,
        Unverified = 1,
        Verified = 2,
        Completed = 3
    };

    Transaction() noexcept;
    /**
     * @brief Creates a transaction from raw inputs.
     * @param nameValue Raw transaction name.
     * @param bookingDateValue Raw booking date.
     * @param valutaValue Raw valuta text.
     * @param amountValue Raw amount.
     * @param allocatableValue Whether the transaction can be allocated.
     */
    Transaction(std::string nameValue,
                std::string bookingDateValue,
                std::string valutaValue,
                double amountValue,
                bool allocatableValue = false);

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(Transaction&&) = delete;

    /**
     * @brief Sets the transaction name.
     * @param value Raw transaction name text.
     */
    void setName(std::string value);
    /**
     * @brief Sets the booking date from a value object.
     * @param value Normalized booking date wrapper.
     */
    void setBookingDate(BookingDate value);
    /**
     * @brief Sets the booking date from raw text.
     * @param value Raw booking date text.
     */
    void setBookingDate(std::string value);
    /**
     * @brief Sets the valuta text.
     * @param value Raw valuta text.
     */
    void setValuta(std::string value);
    /**
     * @brief Sets the amount from a value object.
     * @param value Normalized money amount wrapper.
     */
    void setAmount(MoneyAmount value);
    /**
     * @brief Sets the amount from raw numeric input.
     * @param value Raw amount.
     */
    void setAmount(double value);
    /**
     * @brief Sets the transaction status.
     * @param value Transaction status value.
     */
    void setStatus(Status value);
    /**
     * @brief Sets the contract id.
     * @param value Raw contract id text.
     */
    void setContractId(std::string value);
    /**
     * @brief Sets the actor id.
     * @param value Raw actor id text.
     */
    void setActorId(std::string value);
    /**
     * @brief Sets the statement id.
     * @param value Raw statement id text.
     */
    void setStatementId(std::string value);
    /**
     * @brief Sets the allocatable flag.
     * @param value Allocatable flag.
     */
    void setAllocatable(bool value);
    /**
     * @brief Clears the linked contract id.
     */
    void clearContract();
    /**
     * @brief Clears the linked actor id.
     */
    void clearActor();
    /**
     * @brief Clears the linked statement id.
     */
    void clearStatement();
    /**
     * @brief Clears linked property ids.
     */
    void clearProperties();

    /**
     * @brief Checks whether the transaction has a given status.
     * @param value Status value to compare.
     * @return `true` when the current status matches.
     */
    [[nodiscard]] bool hasStatus(Status value) const noexcept;

    /**
     * @brief Checks whether the transaction is linked to a statement.
     * @return `true` when a statement id is set.
     */
    [[nodiscard]] bool hasStatement() const noexcept;
    /**
     * @brief Checks whether the transaction is linked to a contract.
     * @return `true` when a contract id is set.
     */
    [[nodiscard]] bool hasContract() const noexcept;
    /**
     * @brief Checks whether the transaction is linked to an actor.
     * @return `true` when an actor id is set.
     */
    [[nodiscard]] bool hasActor() const noexcept;
    /**
     * @brief Checks whether the transaction is linked to properties.
     * @return `true` when at least one property id is set.
     */
    [[nodiscard]] bool hasProperties() const noexcept;
    /**
     * @brief Returns the number of linked properties.
     * @return Property relation count.
     */
    [[nodiscard]] std::size_t propertyCount() const noexcept;
    /**
     * @brief Checks whether the transaction belongs to a statement.
     * @param value Statement id to compare.
     * @return `true` when the transaction is linked to the given statement.
     */
    [[nodiscard]] bool belongsToStatement(const std::string& value) const;
    /**
     * @brief Checks whether the transaction can be allocated.
     * @return `true` when the allocatable flag is set.
     */
    [[nodiscard]] bool isAllocatable() const noexcept;
    /**
     * @brief Advances the transaction to unverified status when allowed.
     */
    void markUnverified();
    /**
     * @brief Advances the transaction to verified status when allowed.
     */
    void markVerified();
    /**
     * @brief Advances the transaction to completed status when allowed.
     */
    void markCompleted();
    /**
     * @brief Checks whether the transaction has any relations.
     * @return `true` when statement, contract, actor or properties exist.
     */
    [[nodiscard]] bool hasRelations() const noexcept;
    /**
     * @brief Checks whether the transaction is standalone.
     * @return `true` when no relations are assigned.
     */
    [[nodiscard]] bool isStandalone() const noexcept;

    /**
     * @brief Replaces the property id list with normalized values.
     * @param value Property id list to store.
     */
    void setPropertyIds(std::vector<std::string> value);

    /**
     * @brief Adds a property id when it is not already present.
     * @param value Property id to add.
     */
    void addPropertyId(std::string value);

    /**
     * @brief Removes a property id from the transaction.
     * @param value Property id to remove.
     */
    void removePropertyId(const std::string& value);

    /**
     * @brief Checks whether a property id exists.
     * @param value Property id to search for.
     * @return `true` when the property id exists.
     */
    [[nodiscard]] bool containsPropertyId(const std::string& value) const;

private:
    std::string id_;
    std::string name_;
    std::string bookingDate_;
    std::string valuta_;
    double amount_ = 0.0;
    Status status_ = Status::Neutral;
    std::string contractId_;
    std::string actorId_;
    std::string statementId_;
    bool allocatable_ = false;
    std::vector<std::string> propertyIds_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& bookingDate() const noexcept;
    [[nodiscard]] const std::string& valuta() const noexcept;
    [[nodiscard]] const double& amount() const noexcept;
    [[nodiscard]] const Status& status() const noexcept;
    [[nodiscard]] const std::string& contractId() const noexcept;
    [[nodiscard]] const std::string& actorId() const noexcept;
    [[nodiscard]] const std::string& statementId() const noexcept;
    [[nodiscard]] const std::vector<std::string>& propertyIds() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);
};

} // namespace core::domain
