/**
 * @file ui/src/workspace/WorkspacePayloads.cpp
 * @brief Implements workspace-specific QML payload builders.
 */

#include "ui/workspace/WorkspacePayloads.h"

#include <QFileInfo>

#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"

namespace ui::workspace_payloads {

namespace {

QString
validationSeverityName(core::ports::workspace::ValidationSeverity severity) {
  switch (severity) {
    case core::ports::workspace::ValidationSeverity::Info:
      return QStringLiteral("info");
    case core::ports::workspace::ValidationSeverity::Warning:
      return QStringLiteral("warning");
    case core::ports::workspace::ValidationSeverity::Error:
      return QStringLiteral("error");
  }
  return QStringLiteral("error");
}

QString displayFileName(const QString& path) {
  const QFileInfo info(path);
  const QString name = info.fileName();
  return name.isEmpty() ? path : name;
}

QStringList sortedStdStrings(const std::vector<std::string>& values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) {
    const QString trimmed = QString::fromStdString(value).trimmed();
    if (!trimmed.isEmpty()) {
      out.push_back(trimmed);
    }
  }
  out.sort();
  return out;
}

std::vector<std::string>
aliasValues(const std::vector<core::ports::workspace::AliasSnapshot>& aliases) {
  std::vector<std::string> out;
  out.reserve(aliases.size());
  for (const auto& alias : aliases) {
    out.push_back(alias.value);
  }
  return out;
}

template<typename Item>
void insertIdName(QVariantMap& row, const Item& item) {
  row.insert(payload::keys::common::kId, QString::fromStdString(item.id));
  row.insert(payload::keys::common::kName, QString::fromStdString(item.name));
}

template<typename Item>
void insertIdNameDisplay(QVariantMap& row, const Item& item) {
  insertIdName(row, item);
  row.insert(payload::keys::common::kDisplay,
             QString::fromStdString(item.name));
}

QVariantMap
adjustmentMap(const core::ports::workspace::AnalysisSnapshot& analysis) {
  QVariantMap values;
  for (const auto& [id, amount] : analysis.adjustments) {
    values.insert(QString::fromStdString(id), amount);
  }
  return values;
}

} // namespace

QVariantMap
analysisConfigRow(const core::ports::analysis::AnalysisConfigInput& config) {
  QVariantMap row;
  row.insert(QStringLiteral("type"), QString::fromStdString(config.type));
  row.insert(QStringLiteral("plotType"),
             QString::fromStdString(config.plotType));
  row.insert(QStringLiteral("plotMeasure"),
             QString::fromStdString(config.plotMeasure));
  row.insert(QStringLiteral("propertyIds"),
             payload::mapper::toQStringList(config.propertyIds));
  row.insert(QStringLiteral("contractTypes"),
             payload::mapper::toQStringList(config.contractTypes));
  row.insert(QStringLiteral("taxPercent"), config.taxPercent);
  return row;
}

QVariantMap analysisFilterRow(
    const core::ports::analysis::AnalysisFilterSelection& filter) {
  QVariantMap row;
  row.insert(QStringLiteral("dateField"),
             QString::fromStdString(filter.dateField));
  row.insert(QStringLiteral("dateMode"),
             QString::fromStdString(filter.dateMode));
  row.insert(QStringLiteral("year"), QString::fromStdString(filter.year));
  row.insert(QStringLiteral("dateFrom"),
             QString::fromStdString(filter.dateFrom));
  row.insert(QStringLiteral("dateTo"), QString::fromStdString(filter.dateTo));
  row.insert(QStringLiteral("propertyIds"),
             payload::mapper::toQStringList(filter.propertyIds));
  row.insert(QStringLiteral("propertyIdsNone"), filter.propertyIdsUnassigned);
  row.insert(QStringLiteral("contractTypes"),
             payload::mapper::toQStringList(filter.contractTypes));
  row.insert(QStringLiteral("contractTypesNone"),
             filter.contractTypesUnassigned);
  row.insert(QStringLiteral("allocatableMode"),
             QString::fromStdString(filter.allocatableMode));
  return row;
}

