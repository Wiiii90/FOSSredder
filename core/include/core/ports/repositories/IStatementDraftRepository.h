/**
 * @file core/include/core/ports/repositories/IStatementDraftRepository.h
 * @brief Repository port for statement draft persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::application::importing::draft {
class StatementDraft;
}

namespace core::ports::repositories {

class IStatementDraftRepository {
public:
    /**
     * @brief Destroy the statement draft repository interface.
     */
    virtual ~IStatementDraftRepository() = default;

    /**
     * @brief Add a new statement draft.
     * @param draft Statement draft to add.
     */
    virtual void addStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) = 0;

    /**
     * @brief Retrieve all statement drafts.
     * @return All stored statement drafts.
     */
    virtual std::vector<std::shared_ptr<core::application::importing::draft::StatementDraft>> getStatementDrafts() const = 0;

    /**
     * @brief Retrieve a statement draft by identifier.
     * @param id Statement draft identifier.
     * @return Statement draft with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::application::importing::draft::StatementDraft>> getStatementDraftById(const std::string& id) const = 0;

    /**
     * @brief Remove a statement draft by identifier.
     * @param id Statement draft identifier.
     */
    virtual void removeStatementDraft(const std::string& id) = 0;

    /**
     * @brief Update an existing statement draft.
     * @param draft Statement draft to update.
     */
    virtual void updateStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) = 0;

    /**
     * @brief Insert or update a statement draft.
     * @param draft Statement draft to upsert.
     */
    virtual void upsertStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft) = 0;

    /**
     * @brief Remove all statement drafts.
     */
    virtual void clearStatementDrafts() = 0;

    /**
     * @brief Replace the stored statement draft with the provided draft.
     * @param draft Statement draft to save.
     */
    void saveStatementDraft(const std::shared_ptr<core::application::importing::draft::StatementDraft>& draft)
    {
        clearStatementDrafts();
        if (draft) {
            upsertStatementDraft(draft);
        }
    }

    /**
     * @brief Retrieve the first stored statement draft, if any.
     * @return The first stored statement draft, if one exists.
     */
    std::optional<std::shared_ptr<core::application::importing::draft::StatementDraft>> getStatementDraft() const
    {
        const auto drafts = getStatementDrafts();
        if (drafts.empty()) {
            return std::nullopt;
        }
        return drafts.front();
    }

    /**
     * @brief Remove the stored statement draft.
     */
    void clearStatementDraft()
    {
        clearStatementDrafts();
    }
};

} // namespace core::ports::repositories
