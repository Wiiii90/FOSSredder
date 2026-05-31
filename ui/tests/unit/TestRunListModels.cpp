/**
 * @file ui/tests/unit/TestRunListModels.cpp
 * @brief Tests for import/export run-list display roles.
 */

#include <gtest/gtest.h>

#include <QAbstractItemModel>

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/viewmodels/export/ExportRunListModel.h"
#include "ui/viewmodels/import/ImportRunListModel.h"

namespace ui {

namespace {

QVariant roleData(const QAbstractItemModel& model, int row, const QString& role)
{
  const auto roles = model.roleNames();
  const int roleId = roles.key(role.toUtf8(), -1);
  if (roleId < 0) return {};
  return model.data(model.index(row, 0), roleId);
}

} // namespace

TEST(RunListModelsTest, RUN_LIST_001_ImportRunDisplayRolesStayPresentationReady)
{
  ImportRunList model;
  model.addRun(QStringLiteral("2026-01-01T10:00:00Z"), QStringLiteral("import"),
               QStringLiteral("test:///imports/statement.pdf"),
               QStringLiteral("Success"), QStringLiteral("Imported."));

  EXPECT_EQ(roleData(model, 0, payload::keys::common::kDisplayTime).toString(),
            QStringLiteral("2026-01-01T10:00:00Z"));
  EXPECT_EQ(roleData(model, 0, payload::keys::common::kDisplayTitle).toString(),
            QStringLiteral("statement.pdf"));
  EXPECT_EQ(roleData(model, 0, payload::keys::common::kDisplayStatusDetail).toString(),
            QStringLiteral("Imported."));
}

TEST(RunListModelsTest, RUN_LIST_002_ExportRunDisplayRolesProjectPayloadSummary)
{
  ExportRunList model;
  model.upsertRun({QStringLiteral("export-1"),
                   QStringLiteral("2026-01-01T10:00:00Z"),
                   QStringLiteral("test:///exports/export.xlsx"),
                   QStringLiteral("Success"),
                   {},
                   QStringLiteral(R"({"items":[{"objectType":"Annual","objectName":"2025"},{"objectType":"Analysis","objectName":"Rent Overview"}]})")});

  EXPECT_EQ(roleData(model, 0, payload::keys::common::kDisplayTitle).toString(),
            QStringLiteral("Export 'Rent Overview'"));
  EXPECT_EQ(roleData(model, 0, payload::keys::common::kDisplayStatusDetail).toString(),
            QStringLiteral("Export completed successfully."));
}

} // namespace ui
