/**
 * @file ui/src/workspace/WorkspaceCommands.cpp
 * @brief Implements UI-side workspace command submission.
 */

#include "ui/workspace/WorkspaceCommands.h"

#include <QDateTime>
#include <QUuid>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <utility>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"
#include "ui/util/StringConversions.h"
#include "ui/workspace/WorkspacePayloads.h"

namespace ui {

namespace {

std::vector<std::string> stdStrings(const QStringList& values) {
  return strings::toStdList(values);
}

std::vector<core::ports::workspace::AliasSnapshot>
aliasSnapshots(const QStringList& aliases) {
  std::vector<core::ports::workspace::AliasSnapshot> out;
  out.reserve(aliases.size());
  for (const auto& alias : aliases) {
    const auto text = strings::toStdString(alias);
    out.push_back({text, {}, text, 0, {}, {}, {}});
  }
  return out;
}

core::ports::workspace::ActorCommand
makeActorCommand(const QString& id, const QString& name,
                 const QStringList& aliases, const QStringList& contractIds) {
  core::ports::workspace::ActorCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::PropertyCommand
makePropertyCommand(const QString& id, const QString& name,
                    const QStringList& aliases,
                    const QStringList& contractIds) {
  core::ports::workspace::PropertyCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::ContractCommand
makeContractCommand(const QString& id, const QString& name, const QString& type,
                    const QString& allocatableMode, const QStringList& actorIds,
                    const QStringList& propertyIds,
                    const QStringList& aliases) {
  core::ports::workspace::ContractCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.type = strings::toStdString(type);
  command.allocatableMode = strings::toStdString(allocatableMode);
  command.actorIds = stdStrings(actorIds);
  command.propertyIds = stdStrings(propertyIds);
  command.aliases = aliasSnapshots(aliases);
  return command;
}

core::ports::workspace::TransactionCommand
makeTransactionCommand(const QString& id, const QString& name,
                       const QString& bookingDate, const QString& valuta,
                       const QVariant& amount, const QString& statementId,
                       const QString& insertAfterTransactionId, int status,
                       const QString& actorId, const QString& contractId,
                       bool allocatable, const QStringList& propertyIds) {
  core::ports::workspace::TransactionCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.bookingDate = strings::toStdString(bookingDate);
  command.valuta = strings::toStdString(valuta);
  command.amountText = strings::toStdString(amount.toString());
  command.statementId = strings::toStdString(statementId);
  command.insertAfterTransactionId =
      strings::toStdString(insertAfterTransactionId);
  command.status = status;
  command.actorId = strings::toStdString(actorId);
  command.contractId = strings::toStdString(contractId);
  command.allocatable = allocatable;
  command.propertyIds = stdStrings(propertyIds);
  return command;
}

core::ports::workspace::TransactionCommand
makeTransactionCommand(const QVariantMap& state,
                       const QString& statementId = {}) {
  return makeTransactionCommand(
      state.value(QStringLiteral("id")).toString(),
      state.value(QStringLiteral("name")).toString(),
      state.value(QStringLiteral("bookingDate")).toString(),
      state.value(QStringLiteral("valuta")).toString(),
      state.value(QStringLiteral("amount")), statementId, {},
      state.value(payload::keys::common::kStatus).toInt(),
      state.value(payload::keys::transaction::kActorId).toString(),
      state.value(payload::keys::transaction::kContractId).toString(),
      state.value(QStringLiteral("allocatable")).toBool(),
      payload::mapper::toQStringList(
          state.value(payload::keys::transaction::kPropertyIds).toList()));
}

core::ports::analysis::AnalysisConfigInput
analysisConfigInput(const QString& type, const QVariantMap& config) {
  core::ports::analysis::AnalysisConfigInput input;
  input.type = strings::toStdString(type);
  input.plotType =
      strings::toStdString(config.value(QStringLiteral("plotType")).toString());
  input.plotMeasure = strings::toStdString(
      config.value(QStringLiteral("plotMeasure")).toString());
  input.propertyIds = stdStrings(payload::mapper::toQStringList(
      config.value(QStringLiteral("propertyIds")).toList()));
  input.contractTypes = stdStrings(payload::mapper::toQStringList(
      config.value(QStringLiteral("contractTypes")).toList()));
  input.taxPercent = config.value(QStringLiteral("taxPercent")).toDouble();
  return input;
}

core::ports::analysis::AnalysisFilterSelection
analysisFilterSelection(const QVariantMap& filter) {
  core::ports::analysis::AnalysisFilterSelection selection;
  selection.dateField = strings::toStdString(
      filter.value(QStringLiteral("dateField")).toString());
  selection.dateMode =
      strings::toStdString(filter.value(QStringLiteral("dateMode")).toString());
  selection.year =
      strings::toStdString(filter.value(QStringLiteral("year")).toString());
  selection.dateFrom =
      strings::toStdString(filter.value(QStringLiteral("dateFrom")).toString());
  selection.dateTo =
      strings::toStdString(filter.value(QStringLiteral("dateTo")).toString());
  selection.propertyIds = stdStrings(payload::mapper::toQStringList(
      filter.value(QStringLiteral("propertyIds")).toList()));
  selection.propertyIdsUnassigned =
      filter.value(QStringLiteral("propertyIdsNone")).toBool();
  selection.contractTypes = stdStrings(payload::mapper::toQStringList(
      filter.value(QStringLiteral("contractTypes")).toList()));
  selection.contractTypesUnassigned =
      filter.value(QStringLiteral("contractTypesNone")).toBool();
  selection.allocatableMode = strings::toStdString(
      filter.value(QStringLiteral("allocatableMode")).toString());
  return selection;
}

core::ports::workspace::AnalysisCommand
makeAnalysisCommand(const QString& id, const QString& name, const QString& type,
                    const QVariantMap& config, const QVariantMap& filter,
                    const QString& exportFormat, bool includeCalcAdjustments,
                    const QVariantList& snapshotTransactions) {
  core::ports::workspace::AnalysisCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.type = strings::toStdString(type);
  command.config = analysisConfigInput(type, config);
  command.filter = analysisFilterSelection(filter);
  command.exportFormat = strings::toStdString(exportFormat);
  command.includeCalculationAdjustments = includeCalcAdjustments;
  command.snapshotTransactions.reserve(
      static_cast<std::size_t>(snapshotTransactions.size()));
  for (const auto& value : snapshotTransactions) {
    const QVariantMap row = value.toMap();
    core::ports::workspace::TransactionSnapshot tx;
    tx.id = strings::toStdString(
        row.value(QStringLiteral("id"),
                  row.value(QStringLiteral("transactionId")))
            .toString());
    tx.name = strings::toStdString(
        row.value(QStringLiteral("name"),
                  row.value(QStringLiteral("transactionName")))
            .toString());
    tx.bookingDate =
        strings::toStdString(row.value(QStringLiteral("bookingDate"),
                                       row.value(QStringLiteral("date")))
                                 .toString());
    tx.valuta =
        strings::toStdString(row.value(QStringLiteral("valuta")).toString());
    tx.amount = row.value(QStringLiteral("amount")).toDouble();
    tx.status = row.value(QStringLiteral("status")).toInt();
    tx.contractId = strings::toStdString(
        row.value(QStringLiteral("contractId")).toString());
    tx.contractType = strings::toStdString(
        row.value(QStringLiteral("contractType")).toString());
    tx.actorId =
        strings::toStdString(row.value(QStringLiteral("actorId")).toString());
    tx.statementId = strings::toStdString(
        row.value(QStringLiteral("statementId")).toString());
    tx.allocatable = row.value(QStringLiteral("allocatable")).toBool();
    tx.propertyIds = stdStrings(payload::mapper::toQStringList(
        row.value(QStringLiteral("propertyIds")).toList()));
    tx.propertyNames = stdStrings(payload::mapper::toQStringList(
        row.value(QStringLiteral("propertyNames")).toList()));
    command.snapshotTransactions.push_back(std::move(tx));
  }
  return command;
}

std::vector<std::pair<std::string, double>>
analysisAdjustmentPairs(const QVariantMap& adjustments) {
  std::vector<std::pair<std::string, double>> out;
  out.reserve(static_cast<std::size_t>(adjustments.size()));
  for (auto it = adjustments.constBegin(); it != adjustments.constEnd(); ++it) {
    const QString id = it.key().trimmed();
    if (!id.isEmpty()) {
      out.emplace_back(strings::toStdString(id), it.value().toDouble());
    }
  }
  return out;
}

core::ports::workspace::AnnualCommand
makeAnnualCommand(const QString& id, const QString& name, int year,
                  const QStringList& analysisIds) {
  core::ports::workspace::AnnualCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.year = year;
  command.assignedAnalysisIds = stdStrings(analysisIds);
  return command;
}

} // namespace

WorkspaceCommands::WorkspaceCommands(WorkspaceStore& store, QObject* parent)
    : QObject(parent), store_(store) {}

void WorkspaceCommands::refreshStoreFromReader() {
  if (store_.reader()) {
    store_.loadFromState(store_.reader()->workspaceSnapshot());
  }
}

bool WorkspaceCommands::rejectInvalidCommand(
    const char* origin,
    const core::ports::workspace::ValidationResult& result) const {
  if (result.valid()) {
    return false;
  }
  core::errors::ErrorContext context{
      {"issueCount", std::to_string(result.issues.size())}};
  if (!result.issues.empty()) {
    context.emplace_back(observability::context::kError,
                         result.issues.front().message);
    context.emplace_back("field", result.issues.front().field);
    context.emplace_back("code", result.issues.front().code);
  }
  observability::traceWorkspace(origin, "Workspace command rejected",
                                std::move(context));
  return true;
}

void WorkspaceCommands::runStorageOperation(
    const QString& operation, const std::function<void()>& action) {
  if (!store_.writer()) {
    observability::traceWorkspace(
        "WorkspaceCommands::runStorageOperation",
        "Storage operation ignored because no workspace writer is bound",
        {{observability::context::kOperation, operation.toStdString()}});
    return;
  }
  try {
    action();
    emit operationSucceeded(operation);
  } catch (const std::exception& ex) {
    emit operationFailed(operation, QString::fromUtf8(ex.what()));
  }
}

std::optional<core::ports::workspace::StatementDraftSnapshot>
WorkspaceCommands::statementDraftSnapshot(const QString& draftId) const {
  return store_.reader() ? store_.reader()->statementDraftSnapshot(
                               strings::toStdString(draftId))
                         : std::nullopt;
}

QString WorkspaceCommands::finalizeStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot& draft) {
  if (!store_.writer()) {
    return {};
  }
  core::ports::workspace::FinalizeStatementDraftCommand command;
  command.draft = draft;
  const QString id =
      QString::fromStdString(store_.writer()->finalizeStatementDraft(command));
  refreshStoreFromReader();
  return id;
}

void WorkspaceCommands::saveStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot& draft) {
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::StatementDraftCommand command;
  command.draft = draft;
  store_.writer()->saveStatementDraft(command);
  refreshStoreFromReader();
}

void WorkspaceCommands::clearStatementDraft(const QString& draftId) {
  if (!store_.writer()) {
    return;
  }
  store_.writer()->clearStatementDraft(strings::toStdString(draftId));
  refreshStoreFromReader();
}

void WorkspaceCommands::saveImportLog(
    const core::ports::workspace::ImportLogSnapshot& log) {
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::ImportLogCommand command;
  command.log = log;
  store_.writer()->saveImportLog(command);
}

void WorkspaceCommands::upsertImportLog(
    const QString& logId, const QString& status, const QString& message,
    bool draftAttached, const QString& draftId, const QString& statementId,
    const QString& importFile) {
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::ImportLogSnapshot log;
  const QString resolvedLogId =
      logId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                      : logId;
  for (const auto& existing : store_.snapshot().importLogs) {
    if (QString::fromStdString(existing.id) == resolvedLogId) {
      log = existing;
      break;
    }
  }

  log.id = resolvedLogId.toStdString();
  log.time = QDateTime::currentDateTime()
                 .toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))
                 .toStdString();
  log.status = status.toStdString();
  log.message = message.toStdString();
  log.draftAttached = draftAttached;
  log.statementId = statementId.toStdString();
  if (log.type.empty()) {
    log.type = "statement";
  }
  if (!importFile.isEmpty()) {
    log.file = importFile.toStdString();
  }
  const QString resolvedDraftId =
      draftAttached ? (draftId.isEmpty() ? resolvedLogId : draftId) : QString();
  log.draftId = resolvedDraftId.toStdString();
  log.statementDraftIds.clear();
  if (draftAttached && !resolvedDraftId.isEmpty()) {
    log.statementDraftIds.push_back(resolvedDraftId.toStdString());
  }
  saveImportLog(log);
}

