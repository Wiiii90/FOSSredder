/**
 * @file core/include/core/ports/infra/xlsx-writer/IXlsxWriter.h
 * @brief Declares the XLSX writing port used by the export layer.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace core::ports::xlsx_writer {

/**
 * @brief Writes tabular export data to XLSX workbooks.
 */
class IXlsxWriter {
public:
    /**
     * @brief Destroys the XLSX writer port.
     */
    virtual ~IXlsxWriter() = default;

    /**
     * @brief Writes rows into a worksheet.
     * @param outputPath Target workbook path.
     * @param rows Table rows to write.
     * @param worksheetTitle Worksheet title.
     * @return True when the workbook was written successfully.
     */
    virtual bool writeTable(const std::filesystem::path& outputPath,
                            const std::vector<std::vector<std::string>>& rows,
                            const std::string& worksheetTitle) const = 0;
};

} // namespace core::ports::xlsx_writer
