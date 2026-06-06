/**
 * @file core/tests/application/export/TestExportService.cpp
 * @brief Tests export service object-request format coverage for
 * CSV/XLSX/JPG/PNG.
 */

#include <gtest/gtest.h>

#include "core/application/export/ExportService.h"
#include "core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h"
#include "core/ports/infra/archive/IArchive.h"
#include "core/ports/usecases/export/ExportRequest.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "core/ports/infra/xlsx-writer/IXlsxWriter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

namespace core::application::exporting {

namespace {

using core::ports::exporting::AnalysisExportFormat;
using core::ports::exporting::ExportObjectType;
using core::ports::exporting::ExportRequest;
using core::ports::exporting::ExportStatus;
using core::ports::exporting::PackageFormat;

class FakeXlsxWriter final : public core::ports::xlsx_writer::IXlsxWriter {
public:
  mutable std::vector<std::vector<std::string>> lastRows;

  bool writeTable(const std::filesystem::path &outputPath,
                  const std::vector<std::vector<std::string>> &rows,
                  const std::string &worksheetTitle) const override {
    lastRows = rows;
    (void)worksheetTitle;
    std::ofstream out(outputPath, std::ios::binary);
    out << "xlsx-ok";
    return static_cast<bool>(out);
  }
};

class FakeAnalysisImageRenderer final
    : public core::ports::analysis_image_renderer::IAnalysisImageRenderer {
public:
  bool writeAnalysisImage(
      const std::filesystem::path &outputPath, const std::string &title,
      const core::ports::analysis::AnalysisResult &result) const override {
    (void)title;
    (void)result;
    std::ofstream out(outputPath, std::ios::binary);
    out << "image-ok";
    return static_cast<bool>(out);
  }
};

class FakeArchive final : public core::ports::archive::IArchive {
public:
  mutable int calls = 0;

  bool create(const std::filesystem::path &sourceDirectory,
              const std::filesystem::path &outputArchive,
              core::ports::exporting::PackageFormat format) const override {
    (void)sourceDirectory;
    (void)format;
    ++calls;
    std::ofstream out(outputArchive, std::ios::binary);
    out << "zip-ok";
    return static_cast<bool>(out);
  }
};

std::vector<std::string> splitCsvHeader(std::string line) {
  if (line.size() >= 3 &&
      static_cast<unsigned char>(line[0]) == 0xEF &&
      static_cast<unsigned char>(line[1]) == 0xBB &&
      static_cast<unsigned char>(line[2]) == 0xBF) {
    line.erase(0, 3);
  }

  std::vector<std::string> columns;
  std::stringstream stream(line);
  std::string column;
  while (std::getline(stream, column, ';')) {
    columns.push_back(column);
  }
  return columns;
}

core::ports::workspace::WorkspaceSnapshot buildState() {
  core::ports::workspace::WorkspaceSnapshot state;

  core::ports::workspace::AnalysisSnapshot analysisTable;
  analysisTable.id = "analysis-table";
  analysisTable.name = "Table Export";
  analysisTable.type = "tab";
  analysisTable.exportFormat = "csv";
  core::ports::workspace::TransactionSnapshot tableTx1;
  tableTx1.id = "tx-1";
  tableTx1.name = "Rent";
  tableTx1.bookingDate = "2026-01-31";
  tableTx1.amount = 100.0;
  tableTx1.contractId = "contract-1";
  tableTx1.contractType = "rent";
  tableTx1.propertyIds = {"property-1"};
  tableTx1.propertyNames = {"Building A"};
  tableTx1.allocatable = true;
  core::ports::workspace::TransactionSnapshot tableTx2;
  tableTx2.id = "tx-2";
  tableTx2.name = "Misc";
  tableTx2.bookingDate = "2026-02-01";
  tableTx2.amount = 50.0;
  tableTx2.contractId = "contract-2";
  tableTx2.allocatable = false;
  analysisTable.snapshotTransactions = {tableTx1, tableTx2};

  core::ports::workspace::AnalysisSnapshot analysisPlot;
  analysisPlot.id = "analysis-plot";
  analysisPlot.name = "Plot Export";
  analysisPlot.type = "plot";
  analysisPlot.exportFormat = "png";
  analysisPlot.config.type = "plot";
  analysisPlot.config.plotType = "pie";
  analysisPlot.config.plotMeasure = "totalAmount";
  core::ports::workspace::TransactionSnapshot plotTx;
  plotTx.id = "tx-3";
  plotTx.name = "PlotTx";
  plotTx.bookingDate = "2026-03-01";
  plotTx.amount = 42.0;
  plotTx.contractId = "contract-3";
  plotTx.contractType = "service";
  plotTx.propertyIds = {"property-2"};
  plotTx.propertyNames = {"Building B"};
  plotTx.allocatable = true;
  analysisPlot.snapshotTransactions = {plotTx};

  state.analyses = {analysisTable, analysisPlot};
  return state;
}

} // namespace

TEST(ExportServiceTest, ExportsCsvXlsxJpgAndPngFromAnalysisItems) {
  const auto state = buildState();
  const auto unique = std::to_string(
      std::filesystem::file_time_type::clock::now().time_since_epoch().count());
  const auto outputDir = std::filesystem::temp_directory_path() /
                         ("fossredder-export-formats-" + unique);
  std::filesystem::create_directories(outputDir);

  ExportRequest request;
  request.outputPath = outputDir.string();
  request.objectRequests = {{ExportObjectType::Analysis, "analysis-table", "",
                             AnalysisExportFormat::Csv, "table-csv"},
                            {ExportObjectType::Analysis, "analysis-table", "",
                             AnalysisExportFormat::Xlsx, "table-xlsx"},
                            {ExportObjectType::Analysis, "analysis-plot", "",
                             AnalysisExportFormat::Jpg, "plot-jpg"},
                            {ExportObjectType::Analysis, "analysis-plot", "",
                             AnalysisExportFormat::Png, "plot-png"}};

  ExportService service({}, std::make_shared<FakeXlsxWriter>(),
                        std::make_shared<FakeAnalysisImageRenderer>());

  const auto result = service.runExport(state, request);

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.status, ExportStatus::Ok);
  EXPECT_TRUE(std::filesystem::exists(outputDir / "table-csv.csv"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "table-xlsx.xlsx"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "plot-jpg.jpg"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "plot-png.png"));

