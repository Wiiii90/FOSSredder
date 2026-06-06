/**
 * @file infra/xlsx-writer/src/XlntTableWriterAdapter.cpp
 * @brief Implements XLSX table writing using xlnt.
 */

#include "xlsx-writer/XlntTableWriterAdapter.h"

#include <xlnt/xlnt.hpp>

namespace infra::xlsx_writer {

bool XlntTableWriterAdapter::writeTable(const std::filesystem::path& outputPath,
                                        const std::vector<std::vector<std::string>>& rows,
                                        const std::string& worksheetTitle) const
{
    try {
        xlnt::workbook workbook;
        xlnt::worksheet worksheet = workbook.active_sheet();
        worksheet.title(worksheetTitle);

        for (size_t row = 0; row < rows.size(); ++row) {
            const auto& columns = rows[row];
            for (size_t column = 0; column < columns.size(); ++column) {
                auto cell = worksheet.cell(static_cast<int>(column + 1), static_cast<int>(row + 1));
                if (!columns[column].empty() && columns[column].front() == '=') {
                    cell.formula(columns[column].substr(1));
                } else {
                    cell.value(columns[column]);
                }
            }
        }

        workbook.save(outputPath.string());
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace infra::xlsx_writer
