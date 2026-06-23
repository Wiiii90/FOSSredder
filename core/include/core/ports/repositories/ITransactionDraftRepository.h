/**
 * @file core/include/core/ports/repositories/ITransactionDraftRepository.h
 * @brief Repository port for transaction draft persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::application::importing::draft {
class TransactionDraft;
}

namespace core::ports::repositories {

class ITransactionDraftRepository {
public:
    /**
     * @brief Destroy the transaction draft repository interface.
     */
    virtual ~ITransactionDraftRepository() = default;

    /**
     * @brief Add a new transaction draft.
     * @param draft Transaction draft to add.
     */
    virtual void addTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) = 0;

    /**
     * @brief Retrieve all transaction drafts.
     * @return All stored transaction drafts.
     */
    virtual std::vector<std::shared_ptr<core::application::importing::draft::TransactionDraft>> getTransactionDrafts() const = 0;

    /**
     * @brief Retrieve a transaction draft by identifier.
     * @param id Transaction draft identifier.
     * @return Transaction draft with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::application::importing::draft::TransactionDraft>> getTransactionDraftById(const std::string& id) const = 0;

    /**
     * @brief Remove a transaction draft by identifier.
     * @param id Transaction draft identifier.
     */
    virtual void removeTransactionDraft(const std::string& id) = 0;

    /**
     * @brief Update an existing transaction draft.
     * @param draft Transaction draft to update.
     */
    virtual void updateTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) = 0;

    /**
     * @brief Insert or update a transaction draft.
     * @param draft Transaction draft to upsert.
     */
    virtual void upsertTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft) = 0;

    /**
     * @brief Remove all transaction drafts.
     */
    virtual void clearTransactionDrafts() = 0;

    /**
     * @brief Replace the stored transaction drafts with the provided drafts.
     * @param drafts Transaction drafts to save.
     */
    void saveTransactionDrafts(const std::vector<std::shared_ptr<core::application::importing::draft::TransactionDraft>>& drafts)
    {
        clearTransactionDrafts();
        for (const auto& draft : drafts) {
            if (draft) {
                upsertTransactionDraft(draft);
            }
        }
    }
};

} // namespace core::ports::repositories
