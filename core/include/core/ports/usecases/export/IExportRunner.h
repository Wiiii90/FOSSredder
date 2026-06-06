/**
 * @file core/include/core/ports/usecases/export/IExportRunner.h
 * @brief Export use-case port consumed by UI workflows.
 */

#pragma once

#include "core/ports/usecases/export/ExportRequest.h"
#include "core/ports/usecases/export/ExportResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::exporting {

class IExportRunner {
public:
    /**
     * @brief Destroys the export runner port.
     */
    virtual ~IExportRunner() = default;

    /**
     * @brief Exports workspace data according to the provided request.
     * @param workspace Workspace data used as export input.
     * @param request Export target, format, and object selection.
     * @return Export result with generated artifact information.
     */
    [[nodiscard]] virtual ExportResult
    runExport(const core::ports::workspace::WorkspaceSnapshot& workspace,
              ExportRequest request) const = 0;
};

} // namespace core::ports::exporting
