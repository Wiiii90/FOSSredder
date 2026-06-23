/**
 * @file core/include/core/ports/usecases/export/ExportResult.h
 * @brief Declares the export execution result port contract.
 */

#pragma once

#include "core/ports/usecases/export/ExportRequest.h"

#include <string>

namespace core::ports::exporting {

struct ExportResult {
    bool success = false;
    ExportStatus status = ExportStatus::InternalError;
    ExportFormat actualFormat = ExportFormat::Xlsx;
    std::string resolvedOutputPath;
    std::string errorCode;
    std::string message;
};

} // namespace core::ports::exporting
