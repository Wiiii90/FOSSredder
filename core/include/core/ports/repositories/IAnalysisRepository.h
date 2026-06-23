/**
 * @file core/include/core/ports/repositories/IAnalysisRepository.h
 * @brief Repository port for core::domain::Analysis persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Analysis;
}

namespace core::ports::repositories {

class IAnalysisRepository {
public:
    /**
     * @brief Destroy the analysis repository interface.
     */
    virtual ~IAnalysisRepository() = default;

    /**
     * @brief Add a new analysis.
     * @param analysis Analysis to add.
     */
    virtual void addAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Retrieve all analyses.
     * @return All stored analyses.
     */
    virtual std::vector<std::shared_ptr<core::domain::Analysis>> getAnalyses() const = 0;

    /**
     * @brief Retrieve an analysis by identifier.
     * @param id Analysis identifier.
     * @return Analysis with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Analysis>> getAnalysisById(const std::string& id) const = 0;

    /**
     * @brief Remove an analysis by identifier.
     * @param id Analysis identifier.
     */
    virtual void removeAnalysis(const std::string& id) = 0;

    /**
     * @brief Update an existing analysis.
     * @param analysis Analysis to update.
     */
    virtual void updateAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Insert or update an analysis.
     * @param analysis Analysis to upsert.
     */
    virtual void upsertAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Remove all analyses.
     */
    virtual void clearAnalyses() = 0;
};

} // namespace core::ports::repositories
