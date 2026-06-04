/**
 * @file ui/src/adapters/AnalysisAdapter.cpp
 * @brief Implements the analysis adapter used by UI workflows.
 */

#include "ui/adapters/AnalysisAdapter.h"

#include "ui/presentation/PayloadKeys.h"
#include "ui/observability/Trace.h"
#include "ui/i18n/Text.h"

#include <utility>

namespace ui::adapters {
namespace {

QVariantMap toMetricsMap(const core::ports::analysis::AnalysisResult &result) {
  QVariantMap metrics;
  for (const auto &[key, value] : result.metrics) {
    metrics.insert(QString::fromStdString(key), value);
  }
  return metrics;
}

QVariantList toTableList(const core::ports::analysis::AnalysisResult &result) {
  QVariantList table;
  for (const auto &row : result.table) {
    QVariantList columns;
    for (const auto &column : row) {
      columns.push_back(QString::fromStdString(column));
    }
    table.push_back(columns);
  }
  return table;
}

QVariantList
toArtifactList(const core::ports::analysis::AnalysisResult &result) {
  QVariantList artifacts;
  for (const auto &artifact : result.artifacts) {
    artifacts.push_back(QString::fromStdString(artifact));
  }
  return artifacts;
}

QVariantList
toTransactionRows(const core::ports::analysis::AnalysisResult &result) {
  QVariantList transactions;
  for (const auto &transaction : result.transactions) {
    QVariantMap item;
    item[ui::payload::keys::common::kId] =
        QString::fromStdString(transaction.id);
    item[ui::payload::keys::common::kName] =
        QString::fromStdString(transaction.name);
    item[ui::payload::keys::transaction::kDate] =
        QString::fromStdString(transaction.bookingDate);
    item[ui::payload::keys::common::kAmount] = transaction.amount;
    item[ui::payload::keys::transaction::kContractId] =
        QString::fromStdString(transaction.contractId);
    item[ui::payload::keys::transaction::kContractType] =
        transaction.contractType.empty()
            ? ui::text::analysis::unassignedContractType()
            : QString::fromStdString(transaction.contractType);

    QStringList propertyIds;
    propertyIds.reserve(static_cast<int>(transaction.propertyIds.size()));
    for (const auto &propertyId : transaction.propertyIds) {
      if (!propertyId.empty()) {
        propertyIds.push_back(QString::fromStdString(propertyId));
      }
    }
    item[ui::payload::keys::transaction::kPropertyIds] = propertyIds;

    QStringList propertyNames;
    propertyNames.reserve(static_cast<int>(transaction.propertyNames.size()));
    for (const auto &propertyName : transaction.propertyNames) {
      if (!propertyName.empty()) {
        propertyNames.push_back(QString::fromStdString(propertyName));
      }
    }
    item[QStringLiteral("propertyNames")] = propertyNames;

    transactions.push_back(item);
  }
  return transactions;
}

std::vector<std::string> toStdStringList(const QStringList &values) {
  std::vector<std::string> out;
  out.reserve(values.size());
  for (const auto &value : values) {
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty()) {
      out.push_back(trimmed.toStdString());
    }
  }
  return out;
}

std::vector<core::ports::analysis::AnalysisAdjustmentTransactionInput>
toAdjustmentTransactions(const QVariantList &transactions) {
  std::vector<core::ports::analysis::AnalysisAdjustmentTransactionInput> out;
  out.reserve(static_cast<std::size_t>(transactions.size()));
  for (const auto &value : transactions) {
    const QVariantMap row = value.toMap();
    core::ports::analysis::AnalysisAdjustmentTransactionInput item;
    item.id = row.value(QStringLiteral("id")).toString().trimmed().toStdString();
    item.amount = row.value(QStringLiteral("amount")).toDouble();
    if (!item.id.empty()) {
      out.push_back(std::move(item));
    }
  }
  return out;
}

} // namespace