QVariantMap
validationResult(const core::ports::workspace::ValidationResult& result) {
  QVariantList issues;
  issues.reserve(static_cast<int>(result.issues.size()));
  for (const auto& issue : result.issues) {
    issues.push_back(QVariantMap{
        {QStringLiteral("field"), QString::fromStdString(issue.field)},
        {QStringLiteral("code"), QString::fromStdString(issue.code)},
        {QStringLiteral("message"), QString::fromStdString(issue.message)},
        {QStringLiteral("severity"), validationSeverityName(issue.severity)}});
  }
  return {{QStringLiteral("valid"), result.valid()},
          {QStringLiteral("issues"), issues}};
}

QVariantMap actorRow(const core::ports::workspace::ActorSnapshot& actor) {
  QVariantMap row;
  insertIdNameDisplay(row, actor);
  row.insert(payload::keys::state::kSelectedIds,
             payload::mapper::toQStringList(actor.contractIds));
  row.insert(payload::keys::actor::kAliases,
             payload::mapper::toVariantStringList(aliasValues(actor.aliases)));
  return row;
}

QVariantMap
propertyRow(const core::ports::workspace::PropertySnapshot& property) {
  QVariantMap row;
  insertIdNameDisplay(row, property);
  row.insert(payload::keys::state::kSelectedIds,
             payload::mapper::toQStringList(property.contractIds));
  row.insert(
      payload::keys::property::kAliases,
      payload::mapper::toVariantStringList(aliasValues(property.aliases)));
  return row;
}

QVariantMap
contractRow(const core::ports::workspace::ContractSnapshot& contract) {
  QVariantMap row;
  insertIdNameDisplay(row, contract);
  row.insert(payload::keys::common::kType,
             QString::fromStdString(contract.type));
  row.insert(payload::keys::contract::kAllocatableMode,
             QString::fromStdString(contract.allocatableMode));
  row.insert(
      payload::keys::contract::kAliases,
      payload::mapper::toVariantStringList(aliasValues(contract.aliases)));
  row.insert(payload::keys::contract::kActorIds,
             payload::mapper::toQStringList(contract.actorIds));
  row.insert(payload::keys::contract::kPropertyIds,
             payload::mapper::toQStringList(contract.propertyIds));
  return row;
}

QVariantMap
analysisRow(const core::ports::workspace::AnalysisSnapshot& analysis) {
  QVariantMap row;
  insertIdName(row, analysis);
  row.insert(payload::keys::common::kType,
             QString::fromStdString(analysis.type));
  row.insert(payload::keys::analysis::kConfig,
             analysisConfigRow(analysis.config));
  row.insert(payload::keys::analysis::kFilter,
             analysisFilterRow(analysis.filter));
  row.insert(payload::keys::analysis::kAdjustments, adjustmentMap(analysis));
  row.insert(payload::keys::analysis::kExportFormat,
             QString::fromStdString(analysis.exportFormat));
  row.insert(payload::keys::analysis::kIncludeCalcAdjustments,
             analysis.includeCalculationAdjustments);
  row.insert(payload::keys::analysis::kSnapshotTransactions,
             transactionRows(analysis.snapshotTransactions));
  row.insert(payload::keys::analysis::kCreatedAt,
             QString::fromStdString(analysis.createdAt));
  row.insert(payload::keys::analysis::kUpdatedAt,
             QString::fromStdString(analysis.updatedAt));
  return row;
}

QVariantMap annualRow(const core::ports::workspace::AnnualSnapshot& annual) {
  QVariantMap row;
  row.insert(payload::keys::common::kId, QString::fromStdString(annual.id));
  row.insert(payload::keys::annual::kName, QString::fromStdString(annual.name));
  row.insert(payload::keys::annual::kYear, annual.year);
  row.insert(payload::keys::annual::kAnalysisIds,
             payload::mapper::toVariantStringList(annual.analysisIds));
  row.insert(payload::keys::common::kDisplay,
             annual.name.empty() ? QString::number(annual.year)
                                 : QString::fromStdString(annual.name));
  return row;
}

QVariantMap
statementRow(const core::ports::workspace::StatementSnapshot& statement) {
  QVariantMap row;
  insertIdName(row, statement);
  return row;
}