void WorkspaceCommands::deleteImportLog(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteImportLog(strings::toStdString(id));
  }
}

void WorkspaceCommands::saveExportLog(
    const core::ports::workspace::ExportLogSnapshot& log) {
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::ExportLogCommand command;
  command.log = log;
  store_.writer()->saveExportLog(command);
}

void WorkspaceCommands::deleteExportLog(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteExportLog(strings::toStdString(id));
  }
}

void WorkspaceCommands::newFile(const QString& path) {
  runStorageOperation(QStringLiteral("newFile"), [&]() {
    store_.writer()->newFile(strings::toEncodedPath(path));
  });
}

void WorkspaceCommands::openFile(const QString& path) {
  runStorageOperation(QStringLiteral("openFile"), [&]() {
    store_.writer()->openFile(strings::toEncodedPath(path));
  });
}

void WorkspaceCommands::saveFile() {
  runStorageOperation(QStringLiteral("saveFile"), [&]() {
    if (beforeStorageSave_) {
      beforeStorageSave_();
    }
    store_.writer()->saveFile();
  });
}

void WorkspaceCommands::saveFileAs(const QString& path) {
  runStorageOperation(QStringLiteral("saveFileAs"), [&]() {
    if (beforeStorageSave_) {
      beforeStorageSave_();
    }
    store_.writer()->saveFileAs(strings::toEncodedPath(path));
  });
}

