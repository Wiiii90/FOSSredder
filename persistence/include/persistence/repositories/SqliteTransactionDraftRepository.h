/**
 * @file persistence/include/persistence/repositories/SqliteTransactionDraftRepository.h
 * @brief Declares the SQLite-backed transaction draft repository.
 */

#pragma once

#include "core/ports/repositories/ITransactionDraftRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteTransactionDraftRepository : public core::ports::repositories::ITransactionDraftRepository {
public:
    /**
     * @brief Create a transaction draft repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteTransactionDraftRepository(const std::string& dbPath);

    /**
     * @brief Create a transaction draft repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteTransactionDraftRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteTransactionDraftRepository() override;

    /**
     * @brief Insert a new transaction draft.
     * @param draft Transaction draft to add.
     */
    void addTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) override;

    /**
     * @brief Retrieve all transaction drafts.
     * @return All stored transaction drafts.
     */
    std::vector<std::shared_ptr<core::application::importing::draft::TransactionDraft>> getTransactionDrafts() const override;

    /**
     * @brief Retrieve a transaction draft by identifier.
     * @param id Transaction draft identifier.
     * @return Transaction draft with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::application::importing::draft::TransactionDraft>> getTransactionDraftById(const std::string& id) const override;

    /**
     * @brief Remove a transaction draft by identifier.
     * @param id Transaction draft identifier.
     */
    void removeTransactionDraft(const std::string& id) override;

    /**
     * @brief Update an existing transaction draft.
     * @param draft Transaction draft to update.
     */
    void updateTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) override;

    /**
     * @brief Insert or update a transaction draft.
     * @param draft Transaction draft to upsert.
     */
    void upsertTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) override;

    /**
     * @brief Remove all transaction drafts.
     */
    void clearTransactionDrafts() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
