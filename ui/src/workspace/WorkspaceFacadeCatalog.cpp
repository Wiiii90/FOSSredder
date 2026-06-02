/**
 * @file ui/src/workspace/WorkspaceFacadeCatalog.cpp
 * @brief Implements catalog and statement commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>
#include <vector>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceRowProjector.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {
namespace {

QString contractBaseName() { return QStringLiteral("Contract"); }

int trailingContractIndex(const QString &value) {
  const QString simplified = value.trimmed().simplified();
  const QString prefix = contractBaseName() + QStringLiteral(" ");
  if (!simplified.startsWith(prefix, Qt::CaseInsensitive)) {
    return -1;
  }
  bool ok = false;
  const int index = simplified.mid(prefix.size()).toInt(&ok);
  return ok ? index : -1;
}

QStringList sortedTrimmed(QStringList values) {
  QStringList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty()) {
      out.push_back(trimmed);
    }
  }
  out.sort();
  return out;
}

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

bool sameStringSet(const std::vector<std::string> &lhs,
                   const QStringList &rhs) {
  return sortedStdStrings(lhs) == sortedTrimmed(rhs);
}

QVariantMap catalogIdentityRow(const QString &id, const QString &name) {
  return {{payload::keys::common::kId, id},
          {payload::keys::common::kName, name},
          {payload::keys::common::kDisplay, name}};
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
  const QString target = strings::normalizedText(name);
  if (target.isEmpty()) {
    return {};
  }
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  for (const auto &actor : snapshot.actors) {
    const QString existingName = QString::fromStdString(actor.name);
    if (strings::normalizedText(existingName) == target) {
      return catalogIdentityRow(QString::fromStdString(actor.id),
                                existingName);
    }
  }
  return {};
}

QVariantMap WorkspaceFacade::propertyIdentityByName(const QString &name) const {
  if (!workspaceReader_) {
    return {};
  }
  const QString target = strings::normalizedText(name);
  if (target.isEmpty()) {
    return {};
  }
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  for (const auto &property : snapshot.properties) {
    const QString existingName = QString::fromStdString(property.name);
    if (strings::normalizedText(existingName) == target) {
      return catalogIdentityRow(QString::fromStdString(property.id),
                                existingName);
    }
  }
  return {};
}

QVariantMap WorkspaceFacade::contractIdentityBySignature(
    const QString &name, const QString &type, const QStringList &actorIds,
    const QStringList &propertyIds) const {
  if (!workspaceReader_) {
    return {};
  }
  const QString normalizedName = strings::normalizedText(name);
  const QString normalizedType = strings::normalizedText(type);
  if (normalizedName.isEmpty() || normalizedType.isEmpty()) {
    return {};
  }
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  for (const auto &contract : snapshot.contracts) {
    if (strings::normalizedText(QString::fromStdString(contract.name)) !=
        normalizedName) {
      continue;
    }
    if (strings::normalizedText(QString::fromStdString(contract.type)) !=
        normalizedType) {
      continue;
    }
    if (!sameStringSet(contract.actorIds, actorIds) ||
        !sameStringSet(contract.propertyIds, propertyIds)) {
      continue;
    }

    QVariantMap row = catalogIdentityRow(QString::fromStdString(contract.id),
                                         QString::fromStdString(contract.name));
    row.insert(payload::keys::common::kType,
               QString::fromStdString(contract.type));
    row.insert(payload::keys::contract::kActorIds,
               sortedStdStrings(contract.actorIds));
    row.insert(payload::keys::contract::kPropertyIds,
               sortedStdStrings(contract.propertyIds));
    row.insert(payload::keys::contract::kAllocatableMode,
               QString::fromStdString(contract.allocatableMode));
    return row;
  }
  return {};
}

QString WorkspaceFacade::nextContractName() const {
  if (!workspaceReader_) {
    return QStringLiteral("%1 1").arg(contractBaseName());
  }
  int maxIndex = 0;
  const auto snapshot = workspaceReader_->workspaceSnapshot();
  for (const auto &contract : snapshot.contracts) {
    const int index =
        trailingContractIndex(QString::fromStdString(contract.name));
    if (index > maxIndex) {
      maxIndex = index;
    }
  }
  return QStringLiteral("%1 %2").arg(contractBaseName()).arg(maxIndex + 1);
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

QString WorkspaceFacade::saveActor(const QString &id, const QString &name,
                                   const QStringList &aliases,
                                   const QStringList &contractIds) {
  if (!workspaceWriter_) {
    return {};
  }
  if (id.isEmpty()) {
    return QString::fromStdString(workspaceWriter_->addActor(
        makeActorCommand({}, name, aliases, contractIds)));
  }
  workspaceWriter_->updateActor(
      makeActorCommand(id, name, aliases, contractIds));
  return id;
}

void WorkspaceFacade::deleteActor(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteActor(strings::toStdString(id));
  }
}

QString WorkspaceFacade::saveProperty(const QString &id, const QString &name,
                                      const QStringList &aliases,
                                      const QStringList &contractIds) {
  if (!workspaceWriter_) {
    return {};
  }
  if (id.isEmpty()) {
    return QString::fromStdString(workspaceWriter_->addProperty(
        makePropertyCommand({}, name, aliases, contractIds)));
  }
  workspaceWriter_->updateProperty(
      makePropertyCommand(id, name, aliases, contractIds));
  return id;
}

void WorkspaceFacade::deleteProperty(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteProperty(strings::toStdString(id));
  }
}

QString WorkspaceFacade::saveContract(const QString &id, const QString &name,
                                      const QString &type,
                                      const QStringList &actorIds,
                                      const QStringList &propertyIds,
                                      const QStringList &aliases,
                                      const QString &allocatableMode) {
  if (!workspaceWriter_) {
    return {};
  }
  if (id.isEmpty()) {
    return QString::fromStdString(
        workspaceWriter_->addContract(makeContractCommand(
            {}, name, type, allocatableMode, actorIds, propertyIds, aliases)));
  }
  workspaceWriter_->updateContract(makeContractCommand(
      id, name, type, allocatableMode, actorIds, propertyIds, aliases));
  return id;
}

void WorkspaceFacade::deleteContract(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteContract(strings::toStdString(id));
  }
}

QString WorkspaceFacade::addStatement(const QString &name) {
  if (!workspaceWriter_) {
    return {};
  }
  core::ports::workspace::StatementCommand command;
  command.name = strings::toStdString(name);
  return QString::fromStdString(workspaceWriter_->addStatement(command));
}

void WorkspaceFacade::updateStatement(const QString &id, const QString &name) {
  if (!workspaceWriter_) {
    return;
  }
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  workspaceWriter_->updateStatement(command);
}

void WorkspaceFacade::deleteStatement(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteStatement(strings::toStdString(id));
  }
}

QString WorkspaceFacade::addTransaction(
    const QString &name, const QString &bookingDate, const QString &valuta,
    double amount, const QString &statementId, int status,
    const QString &actorId, const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    return {};
  }
  return QString::fromStdString(workspaceWriter_->addTransaction(
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             {}, status, actorId, contractId, allocatable,
                             propertyIds)));
}

QString WorkspaceFacade::insertTransactionAfter(
    const QString &afterTransactionId, const QString &name,
    const QString &bookingDate, const QString &valuta, double amount,
    const QString &statementId, int status, const QString &actorId,
    const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    return {};
  }
  return QString::fromStdString(workspaceWriter_->addTransaction(
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             afterTransactionId, status, actorId, contractId,
                             allocatable, propertyIds)));
}

void WorkspaceFacade::updateTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    const QString &valuta, double amount, const QString &statementId,
    int status, const QString &actorId, const QString &contractId,
    bool allocatable, const QStringList &propertyIds) {
  if (!workspaceWriter_) {
    return;
  }
  workspaceWriter_->updateTransaction(makeTransactionCommand(
      id, name, bookingDate, valuta, amount, statementId, {}, status, actorId,
      contractId, allocatable, propertyIds));
}

void WorkspaceFacade::deleteTransaction(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteTransaction(strings::toStdString(id));
  }
}

} // namespace ui
