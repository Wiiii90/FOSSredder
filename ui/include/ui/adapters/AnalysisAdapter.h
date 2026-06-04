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
  explicit AnalysisAdapter(
      std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner);

  core::ports::analysis::AnalysisResult runAnalysis(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const core::ports::analysis::AnalysisRequest& request) const;
  core::ports::analysis::AnalysisPreviewResult previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::string& filterSpec) const;
  core::ports::analysis::AnalysisFilterSelection filterSelectionFromFields(
      const std::string& dateField,
      const std::string& dateMode,
      const std::string& year,
      const std::string& dateFrom,
      const std::string& dateTo,
      const std::vector<std::string>& propertyIds,
      const std::vector<std::string>& contractTypes,
      const std::string& allocatableMode) const;
  QString buildFilterSpec(const QString& dateField, const QString& dateMode,
                          const QString& year, const QString& dateFrom,
                          const QString& dateTo,
                          const QStringList& propertyIds,
                          const QStringList& contractTypes,
                          const QString& allocatableMode) const;
  std::string buildAnalysisConfigJson(
      const core::ports::analysis::AnalysisConfigInput& input) const;
  QString buildAnalysisConfigJson(const QString& type, const QString& plotType,
                                  const QString& plotMeasure,
                                  const QStringList& propertyIds,
                                  const QStringList& contractTypes,
                                  double taxPercent) const;
  std::string buildAnalysisAdjustmentsJson(
      const std::vector<
          core::ports::analysis::AnalysisAdjustmentTransactionInput>&
          transactions,
      const std::vector<std::string>& selectedTransactionIds,
      double taxPercent) const;
  std::string buildAnalysisAdjustmentsJson(
      const QVariantList& transactions,
      const QStringList& selectedTransactionIds,
      double taxPercent) const;
  void applyAnalysisPreviewOverrides(
      core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::string& analysisId,
      bool includeCalculationAdjustments,
      const std::string& adjustmentsJson) const;
  core::ports::analysis::AnalysisTableState projectTableState(
      const core::ports::analysis::AnalysisResult& result,
      const std::string& adjustmentsJson,
      bool includeCalculationAdjustments,
      const std::string& unassignedLabel) const;
  std::vector<std::string> contractTypes(
      const core::ports::workspace::WorkspaceSnapshot& workspace) const;

  [[nodiscard]] QVariantMap
  mapAnalysisResult(const core::ports::analysis::AnalysisResult& result) const;
  [[nodiscard]] QVariantMap mapAnalysisTableState(
      const core::ports::analysis::AnalysisTableState& state) const;
  [[nodiscard]] QVariantMap mapFilterSelection(
      const core::ports::analysis::AnalysisFilterSelection& selection) const;
  [[nodiscard]] QVariantMap mapPreviewResult(
      const core::ports::analysis::AnalysisPreviewResult& preview) const;
  [[nodiscard]] QStringList
  mapContractTypes(const std::vector<std::string>& contractTypes) const;

private:
  std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner_;
};

} // namespace ui::adapters
