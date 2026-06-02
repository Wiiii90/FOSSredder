/**
 * @file core/include/core/ports/export/IExportRunner.h
 * @brief Export use-case port consumed by UI workflows.
 */

#pragma once

#include "core/ports/export/ExportRequest.h"
#include "core/ports/export/ExportResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::exporting {

class IExportRunner {
public:
    virtual ~IExportRunner() = default;

    [[nodiscard]] virtual ExportResult
    runExport(const core::ports::workspace::WorkspaceSnapshot& workspace,
              ExportRequest request) const = 0;
};

} // namespace core::ports::exporting
