/**
 * @file core/include/core/application/export/XlsxExporter.h
 * @brief Declares XLSX export for the property/contract-type matrix.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/export/ExportRequest.h"
#include "core/ports/export/ExportResult.h"

#include <memory>

namespace core::ports::xlsx_writer {
class IXlsxWriter;
}

namespace core::application::exporting {

/**
 * @brief Exports the property/contract matrix to XLSX.
 */
class XlsxExporter {
public:
  explicit XlsxExporter(
      std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> writer = {});

  /**
   * @brief Exports the property/contract matrix to an XLSX file.
   * @param request Export request describing output location and source
   * snapshot.
   * @return Export result describing success or failure.
   */
  core::ports::exporting::ExportResult
  exportData(const core::domain::catalog::WorkspaceCatalog &state,
             const core::ports::exporting::ExportRequest &request) const;

private:
  std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> writer_;
};

} // namespace core::application::exporting
