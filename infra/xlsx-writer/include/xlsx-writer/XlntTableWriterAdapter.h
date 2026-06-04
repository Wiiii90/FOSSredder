/**
 * @file infra/xlsx-writer/include/xlsx-writer/XlntTableWriterAdapter.h
 * @brief Declares the XLSX writer adapter used by export features.
 */

#pragma once

#include "core/ports/infra/xlsx-writer/IXlsxWriter.h"

namespace infra::xlsx_writer {

class XlntTableWriterAdapter final : public core::ports::xlsx_writer::IXlsxWriter {
public:
    bool writeTable(const std::filesystem::path& outputPath,
                    const std::vector<std::vector<std::string>>& rows,
                    const std::string& worksheetTitle) const override;
};

} // namespace infra::xlsx_writer
