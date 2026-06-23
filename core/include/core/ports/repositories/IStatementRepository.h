/**
 * @file core/include/core/ports/repositories/IStatementRepository.h
 * @brief Repository port for core::domain::Statement persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::ports::repositories {

class IStatementRepository {
public:
    /**
     * @brief Destroy the statement repository interface.
     */
    virtual ~IStatementRepository() = default;

    /**
     * @brief Add a new statement.
     * @param statement Statement to add.
     */
    virtual void addStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Retrieve all statements.
     * @return All stored statements.
     */
    virtual std::vector<std::shared_ptr<core::domain::Statement>> getStatements() const = 0;

    /**
     * @brief Retrieve a statement by identifier.
     * @param id Statement identifier.
     * @return Statement with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Statement>> getStatementById(const std::string& id) const = 0;

    /**
     * @brief Remove a statement by identifier.
     * @param id Statement identifier.
     */
    virtual void removeStatement(const std::string& id) = 0;

    /**
     * @brief Update an existing statement.
     * @param statement Statement to update.
     */
    virtual void updateStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Insert or update a statement.
     * @param statement Statement to upsert.
     */
    virtual void upsertStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Remove all statements.
     */
    virtual void clearStatements() = 0;
};

} // namespace core::ports::repositories
