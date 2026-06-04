/**
 * @file ui/tests/unit/adapters/TestExportAdapter.cpp
 * @brief Tests for the UI ExportAdapter boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QVariantList>
#include <QVariantMap>

#include "support/WorkspacePortFakes.h"
#include "ui/adapters/ExportAdapter.h"

namespace ui {

TEST(ExportAdapterTest,
     ADP_EXPORT_001_BuildExportRequestMapsPackageAndObjectSelection) {
  adapters::ExportAdapter adapter(
      std::make_shared<tests::support::ExportRunnerStub>());
  const QVariantMap payload{
      {QStringLiteral("packageFormatIndex"), 1},
      {QStringLiteral("items"),
       QVariantList{QVariantMap{{QStringLiteral("objectId"),
                                 QStringLiteral("analysis-1")},
                                {QStringLiteral("objectName"),
                                 QStringLiteral("Analysis")},
                                {QStringLiteral("objectType"),
                                 QStringLiteral("analysis")},
                                {QStringLiteral("exportType"),
                                 QStringLiteral("png")}}}}};

  const auto request = adapter.buildExportRequest(
      1, QStringLiteral("C:/tmp/out.xlsx"), false, QStringLiteral("de_DE"),
      payload);

  EXPECT_EQ(request.format, core::ports::exporting::ExportFormat::Xlsx);
  EXPECT_EQ(request.packageFormat, core::ports::exporting::PackageFormat::Zip);
  ASSERT_EQ(request.objectRequests.size(), 1);
  EXPECT_EQ(request.objectRequests.front().objectId, "analysis-1");
  EXPECT_EQ(request.objectRequests.front().format,
            core::ports::exporting::AnalysisExportFormat::Png);
}

TEST(ExportAdapterTest, ADP_EXPORT_002_RunExportDelegatesToRunner) {
  adapters::ExportAdapter adapter(
      std::make_shared<tests::support::ExportRunnerStub>());
  const auto result =
      adapter.runExport({}, adapter.buildExportRequest(0, QStringLiteral("out"),
                                                       true, {}, {}));

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.resolvedOutputPath, "out");
}

} // namespace ui