void WorkspaceCommands::setBeforeStorageSaveCallback(
    std::function<void()> callback) {
  beforeStorageSave_ = std::move(callback);
}

QVariantMap
WorkspaceCommands::validateActor(const QString& id, const QString& name,
                                 const QStringList& aliases,
                                 const QStringList& contractIds) const {
  return store_.writer() ? workspace_payloads::validationResult(
                               store_.writer()->validateActor(makeActorCommand(
                                   id, name, aliases, contractIds)))
                         : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::saveActor(const QString& id, const QString& name,
                                     const QStringList& aliases,
                                     const QStringList& contractIds) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makeActorCommand(id, name, aliases, contractIds);
  if (rejectInvalidCommand("WorkspaceCommands::saveActor",
                           store_.writer()->validateActor(command))) {
    return {};
  }
  return id.isEmpty()
             ? QString::fromStdString(store_.writer()->addActor(command))
             : (store_.writer()->updateActor(command), id);
}

void WorkspaceCommands::deleteActor(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteActor(strings::toStdString(id));
  }
}

QVariantMap
WorkspaceCommands::validateProperty(const QString& id, const QString& name,
                                    const QStringList& aliases,
                                    const QStringList& contractIds) const {
  return store_.writer()
             ? workspace_payloads::validationResult(
                   store_.writer()->validateProperty(
                       makePropertyCommand(id, name, aliases, contractIds)))
             : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::saveProperty(const QString& id, const QString& name,
                                        const QStringList& aliases,
                                        const QStringList& contractIds) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makePropertyCommand(id, name, aliases, contractIds);
  if (rejectInvalidCommand("WorkspaceCommands::saveProperty",
                           store_.writer()->validateProperty(command))) {
    return {};
  }
  return id.isEmpty()
             ? QString::fromStdString(store_.writer()->addProperty(command))
             : (store_.writer()->updateProperty(command), id);
}

