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
namespace core::ports::analysis_image_renderer {
class IAnalysisImageRenderer;
}

namespace core::application::exporting {

core::ports::exporting::ExportResult exportObjectRequests(
    const core::domain::catalog::WorkspaceCatalog &state,
    const core::ports::exporting::ExportRequest &request,
    const std::shared_ptr<core::ports::archive::IArchive> &archive,
    const std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> &xlsxWriter,
    const std::shared_ptr<
        core::ports::analysis_image_renderer::IAnalysisImageRenderer>
        &imageRenderer);

} // namespace core::application::exporting
