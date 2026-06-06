/**
 * @file persistence/include/persistence/repositories/SqliteStatementDraftRepository.h
 * @brief Declares the SQLite-backed statement draft repository.
 */

#pragma once

#include "core/ports/repositories/IStatementDraftRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteStatementDraftRepository : public core::ports::repositories::IStatementDraftRepository {
public:
    /**
     * @brief Create a statement draft repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteStatementDraftRepository(const std::string& dbPath);

    /**
     * @brief Create a statement draft repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteStatementDraftRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteStatementDraftRepository() override;

    /**
     * @brief Insert a new statement draft.
     * @param draft Statement draft to add.
     */
    void addStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) override;

    /**
     * @brief Retrieve all statement drafts.
     * @return All stored statement drafts.
     */
    std::vector<std::shared_ptr<core::application::importing::draft::StatementDraft>> getStatementDrafts() const override;

    /**
     * @brief Retrieve a statement draft by identifier.
     * @param id Statement draft identifier.
     * @return Statement draft with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::application::importing::draft::StatementDraft>> getStatementDraftById(const std::string& id) const override;

    /**
     * @brief Remove a statement draft by identifier.
     * @param id Statement draft identifier.
     */
    void removeStatementDraft(const std::string& id) override;

    /**
     * @brief Update an existing statement draft.
     * @param draft Statement draft to update.
     */
    void updateStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) override;

    /**
     * @brief Insert or update a statement draft.
     * @param draft Statement draft to upsert.
     */
    void upsertStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) override;

    /**
     * @brief Remove all statement drafts.
     */
    void clearStatementDrafts() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