AnalysisAdapter::AnalysisAdapter(
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::analysis::AnalysisResult AnalysisAdapter::runAnalysis(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const core::ports::analysis::AnalysisRequest &request) const {
  observability::traceAdapter(
      "AnalysisAdapter::runAnalysis", "Analysis runner invoked",
      {{"analysisId", request.analysisId}});
  return runner_ ? runner_->runAnalysis(workspace, request)
                 : core::ports::analysis::AnalysisResult{};
}

core::ports::analysis::AnalysisPreviewResult
AnalysisAdapter::previewTransactions(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::string &filterSpec) const {
  return runner_ ? runner_->previewTransactions(workspace, filterSpec)
                 : core::ports::analysis::AnalysisPreviewResult{};
}

core::ports::analysis::AnalysisFilterSelection
AnalysisAdapter::filterSelectionFromFields(
    const std::string &dateField, const std::string &dateMode,
    const std::string &year, const std::string &dateFrom,
    const std::string &dateTo, const std::vector<std::string> &propertyIds,
    const std::vector<std::string> &contractTypes,
    const std::string &allocatableMode) const {
  return runner_ ? runner_->filterSelectionFromFields(
                       dateField, dateMode, year, dateFrom, dateTo,
                       propertyIds, contractTypes, allocatableMode)
                 : core::ports::analysis::AnalysisFilterSelection{};
}

std::string AnalysisAdapter::buildAnalysisConfigJson(
    const core::ports::analysis::AnalysisConfigInput &input) const {
  return runner_ ? runner_->buildAnalysisConfigJson(input) : std::string{};
}

QString AnalysisAdapter::buildFilterSpec(
    const QString &dateField, const QString &dateMode, const QString &year,
    const QString &dateFrom, const QString &dateTo,
    const QStringList &propertyIds, const QStringList &contractTypes,
    const QString &allocatableMode) const {
  if (!runner_) {
    return {};
  }
  const auto selection = filterSelectionFromFields(
      dateField.trimmed().toLower().toStdString(),
      dateMode.trimmed().toLower().toStdString(), year.trimmed().toStdString(),
      dateFrom.trimmed().toStdString(), dateTo.trimmed().toStdString(),
      toStdStringList(propertyIds), toStdStringList(contractTypes),
      allocatableMode.trimmed().toLower().toStdString());
  return QString::fromStdString(
      core::ports::analysis::buildAnalysisFilterSpec(selection));
}

QString AnalysisAdapter::buildAnalysisConfigJson(
    const QString &type, const QString &plotType, const QString &plotMeasure,
    const QStringList &propertyIds, const QStringList &contractTypes,
    double taxPercent) const {
  core::ports::analysis::AnalysisConfigInput input;
  input.type = type.trimmed().toLower().toStdString();
  input.plotType = plotType.trimmed().toStdString();
  input.plotMeasure = plotMeasure.trimmed().toStdString();
  input.propertyIds = toStdStringList(propertyIds);
  input.contractTypes = toStdStringList(contractTypes);
  input.taxPercent = taxPercent;
  return QString::fromStdString(buildAnalysisConfigJson(input));
}

std::string AnalysisAdapter::buildAnalysisAdjustmentsJson(
    const std::vector<core::ports::analysis::AnalysisAdjustmentTransactionInput>
        &transactions,
    const std::vector<std::string> &selectedTransactionIds,
    double taxPercent) const {
  return runner_ ? runner_->buildAnalysisAdjustmentsJson(
                       transactions, selectedTransactionIds, taxPercent)
                 : std::string("{}");
}

std::string AnalysisAdapter::buildAnalysisAdjustmentsJson(
    const QVariantList &transactions, const QStringList &selectedTransactionIds,
    double taxPercent) const {
  return buildAnalysisAdjustmentsJson(
      toAdjustmentTransactions(transactions),
      toStdStringList(selectedTransactionIds), taxPercent);
}

void AnalysisAdapter::applyAnalysisPreviewOverrides(
    core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::string &analysisId, bool includeCalculationAdjustments,
    const std::string &adjustmentsJson) const {
  if (runner_) {
    runner_->applyAnalysisPreviewOverrides(workspace, analysisId,
                                           includeCalculationAdjustments,
                                           adjustmentsJson);
  }
}

core::ports::analysis::AnalysisTableState AnalysisAdapter::projectTableState(
    const core::ports::analysis::AnalysisResult &result,
    const std::string &adjustmentsJson, bool includeCalculationAdjustments,
    const std::string &unassignedLabel) const {
  return runner_ ? runner_->projectTableState(
                       result, adjustmentsJson, includeCalculationAdjustments,
                       unassignedLabel)
                 : core::ports::analysis::AnalysisTableState{};
}

std::vector<std::string> AnalysisAdapter::contractTypes(
    const core::ports::workspace::WorkspaceSnapshot &workspace) const {
  return runner_ ? runner_->contractTypes(workspace)
                 : std::vector<std::string>{};
}

QVariantMap AnalysisAdapter::mapAnalysisResult(
    const core::ports::analysis::AnalysisResult &result) const {
  QVariantMap payload;
  payload[ui::payload::keys::analysis::kMetrics] = toMetricsMap(result);
  payload[ui::payload::keys::analysis::kTable] = toTableList(result);
  payload[ui::payload::keys::common::kType] =
      QString::fromStdString(result.type);
  payload[ui::payload::keys::analysis::kConfig] =
      QString::fromStdString(result.configJson);
  payload[ui::payload::keys::analysis::kTransactions] =
      toTransactionRows(result);
  payload[ui::payload::keys::analysis::kArtifacts] = toArtifactList(result);
  payload[ui::payload::keys::analysis::kGeneratedAt] =
      QString::fromStdString(result.generatedAt);
  return payload;
}

QVariantMap AnalysisAdapter::mapAnalysisTableState(
    const core::ports::analysis::AnalysisTableState &state) const {
  QVariantList contractTypes;
  contractTypes.reserve(static_cast<int>(state.contractTypes.size()));
  for (const auto &contractType : state.contractTypes) {
    contractTypes.push_back(QString::fromStdString(contractType));
  }

  QVariantList propertyRows;
  propertyRows.reserve(static_cast<int>(state.propertyRows.size()));
  for (const auto &row : state.propertyRows) {
    QVariantList amounts;
    amounts.reserve(static_cast<int>(row.amounts.size()));
    for (double amount : row.amounts) {
      amounts.push_back(amount);
    }
    propertyRows.push_back(QVariantMap{
        {QStringLiteral("propertyName"), QString::fromStdString(row.propertyName)},
        {QStringLiteral("amounts"), amounts},
        {QStringLiteral("total"), row.total}});
  }

  return QVariantMap{{QStringLiteral("contractTypes"), contractTypes},
                     {QStringLiteral("propertyRows"), propertyRows},
                     {QStringLiteral("grandTotal"), state.grandTotal}};
}

QVariantMap AnalysisAdapter::mapFilterSelection(
    const core::ports::analysis::AnalysisFilterSelection &selection) const {
  QVariantMap out;
  out.insert(QStringLiteral("dateField"),
             QString::fromStdString(selection.dateField));
  out.insert(QStringLiteral("dateMode"),
             QString::fromStdString(selection.dateMode));
  out.insert(QStringLiteral("year"), QString::fromStdString(selection.year));
  out.insert(QStringLiteral("dateFrom"),
             QString::fromStdString(selection.dateFrom));
  out.insert(QStringLiteral("dateTo"),
             QString::fromStdString(selection.dateTo));

  QVariantList propertyIds;
  propertyIds.reserve(static_cast<int>(selection.propertyIds.size()));
  for (const auto &id : selection.propertyIds) {
    propertyIds.push_back(QString::fromStdString(id));
  }
  out.insert(QStringLiteral("propertyIds"), propertyIds);
  out.insert(QStringLiteral("propertyIdsNone"),
             selection.propertyIdsUnassigned);

  QVariantList contractTypes;
  contractTypes.reserve(static_cast<int>(selection.contractTypes.size()));
  for (const auto &type : selection.contractTypes) {
    contractTypes.push_back(QString::fromStdString(type));
  }
  out.insert(QStringLiteral("contractTypes"), contractTypes);
  out.insert(QStringLiteral("contractTypesNone"),
             selection.contractTypesUnassigned);
  out.insert(QStringLiteral("allocatableMode"),
             QString::fromStdString(selection.allocatableMode));
  return out;
}

QVariantMap AnalysisAdapter::mapPreviewResult(
    const core::ports::analysis::AnalysisPreviewResult &preview) const {
  QVariantMap out;
  QVariantList transactions;
  transactions.reserve(
      static_cast<int>(preview.transactions.size()));

  for (const auto &transaction : preview.transactions) {
    QVariantMap row;
    const QString txId = QString::fromStdString(transaction.id);
    const QString statementId = QString::fromStdString(transaction.statementId);
    const QString contractId = QString::fromStdString(transaction.contractId);

    QStringList propertyIds;
    propertyIds.reserve(static_cast<int>(transaction.propertyIds.size()));
    for (const auto &propertyId : transaction.propertyIds) {
      const QString id = QString::fromStdString(propertyId);
      if (!id.isEmpty() && !propertyIds.contains(id)) {
        propertyIds.push_back(id);
      }
    }

    QStringList propertyNames;
    propertyNames.reserve(static_cast<int>(transaction.propertyNames.size()));
    for (const auto &name : transaction.propertyNames) {
      if (!name.empty()) {
        propertyNames.push_back(QString::fromStdString(name));
      }
    }

    row[QStringLiteral("id")] = txId;
    row[QStringLiteral("name")] = QString::fromStdString(transaction.name);
    row[QStringLiteral("transactionName")] =
        QString::fromStdString(transaction.name);
    row[QStringLiteral("date")] =
        QString::fromStdString(transaction.bookingDate);
    row[QStringLiteral("valuta")] = QString::fromStdString(transaction.valuta);
    row[QStringLiteral("amount")] = transaction.amount;
    row[QStringLiteral("statementId")] = statementId;
    row[QStringLiteral("statementName")] =
        QString::fromStdString(transaction.statementName);
    row[QStringLiteral("actorName")] =
        QString::fromStdString(transaction.actorName);
    row[QStringLiteral("contractId")] = contractId;
    row[QStringLiteral("contractName")] =
        QString::fromStdString(transaction.contractName);
    row[QStringLiteral("contractType")] =
        QString::fromStdString(transaction.contractType);
    row[QStringLiteral("propertyIds")] = propertyIds;
    row[QStringLiteral("propertyNames")] = propertyNames;
    row[QStringLiteral("propertiesLabel")] =
        propertyNames.join(QStringLiteral(", "));
    row[QStringLiteral("allocatable")] = transaction.allocatable;

    transactions.push_back(row);
  }

  QVariantMap metrics;
  metrics[QStringLiteral("statementCount")] = preview.metrics.statementCount;
  metrics[QStringLiteral("transactionCount")] = preview.metrics.transactionCount;
  metrics[QStringLiteral("amountSum")] = preview.metrics.amountSum;

  out[QStringLiteral("transactions")] = transactions;
  out[QStringLiteral("metrics")] = metrics;
  return out;
}

QStringList AnalysisAdapter::mapContractTypes(
    const std::vector<std::string> &contractTypes) const {
  QStringList values;
  values.reserve(static_cast<int>(contractTypes.size()));
  for (const auto &type : contractTypes) {
    values.push_back(QString::fromStdString(type));
  }
  return values;
}

} // namespace ui::adapters
