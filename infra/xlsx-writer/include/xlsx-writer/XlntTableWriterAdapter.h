/**
 * @file infra/xlsx-writer/include/xlsx-writer/XlntTableWriterAdapter.h
 * @brief Declares the XLSX writer adapter used by export features.
 */

#pragma once

#include "core/ports/infra/xlsx-writer/IXlsxWriter.h"

#include <memory>

namespace core::errors {
class IErrorReporter;
}

namespace infra::xlsx_writer {

class XlntTableWriterAdapter final : public core::ports::xlsx_writer::IXlsxWriter {
public:
    explicit XlntTableWriterAdapter(std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);

    bool writeTable(const std::filesystem::path& outputPath,
                    const std::vector<std::vector<std::string>>& rows,
                    const std::string& worksheetTitle) const override;

private:
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
};

} // namespace infra::xlsx_writer
