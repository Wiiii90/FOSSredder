/**
 * @file ui/include/ui/adapters/ExportAdapter.h
 * @brief Declares the export adapter used by UI workflows.
 */

#pragma once

#include <memory>

#include <QString>
#include <QVariantMap>

#include "core/ports/usecases/export/ExportRequest.h"
#include "core/ports/usecases/export/IExportRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {

class ExportAdapter final {
public:
  explicit ExportAdapter(
      std::shared_ptr<core::ports::exporting::IExportRunner> runner);

  core::ports::exporting::ExportResult
  runExport(const core::ports::workspace::WorkspaceSnapshot& workspace,
            core::ports::exporting::ExportRequest request) const;

  [[nodiscard]] core::ports::exporting::ExportFormat
  exportFormatFromQmlIndex(int formatIndex) const;

  void applySelectionPayload(core::ports::exporting::ExportRequest& request,
                             const QVariantMap& selectionPayload) const;

  [[nodiscard]] core::ports::exporting::ExportRequest
  buildExportRequest(int formatIndex, const QString& path, bool includeFormulas,
                     const QString& locale,
                     const QVariantMap& selectionPayload) const;

private:
  std::shared_ptr<core::ports::exporting::IExportRunner> runner_;
};

} // namespace ui::adapters
