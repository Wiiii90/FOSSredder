/**
 * @file ui/src/workspace/WorkspaceRowProjector.cpp
 * @brief Implements helpers that project workspace data into QML-friendly rows.
 */

#include "ui/workspace/WorkspaceRowProjector.h"

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/workspace/WorkspaceCache.h"

#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>

namespace ui {

namespace {

QString serializeAdjustmentsJson(
    const core::ports::workspace::AnalysisSnapshot &analysis) {
  QJsonObject object;
  for (const auto &[id, amount] : analysis.adjustments) {
    object.insert(QString::fromStdString(id), amount);
  }
  return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

template <typename Aliases>
std::vector<std::string> aliasValues(const Aliases &aliases) {
  std::vector<std::string> out;
  out.reserve(aliases.size());
  for (const auto &alias : aliases) {
    out.push_back(alias.value);
  }
  return out;
}

template <typename Item> void insertIdName(QVariantMap &row, const Item &item) {
  row.insert(ui::payload::keys::common::kId, QString::fromStdString(item.id));
  row.insert(ui::payload::keys::common::kName,
             QString::fromStdString(item.name));
}

template <typename Item>
void insertIdNameDisplay(QVariantMap &row, const Item &item) {
  insertIdName(row, item);
  row.insert(ui::payload::keys::common::kDisplay,
             QString::fromStdString(item.name));
}

template <typename Item>
QVariantMap linkedCatalogRow(const Item &item, const QString &aliasesKey) {
  QVariantMap row;
  insertIdNameDisplay(row, item);
  row.insert(ui::payload::keys::state::kSelectedIds,
             payload::mapper::toQStringList(item.contractIds));
  row.insert(aliasesKey,
             payload::mapper::toVariantStringList(aliasValues(item.aliases)));
  return row;
}

} // namespace

QVariantList buildActorRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &actor : cache.models().actorSnapshots()) {
    if (!actor)
      continue;

    out.push_back(linkedCatalogRow(*actor, ui::payload::keys::actor::kAliases));
  }
  return out;
}

QVariantList buildPropertyRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &property : cache.models().propertySnapshots()) {
    if (!property)
      continue;

    out.push_back(
        linkedCatalogRow(*property, ui::payload::keys::property::kAliases));
  }
  return out;
}

QVariantList buildContractRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &contract : cache.models().contractSnapshots()) {
    if (!contract)
      continue;

    QVariantMap row;
    insertIdNameDisplay(row, *contract);
    row.insert(ui::payload::keys::common::kType,
               QString::fromStdString(contract->type));
    row.insert(QStringLiteral("allocatableMode"),
               QString::fromStdString(contract->allocatableMode));
    row.insert(ui::payload::keys::contract::kAliases,
               payload::mapper::toVariantStringList(
                   aliasValues(contract->aliases)));
    row.insert(ui::payload::keys::contract::kActorIds,
               payload::mapper::toQStringList(contract->actorIds));
    row.insert(ui::payload::keys::contract::kPropertyIds,
               payload::mapper::toQStringList(contract->propertyIds));
    out.push_back(row);
  }
  return out;
}

QVariantList buildAnalysisRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &analysis : cache.models().analyses().analyses()) {
    if (!analysis)
      continue;

    QVariantMap row;
    insertIdName(row, *analysis);
    row.insert(ui::payload::keys::common::kType,
               QString::fromStdString(analysis->type));
    row.insert(ui::payload::keys::analysis::kConfig,
               QString::fromStdString(analysis->configJson));
    row.insert(ui::payload::keys::analysis::kFilter,
               QString::fromStdString(analysis->filterSpec));
    row.insert(ui::payload::keys::analysis::kAdjustments,
               serializeAdjustmentsJson(*analysis));
    row.insert(ui::payload::keys::analysis::kExportFormat,
               QString::fromStdString(analysis->exportFormat));
    row.insert(ui::payload::keys::analysis::kIncludeCalcAdjustments,
               analysis->includeCalculationAdjustments);
    row.insert(ui::payload::keys::analysis::kExportState,
               QString::fromStdString(analysis->exportStateJson));
    row.insert(ui::payload::keys::analysis::kSnapshotTransactions,
               QString::fromStdString(analysis->snapshotTransactionsJson));
    row.insert(ui::payload::keys::analysis::kCreatedAt,
               QString::fromStdString(analysis->createdAt));
    row.insert(ui::payload::keys::analysis::kUpdatedAt,
               QString::fromStdString(analysis->updatedAt));
    out.push_back(row);
  }
  return out;
}

QVariantList buildAnnualRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &annual : cache.models().annuals().annuals()) {
    if (!annual)
      continue;

    QVariantMap row;
    row.insert(ui::payload::keys::common::kId,
               QString::fromStdString(annual->id));
    row.insert(ui::payload::keys::annual::kName,
               QString::fromStdString(annual->name));
    row.insert(ui::payload::keys::annual::kYear, annual->year);
    row.insert(ui::payload::keys::annual::kAnalysisIds,
               payload::mapper::toVariantStringList(annual->analysisIds));
    row.insert(ui::payload::keys::common::kDisplay,
               annual->name.empty() ? QString::number(annual->year)
                                    : QString::fromStdString(annual->name));
    out.push_back(row);
  }
  return out;
}

QVariantList buildStatementRows(const WorkspaceCache &cache) {
  QVariantList out;
  for (const auto &statement : cache.models().statements().statements()) {
    if (!statement)
      continue;

    QVariantMap row;
    insertIdName(row, *statement);
    out.push_back(row);
  }
  return out;
}

QVariantList buildStatementTransactionRows(const WorkspaceCache &cache,
                                           const QString &statementId) {
  QVariantList out;
  if (statementId.isEmpty())
    return out;

  for (const auto &transaction :
       cache.models().transactions().transactions()) {
    if (QString::fromStdString(transaction.statementId) != statementId)
      continue;

    QVariantMap row;
    insertIdName(row, transaction);
    row.insert(ui::payload::keys::transaction::kBookingDate,
               QString::fromStdString(transaction.bookingDate));
    out.push_back(row);
  }
  return out;
}

} // namespace ui
