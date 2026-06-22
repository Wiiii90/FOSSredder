/**
 * @file core/src/application/export/ObjectExportExecutor.h
 * @brief Declares object export execution helpers.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/usecases/export/ExportRequest.h"
#include "core/ports/usecases/export/ExportResult.h"

#include <memory>

namespace core::ports::archive {
class IArchive;
}
namespace core::ports::xlsx_writer {
class IXlsxWriter;
}
namespace core::ports::analysis_rendering {
class IAnalysisRenderer;
}

namespace core::application::exporting {

/**
 * @brief Export selected workspace objects according to an export request.
 * @param state Workspace catalog snapshot to export from.
 * @param request Object export request containing output and format options.
 * @param archive Optional archive adapter used for packaged exports.
 * @param xlsxWriter Optional XLSX writer adapter used for spreadsheet output.
 * @param imageRenderer Optional analysis renderer used for image output.
 * @return Export result containing status, message, and output paths.
 */
core::ports::exporting::ExportResult exportObjectRequests(
    const core::domain::catalog::WorkspaceCatalog &state,
    const core::ports::exporting::ExportRequest &request,
    const std::shared_ptr<core::ports::archive::IArchive> &archive,
    const std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> &xlsxWriter,
    const std::shared_ptr<
        core::ports::analysis_rendering::IAnalysisRenderer>
        &imageRenderer);

} // namespace core::application::exporting
