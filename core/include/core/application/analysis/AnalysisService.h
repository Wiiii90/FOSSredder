/**
 * @file core/include/core/application/analysis/AnalysisService.h
 * @brief Declares the application service that resolves and executes analyses.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Transaction.h"
#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/ports/usecases/analysis/IAnalysisRunner.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::ports::analysis_image_renderer {
class IAnalysisImageRenderer;
} // namespace core::ports::analysis_image_renderer

namespace core::application::analysis {

/**
 * @brief Coordinates analysis lookup, execution, and transaction preview
 * filtering.
 */
class AnalysisService : public core::ports::analysis::IAnalysisRunner {
public:
  AnalysisService() = default;
  explicit AnalysisService(
      std::shared_ptr<
          core::ports::analysis_image_renderer::IAnalysisImageRenderer>
          imageRenderer);
  ~AnalysisService() = default;

  AnalysisService(const AnalysisService &) = delete;
  AnalysisService &operator=(const AnalysisService &) = delete;
  AnalysisService(AnalysisService &&) noexcept = default;
  AnalysisService &operator=(AnalysisService &&) noexcept = default;

  /**
     * @brief Runs one analysis request against the supplied workspace
   * state.
     * @param state Current workspace catalog snapshot.
   * @param request Analysis request containing identifier and optional filter
   * override.
   * @return Materialized analysis result, or an unfound result when the
   * identifier is missing.
   */
  core::ports::analysis::AnalysisResult
  runAnalysis(const core::domain::catalog::WorkspaceCatalog &state,
              const core::ports::analysis::AnalysisRequest &request) const;

  /**
     * @brief Runs one analysis request against a workspace port
   * snapshot.
     */
  core::ports::analysis::AnalysisResult runAnalysis(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const core::ports::analysis::AnalysisRequest &request) const override;

  /**
   * @brief Resolves an analysis by identifier and executes it.
   * @param state Current workspace catalog snapshot.
   * @param analysisId Identifier of the stored analysis definition.
   * @param filterSpec Optional filter override applied instead of the stored
   * filter.
   * @return Materialized analysis result, or an unfound result when the
   * identifier is missing.
   */
  core::ports::analysis::AnalysisResult
  runAnalysisById(const core::domain::catalog::WorkspaceCatalog &state,
                  const std::string &analysisId,
                  const std::string &filterSpec = {}) const;

  /**
   * @brief Computes a concrete analysis definition against the supplied
   * workspace state.
   * @param analysis Analysis definition to execute.
   * @param state Current workspace catalog snapshot.
   * @param filterSpec Optional filter override applied during execution.
   * @return Fully materialized analysis result.
   */
  core::ports::analysis::AnalysisResult
  computeAnalysis(const core::domain::Analysis &analysis,
                  const core::domain::catalog::WorkspaceCatalog &state,
                  const std::string &filterSpec = {}) const;

  /**
   * @brief Returns the transactions matched by one analysis filter
   * specification.
   * @param state Current workspace catalog snapshot.
   * @param filterSpec Raw analysis filter specification.
   * @return Transactions that satisfy the parsed filter.
   */
  std::vector<std::shared_ptr<core::domain::Transaction>>
  filterTransactions(const core::domain::catalog::WorkspaceCatalog &state,
                     const std::string &filterSpec = {}) const;

  /**
     * @brief Returns port-native transaction preview rows for one filter
   * specification.
     */
  core::ports::analysis::AnalysisPreviewResult
  previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const core::ports::analysis::AnalysisFilterSelection &filter) const override;

  core::ports::analysis::AnalysisFilterSelection filterSelectionFromFields(
      const std::string &dateField, const std::string &dateMode,
      const std::string &year, const std::string &dateFrom,
      const std::string &dateTo, const std::vector<std::string> &propertyIds,
      const std::vector<std::string> &contractTypes,
      const std::string &allocatableMode) const override;

  /**
   * @brief Parses a serialized filter spec through the analysis use case.
   * @param filterSpec Serialized filter specification.
   * @return Parsed filter selection.
   */
  core::ports::analysis::AnalysisFilterSelection
  parseFilterSpec(const std::string &filterSpec) const override;

  /**
   * @brief Serializes a filter selection through the analysis use case.
   * @param selection Filter selection.
   * @return Serialized filter specification.
   */
  std::string buildFilterSpec(
      const core::ports::analysis::AnalysisFilterSelection &selection)
      const override;

  std::string buildAnalysisConfigJson(
      const core::ports::analysis::AnalysisConfigInput &input) const override;

  core::ports::analysis::AnalysisAdjustmentAmounts buildAnalysisAdjustments(
      const std::vector<
          core::ports::analysis::AnalysisAdjustmentTransactionInput>
          &transactions,
      const std::vector<std::string> &selectedTransactionIds,
      double taxPercent) const override;

  std::optional<double>
  parseAnalysisPercentText(const std::string &text) const override;

  void applyAnalysisPreviewOverrides(
      core::ports::workspace::WorkspaceSnapshot &workspace,
      const std::string &analysisId, bool includeCalculationAdjustments,
      const core::ports::analysis::AnalysisAdjustmentAmounts &adjustments)
      const override;

  core::ports::analysis::AnalysisTableState projectTableState(
      const core::ports::analysis::AnalysisResult &result,
      const core::ports::analysis::AnalysisAdjustmentAmounts &adjustments,
      bool includeCalculationAdjustments,
      const std::string &unassignedLabel) const override;

  std::vector<std::string> contractTypes(
      const core::ports::workspace::WorkspaceSnapshot &workspace)
      const override;

private:
  core::ports::analysis::AnalysisResult
  withRenderedArtifacts(const core::ports::analysis::AnalysisRequest &request,
                        core::ports::analysis::AnalysisResult result) const;

  std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer>
      imageRenderer_;
};

} // namespace core::application::analysis

namespace core::application {
using AnalysisService = analysis::AnalysisService;
}