  std::error_code ec;
  std::filesystem::remove_all(outputDir, ec);
}

TEST(ExportServiceTest, XlsxUsesSameTableHeaderShapeAsCsvPath) {
  const auto state = buildState();
  const auto unique = std::to_string(
      std::filesystem::file_time_type::clock::now().time_since_epoch().count());
  const auto outputDir = std::filesystem::temp_directory_path() /
                         ("fossredder-export-xlsx-header-" + unique);
  std::filesystem::create_directories(outputDir);

  auto writer = std::make_shared<FakeXlsxWriter>();
  ExportService service({}, writer,
                        std::make_shared<FakeAnalysisImageRenderer>());

  ExportRequest csvRequest;
  csvRequest.outputPath = outputDir.string();
  csvRequest.objectRequests = {{ExportObjectType::Analysis, "analysis-table",
                                "", AnalysisExportFormat::Csv, "table-csv"}};
  const auto csvResult = service.runExport(state, csvRequest);
  ASSERT_TRUE(csvResult.success);

  std::ifstream csv(outputDir / "table-csv.csv", std::ios::binary);
  ASSERT_TRUE(csv);
  std::string csvHeaderLine;
  ASSERT_TRUE(std::getline(csv, csvHeaderLine));
  const auto csvHeader = splitCsvHeader(csvHeaderLine);

  ExportRequest xlsxRequest;
  xlsxRequest.outputPath = outputDir.string();
  xlsxRequest.objectRequests = {{ExportObjectType::Analysis, "analysis-table",
                                 "", AnalysisExportFormat::Xlsx,
                                 "table-xlsx"}};
  const auto result = service.runExport(state, xlsxRequest);

  ASSERT_TRUE(result.success);
  ASSERT_FALSE(writer->lastRows.empty());
  const auto &header = writer->lastRows.front();
  EXPECT_EQ(header, csvHeader);

  std::error_code ec;
  std::filesystem::remove_all(outputDir, ec);
}

