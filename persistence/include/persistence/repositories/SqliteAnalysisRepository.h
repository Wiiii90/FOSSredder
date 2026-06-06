/**
 * @file persistence/include/persistence/repositories/SqliteAnalysisRepository.h
 * @brief Declares the SQLite-backed analysis repository.
 */

#pragma once

#include "core/ports/repositories/IAnalysisRepository.h"
#include "persistence/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteAnalysisRepository : public core::ports::repositories::IAnalysisRepository, public ISqlConnectionProvider {
public:
    /**
     * @brief Create an analysis repository for the database at the given path.
     * @param dbPath SQLite database path.
     */
    explicit SqliteAnalysisRepository(const std::string& dbPath);

    /**
     * @brief Create an analysis repository using an existing database handle wrapper.
     * @param db Shared SQLite database wrapper.
     */
    explicit SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db);

    /**
     * @brief Destroy the repository.
     */
    ~SqliteAnalysisRepository() override;

    /**
     * @brief Insert a new analysis.
     * @param analysis Analysis to add.
     */
    void addAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;

    /**
     * @brief Retrieve all analyses.
     * @return All stored analyses.
     */
    std::vector<std::shared_ptr<core::domain::Analysis>> getAnalyses() const override;

    /**
     * @brief Retrieve an analysis by identifier.
     * @param id Analysis identifier.
     * @return Analysis with the requested identifier, if found.
     */
    std::optional<std::shared_ptr<core::domain::Analysis>> getAnalysisById(const std::string& id) const override;

    /**
     * @brief Remove an analysis by identifier.
     * @param id Analysis identifier.
     */
    void removeAnalysis(const std::string& id) override;

    /**
     * @brief Update an existing analysis.
     * @param analysis Analysis to update.
     */
    void updateAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;

    /**
     * @brief Insert or update an analysis.
     * @param analysis Analysis to upsert.
     */
    void upsertAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;

    /**
     * @brief Remove all analyses.
     */
    void clearAnalyses() override;

    /**
     * @brief Return the SQLite handle.
     * @return SQLite database handle.
     */
    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
