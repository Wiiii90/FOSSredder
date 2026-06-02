/**
 * @file ui/src/adapters/AnalysisAdapter.cpp
 * @brief Implements the analysis adapter used by UI workflows.
 */

#include "ui/adapters/AnalysisAdapter.h"

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/text/Text.h"

#include <QHash>
#include <QSet>

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
toTransactionViewModel(const core::ports::analysis::AnalysisResult &result) {
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

} // namespace

AnalysisAdapter::AnalysisAdapter(
    std::shared_ptr<core::ports::analysis::IAnalysisRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::analysis::AnalysisResult AnalysisAdapter::runAnalysis(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const core::ports::analysis::AnalysisRequest &request) const {
  return runner_ ? runner_->runAnalysis(workspace, request)
                 : core::ports::analysis::AnalysisResult{};
}

std::vector<core::ports::analysis::AnalysisPreviewTransaction>
AnalysisAdapter::previewTransactions(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::string &filterSpec) const {
  return runner_
             ? runner_->previewTransactions(workspace, filterSpec)
             : std::vector<core::ports::analysis::AnalysisPreviewTransaction>{};
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
      toTransactionViewModel(result);
  payload[ui::payload::keys::analysis::kArtifacts] = toArtifactList(result);
  payload[ui::payload::keys::analysis::kGeneratedAt] =
      QString::fromStdString(result.generatedAt);
  return payload;
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

QVariantMap AnalysisAdapter::mapPreviewTransactions(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::vector<core::ports::analysis::AnalysisPreviewTransaction>
        &filtered) const {
  QVariantMap out;
  QHash<QString, QString> actorNameById;
  actorNameById.reserve(static_cast<int>(workspace.actors.size()));
  for (const auto &actor : workspace.actors) {
    actorNameById.insert(QString::fromStdString(actor.id),
                         QString::fromStdString(actor.name));
  }

  QHash<QString, QString> statementNameById;
  statementNameById.reserve(static_cast<int>(workspace.statements.size()));
  for (const auto &statement : workspace.statements) {
    statementNameById.insert(QString::fromStdString(statement.id),
                             QString::fromStdString(statement.name));
  }

  QVariantList transactions;
  QVariantMap metrics;
  QSet<QString> statementIds;
  double amountSum = 0.0;
  transactions.reserve(static_cast<int>(filtered.size()));

  for (const auto &transaction : filtered) {
    QVariantMap row;
    const QString txId = QString::fromStdString(transaction.id);
    const QString statementId = QString::fromStdString(transaction.statementId);
    const QString contractId = QString::fromStdString(transaction.contractId);
    const QString actorId = QString::fromStdString(transaction.actorId);

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
        statementNameById.value(statementId, QString());
    row[QStringLiteral("actorName")] = actorNameById.value(actorId, QString());
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
    statementIds.insert(statementId);
    amountSum += transaction.amount;
  }

  metrics[QStringLiteral("statementCount")] = statementIds.size();
  metrics[QStringLiteral("transactionCount")] = transactions.size();
  metrics[QStringLiteral("amountSum")] = amountSum;

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