TEST(ExportServiceTest, PackagingNoneSkipsArchiveAndZipCallsArchive) {
  const auto state = buildState();
  const auto unique = std::to_string(
      std::filesystem::file_time_type::clock::now().time_since_epoch().count());
  const auto outputDir = std::filesystem::temp_directory_path() /
                         ("fossredder-export-packaging-" + unique);
  std::filesystem::create_directories(outputDir);

  auto archive = std::make_shared<FakeArchive>();
  ExportService service(archive, std::make_shared<FakeXlsxWriter>(),
                        std::make_shared<FakeAnalysisImageRenderer>());

  ExportRequest noneRequest;
  noneRequest.outputPath = (outputDir / "none").string();
  noneRequest.packageFormat = PackageFormat::None;
  noneRequest.objectRequests = {{ExportObjectType::Analysis, "analysis-table",
                                 "", AnalysisExportFormat::Csv, "table-csv"}};
  const auto noneResult = service.runExport(state, noneRequest);
  EXPECT_TRUE(noneResult.success);
  EXPECT_EQ(archive->calls, 0);

  ExportRequest zipRequest;
  zipRequest.outputPath = (outputDir / "zip").string();
  zipRequest.packageFormat = PackageFormat::Zip;
  zipRequest.objectRequests = {{ExportObjectType::Analysis, "analysis-table",
                                "", AnalysisExportFormat::Csv, "table-csv"}};
  const auto zipResult = service.runExport(state, zipRequest);
  EXPECT_TRUE(zipResult.success);
  EXPECT_EQ(archive->calls, 1);
  EXPECT_TRUE(zipResult.resolvedOutputPath.size() >= 4);
  EXPECT_EQ(zipResult.resolvedOutputPath.substr(
                zipResult.resolvedOutputPath.size() - 4),
            ".zip");

  std::error_code ec;
  std::filesystem::remove_all(outputDir, ec);
}

TEST(ExportServiceTest, ReportsProgressAcrossExportStages) {
  const auto state = buildState();
  const auto unique = std::to_string(
      std::filesystem::file_time_type::clock::now().time_since_epoch().count());
  const auto outputDir = std::filesystem::temp_directory_path() /
                         ("fossredder-export-progress-" + unique);
  std::filesystem::create_directories(outputDir);

  std::vector<double> progressValues;
  ExportRequest request;
  request.outputPath = outputDir.string();
  request.objectRequests = {{ExportObjectType::Analysis, "analysis-table", "",
                             AnalysisExportFormat::Csv, "table-csv"},
                            {ExportObjectType::Analysis, "analysis-plot", "",
                             AnalysisExportFormat::Png, "plot-png"}};
  request.progressCallback = [&progressValues](double progress,
                                               const std::string &) {
    progressValues.push_back(progress);
  };

  ExportService service({}, std::make_shared<FakeXlsxWriter>(),
                        std::make_shared<FakeAnalysisImageRenderer>());
  const auto result = service.runExport(state, request);

  ASSERT_TRUE(result.success);
  ASSERT_GE(progressValues.size(), 3u);
  EXPECT_GE(progressValues.front(), 0.0);
  EXPECT_LE(progressValues.back(), 1.0);
  for (std::size_t i = 1; i < progressValues.size(); ++i) {
    EXPECT_GE(progressValues[i], progressValues[i - 1]);
  }

  std::error_code ec;
  std::filesystem::remove_all(outputDir, ec);
}

TEST(ExportServiceTest, KeepsMultipleFormatsForSameAnalysisId) {
  const auto state = buildState();
  const auto unique = std::to_string(
      std::filesystem::file_time_type::clock::now().time_since_epoch().count());
  const auto outputDir = std::filesystem::temp_directory_path() /
                         ("fossredder-export-multi-format-guard-" + unique);
  std::filesystem::create_directories(outputDir);

  ExportRequest request;
  request.outputPath = outputDir.string();
  request.objectRequests = {{ExportObjectType::Analysis, "analysis-table", "",
                             AnalysisExportFormat::Csv, "table-dual"},
                            {ExportObjectType::Analysis, "analysis-table", "",
                             AnalysisExportFormat::Xlsx, "table-dual"},
                            {ExportObjectType::Analysis, "analysis-plot", "",
                             AnalysisExportFormat::Jpg, "plot-dual"},
                            {ExportObjectType::Analysis, "analysis-plot", "",
                             AnalysisExportFormat::Png, "plot-dual"}};

  ExportService service({}, std::make_shared<FakeXlsxWriter>(),
                        std::make_shared<FakeAnalysisImageRenderer>());
  const auto result = service.runExport(state, request);

  ASSERT_TRUE(result.success);
  EXPECT_TRUE(std::filesystem::exists(outputDir / "table-dual.csv"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "table-dual.xlsx"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "plot-dual.jpg"));
  EXPECT_TRUE(std::filesystem::exists(outputDir / "plot-dual.png"));

  std::error_code ec;
  std::filesystem::remove_all(outputDir, ec);
}

} // namespace core::application::exporting