void WorkspaceCommands::deleteProperty(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteProperty(strings::toStdString(id));
  }
}

QVariantMap WorkspaceCommands::validateContract(
    const QString& id, const QString& name, const QString& type,
    const QStringList& actorIds, const QStringList& propertyIds,
    const QStringList& aliases, const QString& allocatableMode) const {
  return store_.writer()
             ? workspace_payloads::validationResult(
                   store_.writer()->validateContract(
                       makeContractCommand(id, name, type, allocatableMode,
                                           actorIds, propertyIds, aliases)))
             : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::saveContract(const QString& id, const QString& name,
                                        const QString& type,
                                        const QStringList& actorIds,
                                        const QStringList& propertyIds,
                                        const QStringList& aliases,
                                        const QString& allocatableMode) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makeContractCommand(id, name, type, allocatableMode,
                                           actorIds, propertyIds, aliases);
  if (rejectInvalidCommand("WorkspaceCommands::saveContract",
                           store_.writer()->validateContract(command))) {
    return {};
  }
  return id.isEmpty()
             ? QString::fromStdString(store_.writer()->addContract(command))
             : (store_.writer()->updateContract(command), id);
}

void WorkspaceCommands::deleteContract(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteContract(strings::toStdString(id));
  }
}

QVariantMap WorkspaceCommands::validateStatement(const QString& id,
                                                 const QString& name) const {
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  return store_.writer() ? workspace_payloads::validationResult(
                               store_.writer()->validateStatement(command))
                         : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::addStatement(const QString& name) {
  if (!store_.writer()) {
    return {};
  }
  core::ports::workspace::StatementCommand command;
  command.name = strings::toStdString(name);
  if (rejectInvalidCommand("WorkspaceCommands::addStatement",
                           store_.writer()->validateStatement(command))) {
    return {};
  }
  return QString::fromStdString(store_.writer()->addStatement(command));
}

QString WorkspaceCommands::addStatementWithTransactions(
    const QString& name, const QVariantList& transactionStates) {
  if (!store_.writer()) {
    return {};
  }
  core::ports::workspace::StatementWithTransactionsCommand command;
  command.statement.name = strings::toStdString(name);
  command.transactions.reserve(
      static_cast<std::size_t>(transactionStates.size()));
  for (const auto& stateValue : transactionStates) {
    command.transactions.push_back(makeTransactionCommand(stateValue.toMap()));
  }
  if (rejectInvalidCommand(
          "WorkspaceCommands::addStatementWithTransactions",
          store_.writer()->validateStatementWithTransactions(command))) {
    return {};
  }
  return QString::fromStdString(
      store_.writer()->addStatementWithTransactions(command));
}

void WorkspaceCommands::updateStatement(const QString& id,
                                        const QString& name) {
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  if (!rejectInvalidCommand("WorkspaceCommands::updateStatement",
                            store_.writer()->validateStatement(command))) {
    store_.writer()->updateStatement(command);
  }
}

void WorkspaceCommands::deleteStatement(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteStatement(strings::toStdString(id));
  }
}

