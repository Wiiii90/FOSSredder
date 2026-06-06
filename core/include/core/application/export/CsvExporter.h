/**
 * @file core/include/core/application/export/CsvExporter.h
 * @brief Declares CSV export for the property/contract-type matrix.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/usecases/export/ExportRequest.h"
#include "core/ports/usecases/export/ExportResult.h"

namespace core::application::exporting {

/**
 * @brief Exports the property/contract matrix to CSV.
 */
class CsvExporter {
public:
  /**
     * @brief Exports the property/contract matrix to a CSV file.
     *
   * @param request Export request describing output location and source
   * snapshot.
     * @return Export result describing success or failure.
 */
  core::ports::exporting::ExportResult
  exportData(const core::domain::catalog::WorkspaceCatalog &state,
             const core::ports::exporting::ExportRequest &request) const;
};

} // namespace core::application::exporting
