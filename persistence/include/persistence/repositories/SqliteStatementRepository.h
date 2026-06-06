/**
 * @file persistence/include/persistence/repositories/SqliteStatementRepository.h
 * @brief Declares the SQLite-backed statement repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/ports/repositories/IStatementRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteStatementRepository : public core::ports::repositories::IStatementRepository {
public:
    /**
     * @brief Create a statement repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteStatementRepository(const std::string& dbPath);

    /**
     * @brief Create a statement repository for the database at the given path.
     * @param dbPath SQLite database path.
     * @param errorReporter Error reporter used by the repository.
     */
    SqliteStatementRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Create a statement repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteStatementRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Create a statement repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     * @param errorReporter Error reporter used by the repository.
     */
    SqliteStatementRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteStatementRepository() override;

    void addStatement(const std::shared_ptr<core::domain::Statement>& statement) override;
    std::vector<std::shared_ptr<core::domain::Statement>> getStatements() const override;
    std::optional<std::shared_ptr<core::domain::Statement>> getStatementById(const std::string& id) const override;
    void removeStatement(const std::string& id) override;
    void updateStatement(const std::shared_ptr<core::domain::Statement>& statement) override;

    void upsertStatement(const std::shared_ptr<core::domain::Statement>& statement) override;
    void clearStatements() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
