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
  /**
   * @brief Creates an export adapter backed by a core runner.
   * @param runner Core export runner port.
   */
  explicit ExportAdapter(
      std::shared_ptr<core::ports::exporting::IExportRunner> runner);

  /**
   * @brief Runs an export through the core runner.
   * @param workspace Workspace snapshot used as input.
   * @param request Export request.
   * @return Export result.
   */
  core::ports::exporting::ExportResult
  runExport(const core::ports::workspace::WorkspaceSnapshot& workspace,
            core::ports::exporting::ExportRequest request) const;

  /**
   * @brief Builds an export request from QML-facing fields.
   * @param formatIndex QML export format index.
   * @param path Output path.
   * @param includeFormulas Whether formulas should be included.
   * @param locale Export locale.
   * @param selectionPayload Selected export objects.
   * @return Core export request.
   */
  [[nodiscard]] core::ports::exporting::ExportRequest
  buildExportRequest(int formatIndex, const QString& path, bool includeFormulas,
                     const QString& locale,
                     const QVariantMap& selectionPayload) const;

private:
  /**
   * @brief Maps a QML export format index to the core enum.
   * @param formatIndex QML export format index.
   * @return Core export format.
   */
  [[nodiscard]] core::ports::exporting::ExportFormat
  exportFormatFromQmlIndex(int formatIndex) const;
  /**
   * @brief Applies selected export objects to a core request.
   * @param request Request to mutate.
   * @param selectionPayload QML selection payload.
   */
  void applySelectionPayload(core::ports::exporting::ExportRequest& request,
                             const QVariantMap& selectionPayload) const;

  std::shared_ptr<core::ports::exporting::IExportRunner> runner_;
};

} // namespace ui::adapters
