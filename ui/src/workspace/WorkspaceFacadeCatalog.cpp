/**
 * @file ui/src/workspace/WorkspaceFacadeCatalog.cpp
 * @brief Implements catalog and statement commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>
#include <vector>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/util/StringConversions.h"
#include "ui/workspace/WorkspaceRowProjector.h"

namespace ui {
namespace {

QStringList sortedStdStrings(const std::vector<std::string> &values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto &value : values) {
    const QString trimmed = QString::fromStdString(value).trimmed();
    if (!trimmed.isEmpty()) {
      out.push_back(trimmed);
    }
  }
  out.sort();
  return out;
}

QVariantMap catalogIdentityRow(const QString &id, const QString &name) {
  return {{payload::keys::common::kId, id},
          {payload::keys::common::kName, name},
          {payload::keys::common::kDisplay, name}};
}

QVariantMap catalogIdentityRow(
    const core::ports::workspace::WorkspaceIdentitySnapshot &identity) {
  if (identity.empty()) {
    return {};
  }
  QVariantMap row = catalogIdentityRow(QString::fromStdString(identity.id),
                                       QString::fromStdString(identity.name));
  if (!identity.type.empty()) {
    row.insert(payload::keys::common::kType,
               QString::fromStdString(identity.type));
  }
  if (!identity.actorIds.empty()) {
    row.insert(payload::keys::contract::kActorIds,
               sortedStdStrings(identity.actorIds));
  }
  if (!identity.propertyIds.empty()) {
    row.insert(payload::keys::contract::kPropertyIds,
               sortedStdStrings(identity.propertyIds));
  }
  if (!identity.allocatableMode.empty()) {
    row.insert(payload::keys::contract::kAllocatableMode,
               QString::fromStdString(identity.allocatableMode));
  }
  return row;
}

QVariantMap dropdownRow(const QString &id, const QString &display,
                        const QString &name = {}, const QString &type = {},
                        const QVariantList &actorIds = {},
                        const QVariantList &propertyIds = {},
                        const QString &allocatableMode =
                            QStringLiteral("mixed")) {
  return QVariantMap{{payload::keys::common::kId, id},
                     {payload::keys::common::kDisplay, display},
                     {payload::keys::common::kName, name},
                     {payload::keys::common::kType, type},
                     {payload::keys::contract::kActorIds, actorIds},
                     {payload::keys::contract::kPropertyIds, propertyIds},
                     {payload::keys::contract::kAllocatableMode,
                      allocatableMode}};
}

void prependEmptyDropdownRow(QVariantList &rows, const QString &display) {
  rows.push_front(dropdownRow({}, display));
}

QString catalogStringValue(const QVariant &value) {
  const QVariantMap map = value.toMap();
  if (!map.isEmpty()) {
    const QString valueText = map.value(QStringLiteral("value")).toString();
    if (!valueText.isEmpty()) {
      return valueText;
    }
    const QString source = map.value(QStringLiteral("source")).toString();
    if (!source.isEmpty()) {
      return source;
    }
    const QString id = map.value(payload::keys::common::kId).toString();
    if (!id.isEmpty()) {
      return id;
    }
  }
  return value.toString();
}

QVariantList normalizedCatalogIds(const QVariantList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const QVariant &value : values) {
    const QString id = catalogStringValue(value).trimmed();
    if (!id.isEmpty()) {
      out.push_back(id);
    }
  }
  return out;
}

QVariantMap catalogRowById(const QVariantList &rows, const QString &id) {
  const QString normalizedId = id.trimmed();
  if (normalizedId.isEmpty()) {
    return {};
  }
  for (const QVariant &value : rows) {
    const QVariantMap row = value.toMap();
    if (row.value(payload::keys::common::kId).toString() == normalizedId) {
      return row;
    }
  }
  return {};
}

QVariantMap withFormValue(const QVariantMap &base, const QString &key,
                          const QVariant &value) {
  QVariantMap out = base;
  out.insert(key, value);
  return out;
}

bool contractSupportsActor(const QVariantMap &contractRow,
                           const QString &actorId) {
  const QString targetActor = actorId.trimmed();
  if (targetActor.isEmpty() || contractRow.isEmpty()) {
    return true;
  }
  const QVariantList actorIds = normalizedCatalogIds(
      contractRow.value(payload::keys::contract::kActorIds).toList());
  return actorIds.contains(targetActor);
}

bool contractSupportsProperties(const QVariantMap &contractRow,
                                const QVariantList &propertyIds) {
  if (contractRow.isEmpty()) {
    return true;
  }
  const QVariantList normalizedPropertyIds = normalizedCatalogIds(propertyIds);
  const QVariantList allowedPropertyIds = normalizedCatalogIds(
      contractRow.value(payload::keys::contract::kPropertyIds).toList());
  for (const QVariant &propertyValue : normalizedPropertyIds) {
    if (!allowedPropertyIds.contains(propertyValue)) {
      return false;
    }
  }
  return true;
}

std::vector<core::ports::workspace::AliasSnapshot>
aliasSnapshots(const QStringList &aliases) {
  std::vector<core::ports::workspace::AliasSnapshot> out;
  out.reserve(aliases.size());
  for (const auto &alias : aliases) {
    const auto text = strings::toStdString(alias);
    out.push_back({text, {}, text, 0, {}, {}, {}});
  }
  return out;
}

std::vector<std::string> stdStrings(const QStringList &values) {
  return strings::toStdList(values);
}

core::ports::workspace::ActorCommand
makeActorCommand(const QString &id, const QString &name,
                 const QStringList &aliases, const QStringList &contractIds) {
  core::ports::workspace::ActorCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::PropertyCommand
makePropertyCommand(const QString &id, const QString &name,
                    const QStringList &aliases,
                    const QStringList &contractIds) {
  core::ports::workspace::PropertyCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::ContractCommand
makeContractCommand(const QString &id, const QString &name, const QString &type,
                    const QString &allocatableMode, const QStringList &actorIds,
                    const QStringList &propertyIds,
                    const QStringList &aliases) {
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
makeTransactionCommand(const QString &id, const QString &name,
                       const QString &bookingDate, const QString &valuta,
                       double amount, const QString &statementId,
                       const QString &insertAfterTransactionId, int status,
                       const QString &actorId, const QString &contractId,
                       bool allocatable, const QStringList &propertyIds) {
  core::ports::workspace::TransactionCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.bookingDate = strings::toStdString(bookingDate);
  command.valuta = strings::toStdString(valuta);
  command.amount = amount;
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

} // namespace

QVariantMap WorkspaceFacade::actorIdentityByName(const QString &name) const {
  if (!workspaceReader_) {
    return {};
  }
  return catalogIdentityRow(
      workspaceReader_->actorIdentityByName(strings::toStdString(name)));
}

QVariantMap WorkspaceFacade::propertyIdentityByName(const QString &name) const {
  if (!workspaceReader_) {
    return {};
  }
  return catalogIdentityRow(
      workspaceReader_->propertyIdentityByName(strings::toStdString(name)));
}

QVariantMap WorkspaceFacade::contractIdentityBySignature(
    const QString &name, const QString &type, const QStringList &actorIds,
    const QStringList &propertyIds) const {
  if (!workspaceReader_) {
    return {};
  }
  return catalogIdentityRow(workspaceReader_->contractIdentityBySignature(
      strings::toStdString(name), strings::toStdString(type),
      stdStrings(actorIds), stdStrings(propertyIds)));
}

QString WorkspaceFacade::nextContractName() const {
  if (!workspaceReader_) {
    return QStringLiteral("Contract 1");
  }
  return QString::fromStdString(workspaceReader_->nextContractName());
}

QVariantList WorkspaceFacade::actorRows() const {
  return cache_ ? buildActorRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::propertyRows() const {
  return cache_ ? buildPropertyRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::contractRows() const {
  return cache_ ? buildContractRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::analysisRows() const {
  return cache_ ? buildAnalysisRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::annualRows() const {
  return cache_ ? buildAnnualRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::statementRows() const {
  return cache_ ? buildStatementRows(*cache_) : QVariantList{};
}

QVariantList WorkspaceFacade::statementRowsWithTransactions() const {
  QVariantList rows = statementRows();
  for (QVariant &rowValue : rows) {
    QVariantMap row = rowValue.toMap();
    const QString statementId = row.value(QStringLiteral("id")).toString();
    row.insert(QStringLiteral("transactions"),
               statementTransactionRows(statementId));
    rowValue = row;
  }
  return rows;
}

QVariantList
WorkspaceFacade::statementTransactionRows(const QString &statementId) const {
  return cache_ ? buildStatementTransactionRows(*cache_, statementId)
                  : QVariantList{};
}

QVariantMap WorkspaceFacade::transactionRowById(const QString &id) const {
  if (!cache_ || id.isEmpty()) {
    return {};
  }
  auto &transactions = cache_->models().transactions();
  const int row = transactions.findRowById(id);
  return row >= 0 ? transactions.get(row) : QVariantMap{};
}

QVariantList WorkspaceFacade::actorDropdownRows() const {
  QVariantList rows = actorRows();
  prependEmptyDropdownRow(rows, tr("No actor"));
  return rows;
}

QVariantList WorkspaceFacade::propertyDropdownRows() const {
  return propertyRows();
}

QVariantList WorkspaceFacade::contractDropdownRows() const {
  QVariantList rows = contractRows();
  prependEmptyDropdownRow(rows, tr("No contract"));
  return rows;
}

QVariantMap WorkspaceFacade::transactionFormWithCatalogSelection(
    const QVariantMap &formData, const QVariantMap &changes) const {
  QVariantMap out = formData;
  const QVariantList contracts = contractRows();

  if (changes.contains(payload::keys::transaction::kContractId)) {
    const QString contractId =
        changes.value(payload::keys::transaction::kContractId)
            .toString()
            .trimmed();
    const QVariantMap contractRow = catalogRowById(contracts, contractId);
    const QVariantList actorIds = normalizedCatalogIds(
        contractRow.value(payload::keys::contract::kActorIds).toList());
    const QVariantList propertyIds = normalizedCatalogIds(
        contractRow.value(payload::keys::contract::kPropertyIds).toList());
    const QString actorId =
        actorIds.isEmpty() ? QString() : actorIds.first().toString();
    out = withFormValue(out, payload::keys::transaction::kContractId,
                        contractId);
    out = withFormValue(out, payload::keys::transaction::kActorId, actorId);
    out = withFormValue(out, payload::keys::transaction::kPropertyIds,
                        propertyIds);
  }

  if (changes.contains(payload::keys::transaction::kActorId)) {
    const QString actorId =
        changes.value(payload::keys::transaction::kActorId).toString().trimmed();
    const QString contractId =
        out.value(payload::keys::transaction::kContractId).toString().trimmed();
    const QVariantMap contractRow = catalogRowById(contracts, contractId);
    out = withFormValue(out, payload::keys::transaction::kActorId, actorId);
    out = withFormValue(
        out, payload::keys::transaction::kContractId,
        contractSupportsActor(contractRow, actorId) ? contractId : QString());
  }

  if (changes.contains(payload::keys::transaction::kPropertyIds)) {
    const QVariantList propertyIds = normalizedCatalogIds(
        changes.value(payload::keys::transaction::kPropertyIds).toList());
    const QString contractId =
        out.value(payload::keys::transaction::kContractId).toString().trimmed();
    const QVariantMap contractRow = catalogRowById(contracts, contractId);
    out = withFormValue(out, payload::keys::transaction::kPropertyIds,
                        propertyIds);
    out = withFormValue(out, payload::keys::transaction::kContractId,
                        contractSupportsProperties(contractRow, propertyIds)
                            ? contractId
                            : QString());
  }

  return out;
}

double WorkspaceFacade::amountForTransactionCommit(
    const QVariant &rawAmount, const QString &transactionId,
    double fallbackAmount) const {
  return cache_ ? cache_->amountForTransactionCommit(
                        rawAmount, transactionId, fallbackAmount)
                  : fallbackAmount;
}

void WorkspaceFacade::setTransactionPropertyIdsImmediate(
    const QString &transactionId, const QStringList &propertyIds) {
  if (cache_) {
    cache_->setTransactionPropertyIdsImmediate(transactionId, propertyIds);
  }
}

QVariantMap WorkspaceFacade::validateActor(const QString &id,
                                           const QString &name,
                                           const QStringList &aliases,
                                           const QStringList &contractIds) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateActor(
      makeActorCommand(id, name, aliases, contractIds)));
}

QString WorkspaceFacade::saveActor(const QString &id, const QString &name,
                                   const QStringList &aliases,
                                   const QStringList &contractIds) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::saveActor",
                                  "Actor save ignored; no writer bound");
    return {};
  }
  observability::traceWorkspace(
      "WorkspaceFacade::saveActor",
      id.isEmpty() ? "Actor create requested" : "Actor update requested",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"contractCount", std::to_string(contractIds.size())}});
  const auto command = makeActorCommand(id, name, aliases, contractIds);
  if (rejectInvalidCommand("WorkspaceFacade::saveActor",
                           workspaceWriter_->validateActor(command))) {
    return {};
  }
  if (id.isEmpty()) {
    const QString createdId =
        QString::fromStdString(workspaceWriter_->addActor(command));
    observability::traceWorkspace(
        "WorkspaceFacade::saveActor", "Actor created",
        {{observability::context::kId, createdId.toStdString()},
         {observability::context::kName, name.toStdString()}});
    return createdId;
  }
  workspaceWriter_->updateActor(command);
  observability::traceWorkspace(
      "WorkspaceFacade::saveActor", "Actor updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()}});
  return id;
}

void WorkspaceFacade::deleteActor(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteActor", "Actor delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteActor(strings::toStdString(id));
  }
}

QVariantMap WorkspaceFacade::validateProperty(
    const QString &id, const QString &name, const QStringList &aliases,
    const QStringList &contractIds) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateProperty(
      makePropertyCommand(id, name, aliases, contractIds)));
}

QString WorkspaceFacade::saveProperty(const QString &id, const QString &name,
                                      const QStringList &aliases,
                                      const QStringList &contractIds) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::saveProperty",
                                  "Property save ignored; no writer bound");
    return {};
  }
  observability::traceWorkspace(
      "WorkspaceFacade::saveProperty",
      id.isEmpty() ? "Property create requested" : "Property update requested",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"contractCount", std::to_string(contractIds.size())}});
  const auto command = makePropertyCommand(id, name, aliases, contractIds);
  if (rejectInvalidCommand("WorkspaceFacade::saveProperty",
                           workspaceWriter_->validateProperty(command))) {
    return {};
  }
  if (id.isEmpty()) {
    const QString createdId =
        QString::fromStdString(workspaceWriter_->addProperty(command));
    observability::traceWorkspace(
        "WorkspaceFacade::saveProperty", "Property created",
        {{observability::context::kId, createdId.toStdString()},
         {observability::context::kName, name.toStdString()}});
    return createdId;
  }
  workspaceWriter_->updateProperty(command);
  observability::traceWorkspace(
      "WorkspaceFacade::saveProperty", "Property updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()}});
  return id;
}

void WorkspaceFacade::deleteProperty(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteProperty", "Property delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteProperty(strings::toStdString(id));
  }
}

QVariantMap WorkspaceFacade::validateContract(
    const QString &id, const QString &name, const QString &type,
    const QStringList &actorIds, const QStringList &propertyIds,
    const QStringList &aliases, const QString &allocatableMode) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateContract(
      makeContractCommand(id, name, type, allocatableMode, actorIds,
                          propertyIds, aliases)));
}

QString WorkspaceFacade::saveContract(const QString &id, const QString &name,
                                      const QString &type,
                                      const QStringList &actorIds,
                                      const QStringList &propertyIds,
                                      const QStringList &aliases,
                                      const QString &allocatableMode) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::saveContract",
                                  "Contract save ignored; no writer bound");
    return {};
  }
  observability::traceWorkspace(
      "WorkspaceFacade::saveContract",
      id.isEmpty() ? "Contract create requested" : "Contract update requested",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"type", type.toStdString()},
       {"actorCount", std::to_string(actorIds.size())},
       {"propertyCount", std::to_string(propertyIds.size())}});
  const auto command =
      makeContractCommand(id, name, type, allocatableMode, actorIds,
                          propertyIds, aliases);
  if (rejectInvalidCommand("WorkspaceFacade::saveContract",
                           workspaceWriter_->validateContract(command))) {
    return {};
  }
  if (id.isEmpty()) {
    const QString createdId =
        QString::fromStdString(workspaceWriter_->addContract(command));
    observability::traceWorkspace(
        "WorkspaceFacade::saveContract", "Contract created",
        {{observability::context::kId, createdId.toStdString()},
         {observability::context::kName, name.toStdString()}});
    return createdId;
  }
  workspaceWriter_->updateContract(command);
  observability::traceWorkspace(
      "WorkspaceFacade::saveContract", "Contract updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()}});
  return id;
}

void WorkspaceFacade::deleteContract(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteContract", "Contract delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteContract(strings::toStdString(id));
  }
}

QVariantMap WorkspaceFacade::validateStatement(const QString &id,
                                               const QString &name) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  return validationResultToMap(workspaceWriter_->validateStatement(command));
}

QString WorkspaceFacade::addStatement(const QString &name) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::addStatement",
                                  "Statement create ignored; no writer bound");
    return {};
  }
  core::ports::workspace::StatementCommand command;
  command.name = strings::toStdString(name);
  if (rejectInvalidCommand("WorkspaceFacade::addStatement",
                           workspaceWriter_->validateStatement(command))) {
    return {};
  }
  const QString createdId =
      QString::fromStdString(workspaceWriter_->addStatement(command));
  observability::traceWorkspace(
      "WorkspaceFacade::addStatement", "Statement created",
      {{observability::context::kId, createdId.toStdString()},
       {observability::context::kName, name.toStdString()}});
  return createdId;
}

void WorkspaceFacade::updateStatement(const QString &id, const QString &name) {
  if (!workspaceWriter_) {
    return;
  }
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  if (rejectInvalidCommand("WorkspaceFacade::updateStatement",
                           workspaceWriter_->validateStatement(command))) {
    return;
  }
  workspaceWriter_->updateStatement(command);
  observability::traceWorkspace(
      "WorkspaceFacade::updateStatement", "Statement updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()}});
}

void WorkspaceFacade::deleteStatement(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteStatement", "Statement delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteStatement(strings::toStdString(id));
  }
}

QVariantMap WorkspaceFacade::validateTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    const QString &valuta, double amount, const QString &statementId,
    int status, const QString &actorId, const QString &contractId,
    bool allocatable, const QStringList &propertyIds) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateTransaction(
      makeTransactionCommand(id, name, bookingDate, valuta, amount, statementId,
                             {}, status, actorId, contractId, allocatable,
                             propertyIds)));
}

QString WorkspaceFacade::addTransaction(
    const QString &name, const QString &bookingDate, const QString &valuta,
    double amount, const QString &statementId, int status,
    const QString &actorId, const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::addTransaction",
                                  "Transaction create ignored; no writer bound");
    return {};
  }
  const auto command =
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             {}, status, actorId, contractId, allocatable,
                             propertyIds);
  if (rejectInvalidCommand("WorkspaceFacade::addTransaction",
                           workspaceWriter_->validateTransaction(command))) {
    return {};
  }
  const QString createdId =
      QString::fromStdString(workspaceWriter_->addTransaction(command));
  observability::traceWorkspace(
      "WorkspaceFacade::addTransaction", "Transaction created",
      {{observability::context::kId, createdId.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"statementId", statementId.toStdString()},
       {"amount", std::to_string(amount)}});
  return createdId;
}

QString WorkspaceFacade::insertTransactionAfter(
    const QString &afterTransactionId, const QString &name,
    const QString &bookingDate, const QString &valuta, double amount,
    const QString &statementId, int status, const QString &actorId,
    const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::insertTransactionAfter",
        "Transaction insert ignored; no writer bound");
    return {};
  }
  const auto command =
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             afterTransactionId, status, actorId, contractId,
                             allocatable, propertyIds);
  if (rejectInvalidCommand("WorkspaceFacade::insertTransactionAfter",
                           workspaceWriter_->validateTransaction(command))) {
    return {};
  }
  const QString createdId =
      QString::fromStdString(workspaceWriter_->addTransaction(command));
  observability::traceWorkspace(
      "WorkspaceFacade::insertTransactionAfter", "Transaction inserted",
      {{observability::context::kId, createdId.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"statementId", statementId.toStdString()},
       {"afterTransactionId", afterTransactionId.toStdString()}});
  return createdId;
}

void WorkspaceFacade::updateTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    const QString &valuta, double amount, const QString &statementId,
    int status, const QString &actorId, const QString &contractId,
    bool allocatable, const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    return;
  }
  const auto command = makeTransactionCommand(
      id, name, bookingDate, valuta, amount, statementId, {}, status, actorId,
      contractId, allocatable, propertyIds);
  if (rejectInvalidCommand("WorkspaceFacade::updateTransaction",
                           workspaceWriter_->validateTransaction(command))) {
    return;
  }
  workspaceWriter_->updateTransaction(command);
  observability::traceWorkspace(
      "WorkspaceFacade::updateTransaction", "Transaction updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"statementId", statementId.toStdString()},
       {"amount", std::to_string(amount)}});
}

void WorkspaceFacade::deleteTransaction(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteTransaction", "Transaction delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteTransaction(strings::toStdString(id));
  }
}

} // namespace ui