QVariantMap WorkspaceCommands::validateTransaction(
    const QString& id, const QString& name, const QString& bookingDate,
    const QString& valuta, const QVariant& amount, const QString& statementId,
    int status, const QString& actorId, const QString& contractId,
    bool allocatable, const QStringList& propertyIds) const {
  return store_.writer()
             ? workspace_payloads::validationResult(
                   store_.writer()->validateTransaction(makeTransactionCommand(
                       id, name, bookingDate, valuta, amount, statementId, {},
                       status, actorId, contractId, allocatable, propertyIds)))
             : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::addTransaction(
    const QString& name, const QString& bookingDate, const QString& valuta,
    const QVariant& amount, const QString& statementId, int status,
    const QString& actorId, const QString& contractId, bool allocatable,
    const QStringList& propertyIds) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makeTransactionCommand(
      {}, name, bookingDate, valuta, amount, statementId, {}, status, actorId,
      contractId, allocatable, propertyIds);
  if (rejectInvalidCommand("WorkspaceCommands::addTransaction",
                           store_.writer()->validateTransaction(command))) {
    return {};
  }
  return QString::fromStdString(store_.writer()->addTransaction(command));
}

QString WorkspaceCommands::insertTransactionAfter(
    const QString& afterTransactionId, const QString& name,
    const QString& bookingDate, const QString& valuta, const QVariant& amount,
    const QString& statementId, int status, const QString& actorId,
    const QString& contractId, bool allocatable,
    const QStringList& propertyIds) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makeTransactionCommand(
      {}, name, bookingDate, valuta, amount, statementId, afterTransactionId,
      status, actorId, contractId, allocatable, propertyIds);
  if (rejectInvalidCommand("WorkspaceCommands::insertTransactionAfter",
                           store_.writer()->validateTransaction(command))) {
    return {};
  }
  return QString::fromStdString(store_.writer()->addTransaction(command));
}

void WorkspaceCommands::updateTransaction(
    const QString& id, const QString& name, const QString& bookingDate,
    const QString& valuta, const QVariant& amount, const QString& statementId,
    int status, const QString& actorId, const QString& contractId,
    bool allocatable, const QStringList& propertyIds) {
  if (!store_.writer()) {
    return;
  }
  const auto command = makeTransactionCommand(
      id, name, bookingDate, valuta, amount, statementId, {}, status, actorId,
      contractId, allocatable, propertyIds);
  if (!rejectInvalidCommand("WorkspaceCommands::updateTransaction",
                            store_.writer()->validateTransaction(command))) {
    store_.writer()->updateTransaction(command);
  }
}

