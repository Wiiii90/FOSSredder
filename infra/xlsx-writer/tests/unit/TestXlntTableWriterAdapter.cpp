/**
 * @file infra/xlsx-writer/tests/unit/TestXlntTableWriterAdapter.cpp
 * @brief Tests for the XLSX table writer adapter.
 */

#include <gtest/gtest.h>

#include "xlsx-writer/XlntTableWriterAdapter.h"

#include <zip.h>
#include <xlnt/xlnt.hpp>

namespace infra::xlsx_writer::tests {
namespace {

std::filesystem::path makeTempDir(const std::string& stem) {
    auto dir = std::filesystem::temp_directory_path() / stem;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

TEST(XlntTableWriterAdapterTest, WritesWorkbookWithExpectedCellValues) {
    const auto tempDir = makeTempDir("fossredder_xlsx_writer_test");
    const auto outputPath = tempDir / "out.xlsx";
    const std::vector<std::vector<std::string>> rows = {
        {"Name", "Value"},
        {"Rent", "120.50"},
        {"Formula", "=SUM(1,2)"},
    };

    XlntTableWriterAdapter adapter;
    ASSERT_TRUE(adapter.writeTable(outputPath, rows, "Analysis"));
    ASSERT_TRUE(std::filesystem::exists(outputPath));
    ASSERT_GT(std::filesystem::file_size(outputPath), 0u);

    xlnt::workbook workbook;
    workbook.load(outputPath.string());
    auto sheet = workbook.active_sheet();

    EXPECT_EQ(sheet.title(), "Analysis");
    EXPECT_EQ(sheet.cell("A1").to_string(), "Name");
    EXPECT_EQ(sheet.cell("B1").to_string(), "Value");
    EXPECT_EQ(sheet.cell("A2").to_string(), "Rent");
    EXPECT_EQ(sheet.cell("B2").to_string(), "120.50");
    EXPECT_EQ(sheet.cell("A3").to_string(), "Formula");

    int err = 0;
    zip_t* archive = zip_open(outputPath.string().c_str(), ZIP_RDONLY, &err);
    ASSERT_NE(archive, nullptr);

    zip_int64_t index = zip_name_locate(archive, "xl/worksheets/sheet1.xml", 0);
    ASSERT_GE(index, 0);

    zip_file_t* sheetFile = zip_fopen_index(archive, index, 0);
    ASSERT_NE(sheetFile, nullptr);

    std::string xml;
    char buffer[4096];
    zip_int64_t bytesRead = 0;
    while ((bytesRead = zip_fread(sheetFile, buffer, sizeof(buffer))) > 0) {
        xml.append(buffer, static_cast<size_t>(bytesRead));
    }

    zip_fclose(sheetFile);
    zip_close(archive);

    EXPECT_NE(xml.find("<f>SUM(1,2)</f>"), std::string::npos);
}

TEST(XlntTableWriterAdapterTest, RejectsUnwritableOutputPaths) {
    const auto tempDir = makeTempDir("fossredder_xlsx_writer_invalid");
    const auto outputDir = tempDir / "as_directory.xlsx";
    std::filesystem::create_directories(outputDir);

    XlntTableWriterAdapter adapter;
    EXPECT_FALSE(adapter.writeTable(outputDir, {{"A"}}, "Sheet"));
}

} // namespace
} // namespace infra::xlsx_writer::tests