QVariantMap
transactionRow(const core::ports::workspace::TransactionSnapshot& transaction,
               const QString& contractType) {
  QVariantMap row;
  insertIdName(row, transaction);
  row[payload::keys::statement::kStatementId] =
      QString::fromStdString(transaction.statementId);
  row[payload::keys::transaction::kBookingDate] =
      QString::fromStdString(transaction.bookingDate);
  row[payload::keys::transaction::kValuta] =
      QString::fromStdString(transaction.valuta);
  row[payload::keys::common::kAmount] = transaction.amount;
  row[payload::keys::common::kStatus] = transaction.status;
  row[payload::keys::transaction::kActorId] =
      QString::fromStdString(transaction.actorId);
  row[payload::keys::transaction::kContractId] =
      QString::fromStdString(transaction.contractId);
  row[payload::keys::common::kMetadata] = QString();
  row[payload::keys::common::kType] =
      contractType.isEmpty() ? QString::fromStdString(transaction.contractType)
                             : contractType;
  row[payload::keys::transaction::kContractType] =
      row.value(payload::keys::common::kType);
  row[payload::keys::transaction::kAllocatable] = transaction.allocatable;
  row[payload::keys::transaction::kPropertyIds] =
      payload::mapper::toVariantStringList(transaction.propertyIds);
  row[QStringLiteral("propertyNames")] =
      payload::mapper::toVariantStringList(transaction.propertyNames);
  return row;
}

QVariantList
transactionRows(const std::vector<core::ports::workspace::TransactionSnapshot>&
                    transactions) {
  QVariantList rows;
  rows.reserve(static_cast<int>(transactions.size()));
  for (const auto& transaction : transactions) {
    rows.push_back(transactionRow(transaction));
  }
  return rows;
}

QVariantMap statementTransactionRow(
    const core::ports::workspace::TransactionSnapshot& transaction) {
  QVariantMap row;
  insertIdName(row, transaction);
  row.insert(payload::keys::transaction::kBookingDate,
             QString::fromStdString(transaction.bookingDate));
  return row;
}

QVariantMap importLogRow(const core::ports::workspace::ImportLogSnapshot& log) {
  const QString file = QString::fromStdString(log.file);
  const QString message = QString::fromStdString(log.message);
  QVariantMap row;
  row[payload::keys::importLog::kLogId] = QString::fromStdString(log.id);
  row[payload::keys::importLog::kTime] = QString::fromStdString(log.time);
  row[payload::keys::common::kType] = QString::fromStdString(log.type);
  row[payload::keys::importLog::kFile] = file;
  row[payload::keys::common::kStatus] = QString::fromStdString(log.status);
  row[payload::keys::importLog::kMessage] = message;
  row[payload::keys::importLog::kDraftAttached] = log.draftAttached;
  row[payload::keys::importLog::kDraftId] = QString::fromStdString(log.draftId);
  row[payload::keys::importLog::kStatementId] =
      QString::fromStdString(log.statementId);
  row[payload::keys::common::kDisplayTime] = QString::fromStdString(log.time);
  row[payload::keys::common::kDisplayTitle] = displayFileName(file);
  row[payload::keys::common::kDisplayStatusDetail] = message;
  return row;
}

QVariantMap exportLogRow(const core::ports::workspace::ExportLogSnapshot& log) {
  const QString targetPath = QString::fromStdString(log.targetPath);
  const QString message = QString::fromStdString(log.message);
  QVariantMap row;
  row[payload::keys::exportLog::kLogId] = QString::fromStdString(log.id);
  row[payload::keys::exportLog::kTime] = QString::fromStdString(log.time);
  row[payload::keys::exportLog::kFile] = targetPath;
  row[payload::keys::common::kStatus] = QString::fromStdString(log.status);
  row[payload::keys::exportLog::kMessage] = message;
  row[payload::keys::importLog::kDraftAttached] = false;
  row[payload::keys::importLog::kDraftId] = QString();
  row[payload::keys::importLog::kStatementId] = QString();
  row[payload::keys::common::kDisplayTime] = QString::fromStdString(log.time);
  row[payload::keys::common::kDisplayTitle] = displayFileName(targetPath);
  row[payload::keys::common::kDisplayStatusDetail] = message;
  return row;
}

QVariantMap dropdownRow(const QString& id, const QString& display,
                        const QString& name, const QString& type,
                        const QVariantList& actorIds,
                        const QVariantList& propertyIds,
                        const QString& allocatableMode) {
  return QVariantMap{
      {payload::keys::common::kId, id},
      {payload::keys::common::kDisplay, display},
      {payload::keys::common::kName, name},
      {payload::keys::common::kType, type},
      {payload::keys::contract::kActorIds, actorIds},
      {payload::keys::contract::kPropertyIds, propertyIds},
      {payload::keys::contract::kAllocatableMode, allocatableMode}};
}

} // namespace ui::workspace_payloads