void WorkspaceCommands::deleteTransaction(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteTransaction(strings::toStdString(id));
  }
}

QString WorkspaceCommands::addAnalysis(const QString& name, const QString& type,
                                       const QVariantMap& config,
                                       const QVariantMap& filter,
                                       const QString& exportFormat,
                                       bool includeCalcAdjustments,
                                       const QVariantList& snapshotTransactions,
                                       const QVariantMap& adjustments) {
  if (!store_.writer()) {
    return {};
  }
  auto command =
      makeAnalysisCommand({}, name, type, config, filter, exportFormat,
                          includeCalcAdjustments, snapshotTransactions);
  command.adjustments = analysisAdjustmentPairs(adjustments);
  if (rejectInvalidCommand("WorkspaceCommands::addAnalysis",
                           store_.writer()->validateAnalysis(command))) {
    return {};
  }
  return QString::fromStdString(store_.writer()->addAnalysis(command));
}

void WorkspaceCommands::updateAnalysis(
    const QString& id, const QString& name, const QString& type,
    const QVariantMap& config, const QVariantMap& filter,
    const QString& exportFormat, bool includeCalcAdjustments,
    const QVariantList& snapshotTransactions, const QVariantMap& adjustments) {
  if (!store_.writer()) {
    return;
  }
  auto command =
      makeAnalysisCommand(id, name, type, config, filter, exportFormat,
                          includeCalcAdjustments, snapshotTransactions);
  command.adjustments = analysisAdjustmentPairs(adjustments);
  if (!rejectInvalidCommand("WorkspaceCommands::updateAnalysis",
                            store_.writer()->validateAnalysis(command))) {
    store_.writer()->updateAnalysis(command);
  }
}

void WorkspaceCommands::deleteAnalysis(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteAnalysis(strings::toStdString(id));
  }
}

void WorkspaceCommands::updateAnalysisExportFormat(
    const QString& analysisId, const QString& exportFormat) {
  const std::string target = strings::toStdString(analysisId.trimmed());
  const auto& analyses = store_.snapshot().analyses;
  const auto it =
      std::find_if(analyses.begin(), analyses.end(), [&](const auto& analysis) {
        return analysis.id == target;
      });
  if (it == analyses.end()) {
    return;
  }
  if (!store_.writer()) {
    return;
  }
  core::ports::workspace::AnalysisCommand command = makeAnalysisCommand(
      analysisId, QString::fromStdString(it->name),
      QString::fromStdString(it->type),
      workspace_payloads::analysisConfigRow(it->config),
      workspace_payloads::analysisFilterRow(it->filter), exportFormat,
      it->includeCalculationAdjustments,
      workspace_payloads::transactionRows(it->snapshotTransactions));
  command.adjustments = it->adjustments;
  if (!rejectInvalidCommand("WorkspaceCommands::updateAnalysisExportFormat",
                            store_.writer()->validateAnalysis(command))) {
    store_.writer()->updateAnalysis(command);
  }
}

QVariantMap
WorkspaceCommands::validateAnnual(const QString& id, const QString& name,
                                  int year,
                                  const QStringList& analysisIds) const {
  return store_.writer()
             ? workspace_payloads::validationResult(
                   store_.writer()->validateAnnual(
                       makeAnnualCommand(id, name, year, analysisIds)))
             : workspace_payloads::validationResult({});
}

QString WorkspaceCommands::addAnnual(const QString& name, int year,
                                     const QStringList& analysisIds) {
  if (!store_.writer()) {
    return {};
  }
  const auto command = makeAnnualCommand({}, name, year, analysisIds);
  if (rejectInvalidCommand("WorkspaceCommands::addAnnual",
                           store_.writer()->validateAnnual(command))) {
    return {};
  }
  return QString::fromStdString(store_.writer()->addAnnual(command));
}

void WorkspaceCommands::updateAnnual(const QString& id, const QString& name,
                                     int year, const QStringList& analysisIds) {
  if (!store_.writer()) {
    return;
  }
  const auto command = makeAnnualCommand(id, name, year, analysisIds);
  if (!rejectInvalidCommand("WorkspaceCommands::updateAnnual",
                            store_.writer()->validateAnnual(command))) {
    store_.writer()->updateAnnual(command);
  }
}

void WorkspaceCommands::deleteAnnual(const QString& id) {
  if (store_.writer()) {
    store_.writer()->deleteAnnual(strings::toStdString(id));
  }
}

} // namespace ui
