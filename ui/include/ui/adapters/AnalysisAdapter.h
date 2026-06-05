/**
 * @file ui/include/ui/adapters/AnalysisAdapter.h
 * @brief Declares the analysis adapter used by UI workflows.
 */

#pragma once

#include <memory>
#include <vector>

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "core/ports/usecases/analysis/AnalysisRequest.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/ports/usecases/analysis/IAnalysisRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {

class AnalysisAdapter final {
public:
  /**
   * @brief Creates an analysis adapter backed by a core runner.
   * @param runner Core analysis runner port.
   */
  explicit AnalysisAdapter(
      std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner);

  /**
   * @brief Runs an analysis through the core runner.
   * @param workspace Workspace snapshot used as input.
   * @param request Analysis request.
   * @return Core analysis result.
   */
  core::ports::analysis::AnalysisResult runAnalysis(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const core::ports::analysis::AnalysisRequest& request) const;
  /**
   * @brief Builds preview transactions through the core runner.
   * @param workspace Workspace snapshot used as input.
   * @param filterSpec Serialized filter specification.
   * @return Preview result.
   */
  core::ports::analysis::AnalysisPreviewResult previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::string& filterSpec) const;
  /**
   * @brief Builds a serialized analysis filter specification from UI fields.
   * @param dateField Selected date field.
   * @param dateMode Selected date mode.
   * @param year Selected year text.
   * @param dateFrom Start date text.
   * @param dateTo End date text.
   * @param propertyIds Selected property ids.
   * @param contractTypes Selected contract types.
   * @param allocatableMode Selected allocatable mode.
   * @return Serialized filter specification.
   */
  QString buildFilterSpec(const QString& dateField, const QString& dateMode,
                          const QString& year, const QString& dateFrom,
                          const QString& dateTo,
                          const QStringList& propertyIds,
                          const QStringList& contractTypes,
                          const QString& allocatableMode) const;
  /**
   * @brief Builds analysis adjustment amounts from preview rows.
   * @param transactions Preview transaction rows.
   * @param selectedTransactionIds Selected transaction ids.
   * @param taxPercent Tax percentage applied by core.
   * @return Adjustment amount map keyed by transaction id.
   */
  QVariantMap buildAnalysisAdjustmentAmounts(
      const QVariantList& transactions,
      const QStringList& selectedTransactionIds,
      double taxPercent) const;
  /**
   * @brief Converts a QML adjustment amount map to core adjustment pairs.
   * @param amountsById Adjustment amount map keyed by transaction id.
   * @return Core adjustment amount pairs.
   */
  [[nodiscard]] core::ports::analysis::AnalysisAdjustmentAmounts
  adjustmentAmountsFromMap(const QVariantMap& amountsById) const;
  /**
   * @brief Parses user-entered percent text through the core runner.
   * @param text Percent text.
   * @param defaultValue Value used when parsing fails or no runner exists.
   * @return Parsed percent value or default value.
   */
  [[nodiscard]] double parseAnalysisPercentText(
      const QString& text,
      double defaultValue = 0.0) const;
  /**
   * @brief Applies preview-only overrides to a workspace snapshot.
   * @param workspace Workspace snapshot to mutate for preview only.
   * @param analysisId Target analysis id.
   * @param includeCalculationAdjustments Whether adjustments are included.
   * @param adjustments Adjustment amount pairs keyed by transaction id.
   */
  void applyAnalysisPreviewOverrides(
      core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::string& analysisId,
      bool includeCalculationAdjustments,
      const core::ports::analysis::AnalysisAdjustmentAmounts& adjustments) const;
  /**
   * @brief Projects table state for QML presentation.
   * @param result Analysis result.
   * @param adjustments Adjustment amount pairs keyed by transaction id.
   * @param includeCalculationAdjustments Whether adjustments are included.
   * @param unassignedLabel Label for unassigned contracts.
   * @return Core table projection.
   */
  core::ports::analysis::AnalysisTableState projectTableState(
      const core::ports::analysis::AnalysisResult& result,
      const core::ports::analysis::AnalysisAdjustmentAmounts& adjustments,
      bool includeCalculationAdjustments,
      const std::string& unassignedLabel) const;
  /**
   * @brief Returns available contract types from the core runner.
   * @param workspace Workspace snapshot used as input.
   * @return Contract type values.
   */
  std::vector<std::string> contractTypes(
      const core::ports::workspace::WorkspaceSnapshot& workspace) const;

  /**
   * @brief Maps an analysis result to a QML payload.
   * @param result Core analysis result.
   * @return QML payload map.
   */
  [[nodiscard]] QVariantMap
  mapAnalysisResult(const core::ports::analysis::AnalysisResult& result) const;
  /**
   * @brief Maps table state to a QML payload.
   * @param state Core table state.
   * @return QML payload map.
   */
  [[nodiscard]] QVariantMap mapAnalysisTableState(
      const core::ports::analysis::AnalysisTableState& state) const;
  /**
   * @brief Maps preview result to a QML payload.
   * @param preview Core preview result.
   * @return QML payload map.
   */
  [[nodiscard]] QVariantMap mapPreviewResult(
      const core::ports::analysis::AnalysisPreviewResult& preview) const;
  /**
   * @brief Maps contract types to a QML string list.
   * @param contractTypes Core contract type values.
   * @return Contract type strings.
   */
  [[nodiscard]] QStringList
  mapContractTypes(const std::vector<std::string>& contractTypes) const;

private:
  /**
   * @brief Builds a core filter selection from normalized string fields.
   * @param dateField Date field.
   * @param dateMode Date mode.
   * @param year Year text.
   * @param dateFrom Start date text.
   * @param dateTo End date text.
   * @param propertyIds Selected property ids.
   * @param contractTypes Selected contract types.
   * @param allocatableMode Allocatable mode.
   * @return Core filter selection.
   */
  core::ports::analysis::AnalysisFilterSelection filterSelectionFromFields(
      const std::string& dateField,
      const std::string& dateMode,
      const std::string& year,
      const std::string& dateFrom,
      const std::string& dateTo,
      const std::vector<std::string>& propertyIds,
      const std::vector<std::string>& contractTypes,
      const std::string& allocatableMode) const;
  /**
   * @brief Builds adjustments from QML preview rows.
   * @param transactions Preview transaction rows.
   * @param selectedTransactionIds Selected transaction ids.
   * @param taxPercent Tax percentage.
   * @return Adjustment amount pairs.
   */
  core::ports::analysis::AnalysisAdjustmentAmounts buildAnalysisAdjustments(
      const QVariantList& transactions,
      const QStringList& selectedTransactionIds,
      double taxPercent) const;
  /**
   * @brief Builds adjustments from core transaction inputs.
   * @param transactions Core adjustment transaction inputs.
   * @param selectedTransactionIds Selected transaction ids.
   * @param taxPercent Tax percentage.
   * @return Adjustment amount pairs.
   */
  core::ports::analysis::AnalysisAdjustmentAmounts buildAnalysisAdjustments(
      const std::vector<
          core::ports::analysis::AnalysisAdjustmentTransactionInput>&
          transactions,
      const std::vector<std::string>& selectedTransactionIds,
      double taxPercent) const;

  std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner_;
};

} // namespace ui::adapters
