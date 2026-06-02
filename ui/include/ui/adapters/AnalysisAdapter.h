/**
 * @file ui/include/ui/adapters/AnalysisAdapter.h
 * @brief Declares the analysis adapter used by UI workflows.
 */

#pragma once

#include <memory>
#include <vector>

#include <QStringList>
#include <QVariantMap>

#include "core/ports/analysis/AnalysisRequest.h"
#include "core/ports/analysis/AnalysisResult.h"
#include "core/ports/analysis/IAnalysisRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {

class AnalysisAdapter final : public core::ports::analysis::IAnalysisRunner {
public:
  explicit AnalysisAdapter(
      std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner);

  core::ports::analysis::AnalysisResult runAnalysis(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const core::ports::analysis::AnalysisRequest &request) const override;
  std::vector<core::ports::analysis::AnalysisPreviewTransaction>
  previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const std::string &filterSpec) const override;

  [[nodiscard]] QVariantMap
  mapAnalysisResult(const core::ports::analysis::AnalysisResult &result) const;
  [[nodiscard]] QVariantMap mapFilterSelection(
      const core::ports::analysis::AnalysisFilterSelection &selection) const;
  [[nodiscard]] QVariantMap mapPreviewTransactions(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const std::vector<core::ports::analysis::AnalysisPreviewTransaction>
          &transactions) const;
  [[nodiscard]] QStringList
  mapContractTypes(const std::vector<std::string> &contractTypes) const;

private:
  std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner_;
};

} // namespace ui::adapters
