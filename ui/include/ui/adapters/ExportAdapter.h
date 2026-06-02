/**
 * @file ui/include/ui/adapters/ExportAdapter.h
 * @brief Declares the export adapter used by UI workflows.
 */

#pragma once

#include <memory>

#include <QString>

#include "core/ports/export/ExportRequest.h"
#include "core/ports/export/IExportRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {

class ExportAdapter final : public core::ports::exporting::IExportRunner {
public:
  explicit ExportAdapter(
      std::shared_ptr<core::ports::exporting::IExportRunner> runner);

  core::ports::exporting::ExportResult runExport(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      core::ports::exporting::ExportRequest request) const override;

  [[nodiscard]] core::ports::exporting::ExportFormat
  exportFormatFromQmlIndex(int formatIndex) const;

  void applySelectionPayload(core::ports::exporting::ExportRequest &request,
                             const QString &selectionPayloadJson) const;

  [[nodiscard]] core::ports::exporting::ExportRequest
  buildExportRequest(int formatIndex, const QString &path,
                     bool includeFormulas, const QString &locale,
                     const QString &selectionPayload) const;

private:
  std::shared_ptr<core::ports::exporting::IExportRunner> runner_;
};

} // namespace ui::adapters
