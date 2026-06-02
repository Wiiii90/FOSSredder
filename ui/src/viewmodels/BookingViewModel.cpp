/**
 * @file ui/src/viewmodels/BookingViewModel.cpp
 * @brief Implements the QML-facing BookingViewModel API.
 */

#include "ui/viewmodels/BookingViewModel.h"

#include <algorithm>
#include <cmath>

#include <QJsonDocument>
#include <QMetaType>

#include "ui/shared/payload/PayloadMapper.h"
#include "ui/workspace/RowSelectionSupport.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

QString stringValue(const QVariant &value) {
  const QVariantMap map = value.toMap();
  if (!map.isEmpty()) {
    const QString aliasValue = map.value(QStringLiteral("value")).toString();
    if (!aliasValue.isEmpty()) {
      return aliasValue;
    }
    const QString aliasSource = map.value(QStringLiteral("source")).toString();
    if (!aliasSource.isEmpty()) {
      return aliasSource;
    }
    const QString id = map.value(QStringLiteral("id")).toString();
    if (!id.isEmpty()) {
      return id;
    }
  }
  return value.toString();
}

QVariantList normalizeStringValues(const QVariantList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(stringValue(value));
  }
  return out;
}

bool containsTrimmed(const QVariantList &values, const QString &id) {
  const QString nextId = id.trimmed();
  if (nextId.isEmpty()) {
    return false;
  }
  for (const auto &value : values) {
    if (stringValue(value).trimmed() == nextId) {
      return true;
    }
  }
  return false;
}

QVariantList addUniqueTrimmed(const QVariantList &values,
                              const QString &value) {
  const QString next = value.trimmed();
  QVariantList out = normalizeStringValues(values);
  if (next.isEmpty() || containsTrimmed(out, next)) {
    return out;
  }
  out.push_back(next);
  return out;
}

QVariantList removeTrimmed(const QVariantList &values, const QString &value) {
  const QString target = value.trimmed();
  if (target.isEmpty()) {
    return values;
  }
  QVariantList out;
  out.reserve(values.size());
  for (const auto &entry : values) {
    const QString current = stringValue(entry).trimmed();
    if (current != target) {
      out.push_back(current);
    }
  }
  return out;
}

bool setSelectedId(QVariantList &selectedIds, const QString &id,
                   bool selected) {
  const QString normalizedId = id.trimmed();
  if (normalizedId.isEmpty()) {
    return false;
  }
  const QVariantList next = selected ? addUniqueTrimmed(selectedIds, normalizedId)
                                     : removeTrimmed(selectedIds, normalizedId);
  if (next == selectedIds) {
    return false;
  }
  selectedIds = next;
  return true;
}

QVariantMap mapWith(const QVariantMap &base, const QString &key,
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
  const QVariantList actorIds =
      normalizeStringValues(contractRow.value(QStringLiteral("actorIds")).toList());
  for (const auto &actorValue : actorIds) {
    if (actorValue.toString() == targetActor) {
      return true;
    }
  }
  return false;
}

bool contractSupportsProperties(const QVariantMap &contractRow,
                                const QVariantList &propertyIds) {
  if (contractRow.isEmpty()) {
    return true;
  }
  const QVariantList normalizedPropertyIds = normalizeStringValues(propertyIds);
  const QVariantList allowedPropertyIds = normalizeStringValues(
      contractRow.value(QStringLiteral("propertyIds")).toList());
  for (const auto &propertyValue : normalizedPropertyIds) {
    if (!allowedPropertyIds.contains(propertyValue)) {
      return false;
    }
  }
  return true;
}

QVariantMap emptyTransactionDraft() {
  QVariantMap tx;
  tx.insert(QStringLiteral("id"), QString());
  tx.insert(QStringLiteral("name"), QString());
  tx.insert(QStringLiteral("bookingDate"), QString());
  tx.insert(QStringLiteral("valuta"), QString());
  tx.insert(QStringLiteral("amount"), 0.0);
  tx.insert(QStringLiteral("description"), QString());
  tx.insert(QStringLiteral("status"), 0);
  tx.insert(QStringLiteral("actorId"), QString());
  tx.insert(QStringLiteral("propertyIds"), QVariantList());
  tx.insert(QStringLiteral("allocatable"), false);
  tx.insert(QStringLiteral("contractId"), QString());
  tx.insert(QStringLiteral("statementId"), QString());
  return tx;
}

QVariantMap normalizeTransactionDraft(const QVariantMap &tx) {
  QVariantMap out = emptyTransactionDraft();
  for (auto it = tx.constBegin(); it != tx.constEnd(); ++it) {
    out.insert(it.key(), it.value());
  }
  out.insert(QStringLiteral("status"),
             out.value(QStringLiteral("status")).toInt());
  out.insert(QStringLiteral("propertyIds"),
             normalizeStringValues(
                 out.value(QStringLiteral("propertyIds")).toList()));
  out.insert(QStringLiteral("actorId"),
             out.value(QStringLiteral("actorId")).toString());
  out.insert(QStringLiteral("contractId"),
             out.value(QStringLiteral("contractId")).toString());
  out.insert(QStringLiteral("name"),
             out.value(QStringLiteral("name")).toString());
  out.insert(QStringLiteral("bookingDate"),
             out.value(QStringLiteral("bookingDate")).toString());
  out.insert(QStringLiteral("valuta"),
             out.value(QStringLiteral("valuta")).toString());
  out.insert(QStringLiteral("description"),
             out.value(QStringLiteral("description")).toString());
  out.insert(QStringLiteral("statementId"),
             out.value(QStringLiteral("statementId")).toString());
  out.insert(QStringLiteral("allocatable"),
             out.value(QStringLiteral("allocatable")).toBool());
  return out;
}

QVariantList normalizeTransactionDrafts(const QVariantList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(normalizeTransactionDraft(value.toMap()));
  }
  return out;
}

bool transactionDraftHasContent(const QVariantMap &tx) {
  const QVariantMap normalized = normalizeTransactionDraft(tx);
  const QVariant amount = normalized.value(QStringLiteral("amount"));
  const bool hasAmount = amount.userType() == QMetaType::Double
                             ? amount.toDouble() != 0.0
                             : !amount.toString().trimmed().isEmpty();
  return !normalized.value(QStringLiteral("name")).toString().isEmpty() ||
         !normalized.value(QStringLiteral("bookingDate"))
              .toString()
              .isEmpty() ||
         !normalized.value(QStringLiteral("valuta")).toString().isEmpty() ||
         hasAmount ||
         !normalized.value(QStringLiteral("actorId")).toString().isEmpty() ||
         !normalized.value(QStringLiteral("contractId")).toString().isEmpty() ||
         !normalized.value(QStringLiteral("propertyIds")).toList().isEmpty() ||
         normalized.value(QStringLiteral("allocatable")).toBool() ||
         normalized.value(QStringLiteral("status")).toInt() != 0;
}

QVariantMap transactionDraft(const QVariantMap &draft,
                             const QVariantList &contractRows,
                             const QVariantMap &changes) {
  QVariantMap out = draft;
  if (changes.contains(QStringLiteral("contractId"))) {
    const QString normalizedContractId =
        changes.value(QStringLiteral("contractId")).toString().trimmed();
    const QVariantMap contractRow = rowById(contractRows, normalizedContractId);
    const QVariantList actorIds =
        normalizeStringValues(contractRow.value(QStringLiteral("actorIds")).toList());
    const QVariantList propertyIds = normalizeStringValues(
        contractRow.value(QStringLiteral("propertyIds")).toList());
    const QString actorId =
        actorIds.isEmpty() ? QString() : actorIds.first().toString();
    out = mapWith(out, QStringLiteral("contractId"), normalizedContractId);
    out = mapWith(out, QStringLiteral("actorId"), actorId);
    out = mapWith(out, QStringLiteral("propertyIds"), propertyIds);
  }
  if (changes.contains(QStringLiteral("actorId"))) {
    const QString normalizedActorId =
        changes.value(QStringLiteral("actorId")).toString().trimmed();
    const QString currentContractId =
        out.value(QStringLiteral("contractId")).toString().trimmed();
    const QVariantMap currentContract = rowById(contractRows, currentContractId);
    const QString nextContractId =
        contractSupportsActor(currentContract, normalizedActorId)
            ? currentContractId
            : QString();
    out = mapWith(out, QStringLiteral("actorId"), normalizedActorId);
    out = mapWith(out, QStringLiteral("contractId"), nextContractId);
  }
  if (changes.contains(QStringLiteral("propertyIds"))) {
    const QVariantList normalizedPropertyIds = normalizeStringValues(
        changes.value(QStringLiteral("propertyIds")).toList());
    const QString currentContractId =
        out.value(QStringLiteral("contractId")).toString().trimmed();
    const QVariantMap currentContract = rowById(contractRows, currentContractId);
    const QString nextContractId =
        contractSupportsProperties(currentContract, normalizedPropertyIds)
            ? currentContractId
            : QString();
    out = mapWith(out, QStringLiteral("propertyIds"), normalizedPropertyIds);
    out = mapWith(out, QStringLiteral("contractId"), nextContractId);
  }
  return out;
}

QVariantMap createDraftListState(const QVariantList &drafts, int currentIndex,
                                 const QVariantMap &emptyDraft) {
  QVariantList normalizedDrafts = normalizeTransactionDrafts(drafts);
  if (normalizedDrafts.isEmpty()) {
    normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
  }
  const int lastIndex = static_cast<int>(normalizedDrafts.size()) - 1;
  const int index = std::clamp(currentIndex, 0, lastIndex);
  return {{QStringLiteral("drafts"), normalizedDrafts},
          {QStringLiteral("index"), index}};
}

QVariantMap insertDraftAfterCurrent(const QVariantList &drafts,
                                    int currentIndex,
                                    const QVariantMap &emptyDraft) {
  const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
  QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
  const int index = base.value(QStringLiteral("index")).toInt();
  const int insertIndex =
      std::clamp(index + 1, 0, static_cast<int>(normalizedDrafts.size()));
  normalizedDrafts.insert(insertIndex, normalizeTransactionDraft(emptyDraft));
  return {{QStringLiteral("drafts"), normalizedDrafts},
          {QStringLiteral("index"), insertIndex}};
}

QVariantMap removeDraftAt(const QVariantList &drafts, int currentIndex,
                          const QVariantMap &emptyDraft) {
  const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
  QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
  int index = base.value(QStringLiteral("index")).toInt();
  if (index < 0 || index >= normalizedDrafts.size()) {
    index = 0;
  }
  normalizedDrafts.removeAt(index);
  if (normalizedDrafts.isEmpty()) {
    normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
    index = 0;
  } else {
    index = std::clamp(index, 0,
                       static_cast<int>(normalizedDrafts.size()) - 1);
  }
  return {{QStringLiteral("drafts"), normalizedDrafts},
          {QStringLiteral("index"), index}};
}

QVariantMap currentDraftState(const QVariantList &drafts, int currentIndex,
                              const QVariantMap &emptyDraft) {
  const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
  const QVariantList normalizedDrafts =
      base.value(QStringLiteral("drafts")).toList();
  const int index = base.value(QStringLiteral("index")).toInt();
  return {{QStringLiteral("drafts"), normalizedDrafts},
          {QStringLiteral("index"), index},
          {QStringLiteral("draft"),
           normalizeTransactionDraft(normalizedDrafts.at(index).toMap())}};
}

QVariantMap setCurrentRawDraft(const QVariantList &drafts, int currentIndex,
                               const QVariantMap &draft,
                               const QVariantMap &emptyDraft) {
  QVariantMap state = currentDraftState(drafts, currentIndex, emptyDraft);
  QVariantList nextDrafts = state.value(QStringLiteral("drafts")).toList();
  const int index = state.value(QStringLiteral("index")).toInt();
  if (index >= 0 && index < nextDrafts.size()) {
    nextDrafts[index] = draft;
  }
  state.insert(QStringLiteral("drafts"), nextDrafts);
  state.insert(QStringLiteral("draft"), draft);
  return state;
}

QString transactionDraftSnapshot(const QVariantMap &tx) {
  return QString::fromUtf8(QJsonDocument::fromVariant(normalizeTransactionDraft(tx))
                               .toJson(QJsonDocument::Compact));
}

bool bookingEditStateChanged(const QString &savedStatementName,
                             const QString &savedTransactionJson,
                             const QString &currentStatementName,
                             const QVariantMap &transaction) {
  return savedStatementName != currentStatementName ||
         savedTransactionJson != transactionDraftSnapshot(transaction);
}

} // namespace
BookingViewModel::BookingViewModel(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  createTransactions_ = QVariantList{emptyTransaction()};
  currentTransactionDraft_ = emptyTransaction();
  bindSignals();
  if (isCreateMode()) {
    resetCreateState();
  } else {
    syncEditState();
  }
}

bool BookingViewModel::isCreateMode() const {
  return selectedStatementId().isEmpty();
}

QString BookingViewModel::statementName() const {
  return isCreateMode() ? createStatementName_ : editStatementName_;
}

void BookingViewModel::setStatementName(const QString &value) {
  if (isCreateMode()) {
    if (createStatementName_ == value) {
      return;
    }
    createStatementName_ = value;
  } else {
    if (editStatementName_ == value) {
      return;
    }
    editStatementName_ = value;
  }
  emit changed();
}

void BookingViewModel::bindSignals() {
  if (!workspace_) {
    return;
  }
  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() {
                     if (isCreateMode()) {
                       resetCreateState();
                     } else {
                       syncEditState();
                     }
                   });
  QObject::connect(workspace_, &WorkspaceFacade::selectedStatementIdChanged, this,
                   [this]() {
                     if (isCreateMode()) {
                       resetCreateState();
                     } else {
                       editTransactionOrderIds_.clear();
                       editTransactionIndex_ = 0;
                       syncEditState();
                     }
                   });
  QObject::connect(workspace_, &WorkspaceFacade::selectedTransactionIdChanged, this,
                   [this]() {
                     if (isCreateMode()) {
                       resetCreateState();
                     } else {
                       rememberSelectedTransaction();
                       syncEditState();
                     }
                   });
}

QVariantList BookingViewModel::statementRows() const {
  return workspace_ ? workspace_->statementRowsWithTransactions()
                    : QVariantList();
}

QString BookingViewModel::selectedStatementId() const {
  return workspace_ ? workspace_->selectedStatementId() : QString();
}

QString BookingViewModel::selectedTransactionId() const {
  return workspace_ ? workspace_->selectedTransactionId() : QString();
}

bool BookingViewModel::hasStatements() const { return !statementRows().isEmpty(); }

QVariantList
BookingViewModel::statementTransactionRows(const QString &statementId) const {
  return workspace_ ? workspace_->statementTransactionRows(statementId)
                    : QVariantList();
}

void BookingViewModel::selectStatement(const QString &statementId) {
  if (!workspace_) {
    return;
  }
  workspace_->selectTransaction(statementId, QString());
}

void BookingViewModel::selectTransaction(const QString &statementId,
                                     const QString &transactionId) {
  if (!workspace_) {
    return;
  }
  workspace_->selectTransaction(statementId, transactionId);
}

QString BookingViewModel::transactionInfoText() const {
  if (isCreateMode()) {
    return createTransactions_.isEmpty() ? tr("No transactions")
                                         : tr("Transaction %1 / %2")
                                               .arg(createTransactionIndex_ + 1)
                                               .arg(createTransactions_.size());
  }

  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  if (rows.isEmpty()) {
    return tr("No transactions");
  }
  const int index = state.value(QStringLiteral("index")).toInt();
  return tr("Transaction %1 / %2").arg(index + 1).arg(rows.size());
}

bool BookingViewModel::hasMultipleTransactions() const {
  return isCreateMode() ? createTransactions_.size() > 1
                        : editTransactionState()
                                  .value(QStringLiteral("rows"))
                                  .toList()
                                  .size() > 1;
}

bool BookingViewModel::canAddTransaction() const {
  return isCreateMode() || !selectedStatementId().isEmpty();
}

bool BookingViewModel::canDeleteTransaction() const {
  if (isCreateMode()) {
    return createTransactions_.size() > 1;
  }
  return hasMultipleTransactions() &&
         !editTransactionData_.value(QStringLiteral("id")).toString().isEmpty();
}

QVariantMap BookingViewModel::emptyTransaction() const {
  return emptyTransactionDraft();
}

QVariantMap BookingViewModel::normalizeTransaction(const QVariantMap &tx) const {
  return normalizeTransactionDraft(tx);
}

QVariantMap BookingViewModel::editTransactionState() const {
  if (!workspace_ || selectedStatementId().isEmpty()) {
    return {{QStringLiteral("rows"), QVariantList()},
            {QStringLiteral("orderIds"), QVariantList()},
            {QStringLiteral("index"), -1},
            {QStringLiteral("id"), QString()}};
  }
  const QVariantList rows = statementTransactionRows(selectedStatementId());
  const QVariantList preferred = editTransactionOrderIds_.isEmpty()
                                     ? rowIds(rows)
                                     : editTransactionOrderIds_;
  return orderedSelectionState(rows, preferred, editTransactionIndex_,
                               selectedTransactionId(), QStringLiteral("id"));
}

QVariantMap BookingViewModel::transactionById(const QString &txId) const {
  if (!workspace_ || txId.isEmpty()) {
    return emptyTransaction();
  }
  QVariantMap transaction = workspace_->transactionRowById(txId);
  if (transaction.isEmpty()) {
    return emptyTransaction();
  }
  transaction.insert(
      payload::keys::transaction::kPropertyIds,
      payload::mapper::toVariantList(
          transaction.value(payload::keys::transaction::kPropertyIds)));
  return normalizeTransaction(transaction);
}

QString BookingViewModel::transactionSnapshot(const QVariantMap &data) const {
  return transactionDraftSnapshot(data);
}

QVariantMap BookingViewModel::transactionData() const {
  return isCreateMode() ? currentTransactionDraft_ : editTransactionData_;
}

void BookingViewModel::setTransactionData(const QVariantMap &value) {
  if (isCreateMode()) {
    if (!workspace_) {
      return;
    }
    const QVariantMap draft = value.isEmpty() ? emptyTransaction() : value;
    const QVariantMap state = setCurrentRawDraft(
        createTransactions_, createTransactionIndex_, draft,
        emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
  } else {
    if (editTransactionData_ == value) {
      return;
    }
    editTransactionData_ = value.isEmpty() ? emptyTransaction() : value;
  }
  emit changed();
}

QVariant BookingViewModel::value(const QString &key) const {
  return transactionData().value(key);
}

void BookingViewModel::setTransactionValue(const QString &key,
                                       const QVariant &value) {
  QVariantMap next = transactionData();
  next.insert(key, value);
  setTransactionData(next);
}

void BookingViewModel::applyTransactionDraftChange(const QVariantMap &changes) {
  if (!workspace_) {
    return;
  }
  setTransactionData(transactionDraft(
      transactionData(), contractRows(), changes));
}

QString BookingViewModel::transactionName() const {
  return value(QStringLiteral("name")).toString();
}

void BookingViewModel::setTransactionName(const QString &value) {
  setTransactionValue(QStringLiteral("name"), value);
}

QString BookingViewModel::transactionBookingDate() const {
  return value(QStringLiteral("bookingDate")).toString();
}

void BookingViewModel::setTransactionBookingDate(const QString &value) {
  setTransactionValue(QStringLiteral("bookingDate"), value);
}

QString BookingViewModel::transactionValuta() const {
  return value(QStringLiteral("valuta")).toString();
}

void BookingViewModel::setTransactionValuta(const QString &value) {
  setTransactionValue(QStringLiteral("valuta"), value);
}

QString BookingViewModel::transactionAmountText() const {
  const QVariant amount = value(QStringLiteral("amount"));
  return amount.isValid() ? amount.toString() : QString();
}

void BookingViewModel::setTransactionAmountText(const QString &value) {
  setTransactionValue(QStringLiteral("amount"), value);
}

bool BookingViewModel::transactionAllocatable() const {
  return value(QStringLiteral("allocatable")).toBool();
}

void BookingViewModel::setTransactionAllocatable(bool value) {
  setTransactionValue(QStringLiteral("allocatable"), value);
}

void BookingViewModel::captureEditState() {
  savedEditStatementName_ = editStatementName_;
  savedEditTransactionJson_ = transactionSnapshot(editTransactionData_);
}

void BookingViewModel::currentCreateTransaction() {
  if (!workspace_) {
    createTransactions_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionDraft_ = emptyTransaction();
    return;
  }
  const QVariantMap state = currentDraftState(
      createTransactions_, createTransactionIndex_, emptyTransaction());
  createTransactions_ = state.value(QStringLiteral("drafts")).toList();
  createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
  currentTransactionDraft_ = state.value(QStringLiteral("draft")).toMap();
}

void BookingViewModel::resetCreateState() {
  createStatementName_.clear();
  if (workspace_) {
    const QVariantMap state = createDraftListState(
        QVariantList(), 0, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentTransactionDraft_ = state.value(QStringLiteral("draft")).toMap();
  } else {
    createTransactions_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionDraft_ = emptyTransaction();
  }
  emit changed();
}

void BookingViewModel::addTransactionAfterCurrent() {
  if (!workspace_) {
    return;
  }
  if (isCreateMode()) {
    const QVariantMap state = insertDraftAfterCurrent(
        createTransactions_, createTransactionIndex_, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
    emit changed();
    return;
  }

  if (selectedStatementId().isEmpty()) {
    return;
  }
  const QVariantMap state = editTransactionState();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  int insertAfterIndex =
      state.value(QStringLiteral("index"), editTransactionIndex_).toInt();
  if (insertAfterIndex < 0 || insertAfterIndex >= rows.size()) {
    insertAfterIndex = rows.size() - 1;
  }
  const QVariantMap current = normalizeTransaction(editTransactionData_);
  const QString newId = workspace_->insertTransactionAfter(
      current.value(QStringLiteral("id")).toString(), QString(),
      current.value(QStringLiteral("bookingDate")).toString(),
      current.value(QStringLiteral("valuta")).toString(), 0.0,
      selectedStatementId(), 0, QString(), QString(), false, QStringList());
  if (newId.isEmpty()) {
    return;
  }
  const QVariantList updatedRows =
      statementTransactionRows(selectedStatementId());
  const QVariantList updatedIds = rowIds(updatedRows);
  editTransactionOrderIds_ = orderWithInsertedId(
      editTransactionOrderIds_, updatedIds, newId, insertAfterIndex);
  workspace_->selectTransaction(selectedStatementId(), newId);
  syncEditState();
}

void BookingViewModel::deleteCurrentTransaction() {
  if (!workspace_) {
    return;
  }
  if (isCreateMode()) {
    const QVariantMap state = removeDraftAt(
        createTransactions_, createTransactionIndex_, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QString deletedId =
      editTransactionData_.value(QStringLiteral("id")).toString();
  if (deletedId.isEmpty() || !hasMultipleTransactions()) {
    return;
  }
  const QVariantMap selectionState = editTransactionState();
  editTransactionOrderIds_ =
      selectionState.value(QStringLiteral("orderIds")).toList();
  workspace_->deleteTransaction(deletedId);
  const QVariantMap reselectionState = deleteReselectionState(
      statementTransactionRows(selectedStatementId()), editTransactionOrderIds_,
      editTransactionIndex_, deletedId, QStringLiteral("id"));
  editTransactionOrderIds_ =
      reselectionState.value(QStringLiteral("orderIds")).toList();
  editTransactionIndex_ =
      reselectionState.value(QStringLiteral("index")).toInt();
  const QString txId = reselectionState.value(QStringLiteral("id")).toString();
  workspace_->selectTransaction(selectedStatementId(), txId);
  rememberSelectedTransaction();
  editTransactionData_ = transactionById(txId);
  emit changed();
}

void BookingViewModel::rememberSelectedTransaction() {
  if (!workspace_ || selectedStatementId().isEmpty() ||
      selectedTransactionId().isEmpty()) {
    return;
  }
  lastTransactionIdByStatementId_.insert(selectedStatementId(),
                                         selectedTransactionId());
}

QString
BookingViewModel::transactionIdForStatement(const QString &statementId) const {
  return workspace_
             ? rememberedOrFirstRowId(statementTransactionRows(statementId),
                                      lastTransactionIdByStatementId_,
                                      statementId, QStringLiteral("id"))
             : QString();
}

bool BookingViewModel::ensureSelectedTransactionForStatement() {
  if (isCreateMode() || !workspace_ || selectedStatementId().isEmpty()) {
    return true;
  }
  const QString statementId = selectedStatementId();
  const QString currentTxId = selectedTransactionId();
  if (rowHasId(statementTransactionRows(statementId), currentTxId,
               QStringLiteral("id"))) {
    return true;
  }
  const QString nextTxId = transactionIdForStatement(statementId);
  if (currentTxId == nextTxId) {
    return true;
  }
  workspace_->selectTransaction(statementId, nextTxId);
  return false;
}

void BookingViewModel::syncEditState() {
  if (isCreateMode()) {
    return;
  }
  if (!ensureSelectedTransactionForStatement()) {
    return;
  }
  editStatementName_ = rowById(statementRows(), selectedStatementId())
                           .value(QStringLiteral("name"))
                           .toString();
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    editTransactionIndex_ = -1;
    editTransactionData_ = emptyTransaction();
    emit changed();
    return;
  }
  editTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
  const QString txId = state.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selectTransaction(selectedStatementId(), txId);
  rememberSelectedTransaction();
  captureEditState();
  emit changed();
}

void BookingViewModel::previousStatement() {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = statementRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString currentId = isCreateMode() ? QString() : selectedStatementId();
  const int currentIndex = indexOfId(rows, currentId);
  if (currentIndex == 0) {
    rememberSelectedTransaction();
    workspace_->selectTransaction(QString(), QString());
    editTransactionOrderIds_.clear();
    editTransactionIndex_ = -1;
    emit changed();
    return;
  }
  const int nextIndex = currentIndex < 0 ? rows.size() - 1 : currentIndex - 1;
  const QString nextId =
      rows.value(nextIndex).toMap().value(QStringLiteral("id")).toString();
  rememberSelectedTransaction();
  workspace_->selectTransaction(nextId, transactionIdForStatement(nextId));
  editTransactionOrderIds_.clear();
  editTransactionIndex_ = 0;
  emit changed();
}

void BookingViewModel::nextStatement() {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = statementRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString currentId = isCreateMode() ? QString() : selectedStatementId();
  const int currentIndex = indexOfId(rows, currentId);
  if (currentIndex == rows.size() - 1) {
    rememberSelectedTransaction();
    workspace_->selectTransaction(QString(), QString());
    editTransactionOrderIds_.clear();
    editTransactionIndex_ = -1;
    emit changed();
    return;
  }
  const int nextIndex = currentIndex < 0 ? 0 : currentIndex + 1;
  const QString nextId =
      rows.value(nextIndex).toMap().value(QStringLiteral("id")).toString();
  rememberSelectedTransaction();
  workspace_->selectTransaction(nextId, transactionIdForStatement(nextId));
  editTransactionOrderIds_.clear();
  editTransactionIndex_ = 0;
  emit changed();
}

void BookingViewModel::previousTransaction() {
  if (!workspace_) {
    return;
  }
  if (isCreateMode()) {
    if (createTransactions_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        wrappedIndex(createTransactionIndex_ - 1, createTransactions_.size());
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    return;
  }
  const QVariantMap next =
      navigateSelectionState(rows, state.value(QStringLiteral("index")).toInt(),
                             state.value(QStringLiteral("id")).toString(), -1,
                             0, QStringLiteral("id"));
  editTransactionIndex_ = next.value(QStringLiteral("index")).toInt();
  const QString txId = next.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selectTransaction(selectedStatementId(), txId);
  rememberSelectedTransaction();
  emit changed();
}

void BookingViewModel::nextTransaction() {
  if (!workspace_) {
    return;
  }
  if (isCreateMode()) {
    if (createTransactions_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        wrappedIndex(createTransactionIndex_ + 1, createTransactions_.size());
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    return;
  }
  const QVariantMap next = navigateSelectionState(
      rows, state.value(QStringLiteral("index")).toInt(),
      state.value(QStringLiteral("id")).toString(), 1, 0, QStringLiteral("id"));
  editTransactionIndex_ = next.value(QStringLiteral("index")).toInt();
  const QString txId = next.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selectTransaction(selectedStatementId(), txId);
  rememberSelectedTransaction();
  emit changed();
}

bool BookingViewModel::transactionDraftCanSubmit(const QVariantMap &draft) const {
  if (!workspace_) {
    return false;
  }

  const QVariantMap tx = normalizeTransaction(draft);
  if (!transactionDraftHasContent(tx)) {
    return true;
  }

  const QString bookingDate =
      tx.value(QStringLiteral("bookingDate")).toString().trimmed();
  if (bookingDate.isEmpty()) {
    return false;
  }

  const double amount = workspace_->amountForTransactionCommit(
      draft.value(QStringLiteral("amount")), QString(),
      tx.value(QStringLiteral("amount")).toDouble());
  return std::isfinite(amount);
}

QVariantList BookingViewModel::submittableTransactionDrafts() const {
  QVariantList out;
  if (!workspace_) {
    return out;
  }

  for (const QVariant &draftValue : createTransactions_) {
    const QVariantMap draft = draftValue.toMap();
    const QVariantMap normalized = normalizeTransaction(draft);
    if (!transactionDraftHasContent(normalized)) {
      continue;
    }
    if (!transactionDraftCanSubmit(draft)) {
      continue;
    }
    out.push_back(draft);
  }
  return out;
}

bool BookingViewModel::canCreate() const {
  if (createStatementName_.trimmed().isEmpty()) {
    return false;
  }
  for (const QVariant &draftValue : createTransactions_) {
    if (!transactionDraftCanSubmit(draftValue.toMap())) {
      return false;
    }
  }
  return true;
}

bool BookingViewModel::canUpdate() const {
  if (isCreateMode() || !workspace_) {
    return false;
  }
  return bookingEditStateChanged(
      savedEditStatementName_, savedEditTransactionJson_, editStatementName_,
      editTransactionData_);
}

QString BookingViewModel::submit() {
  if (!workspace_ || !canCreate()) {
    return QString();
  }
  const QString statementName = createStatementName_;
  const QVariantList transactionDrafts = submittableTransactionDrafts();
  const QString statementId = workspace_->addStatement(statementName);
  if (statementId.isEmpty()) {
    return QString();
  }
  for (const QVariant &draftValue : transactionDrafts) {
    const QVariantMap rawTx = draftValue.toMap();
    const QVariantMap tx = normalizeTransaction(rawTx);
    workspace_->addTransaction(
        tx.value(QStringLiteral("name")).toString(),
        tx.value(QStringLiteral("bookingDate")).toString(),
        tx.value(QStringLiteral("valuta")).toString(),
        workspace_->amountForTransactionCommit(
            rawTx.value(QStringLiteral("amount")), QString(),
            tx.value(QStringLiteral("amount")).toDouble()),
        statementId, tx.value(payload::keys::common::kStatus).toInt(),
        tx.value(payload::keys::transaction::kActorId).toString(),
        tx.value(payload::keys::transaction::kContractId).toString(),
        tx.value(QStringLiteral("allocatable")).toBool(),
        payload::mapper::toQStringList(
            tx.value(payload::keys::transaction::kPropertyIds).toList()));
  }
  resetCreateState();
  workspace_->selectTransaction(statementId, transactionIdForStatement(statementId));
  return statementId;
}

void BookingViewModel::updateCurrent() {
  if (isCreateMode() || !workspace_ || selectedStatementId().isEmpty()) {
    return;
  }
  const QString statementId = selectedStatementId();
  const bool statementChanged = savedEditStatementName_ != editStatementName_;
  const QString statementName = editStatementName_;
  const QVariantMap transactionData = editTransactionData_;
  const QString txId = transactionData.value(QStringLiteral("id")).toString();
  const bool transactionChanged =
      savedEditTransactionJson_ != transactionSnapshot(transactionData);
  const QVariantMap normalizedTx = normalizeTransaction(transactionData);
  if (transactionChanged && !txId.isEmpty()) {
    workspace_->updateTransaction(
        txId, transactionData.value(QStringLiteral("name")).toString(),
        transactionData.value(QStringLiteral("bookingDate")).toString(),
        transactionData.value(QStringLiteral("valuta")).toString(),
        workspace_->amountForTransactionCommit(
            transactionData.value(QStringLiteral("amount")), txId,
            normalizedTx.value(QStringLiteral("amount")).toDouble()),
        statementId, normalizedTx.value(payload::keys::common::kStatus).toInt(),
        transactionData.value(payload::keys::transaction::kActorId).toString(),
        transactionData.value(payload::keys::transaction::kContractId)
            .toString(),
        transactionData.value(QStringLiteral("allocatable")).toBool(),
        payload::mapper::toQStringList(
            transactionData.value(payload::keys::transaction::kPropertyIds)
                .toList()));
  }
  if (statementChanged) {
    workspace_->updateStatement(statementId, statementName);
  }
  syncEditState();
}

void BookingViewModel::deleteCurrentStatement() {
  if (isCreateMode() || !workspace_) {
    return;
  }
  const QString removedId = selectedStatementId();
  workspace_->deleteStatement(removedId);
  const QString nextId = deleteNextSelectionId(statementRows(), removedId, 0,
                                               QStringLiteral("id"));
  workspace_->selectTransaction(nextId,
                                nextId.isEmpty()
                                    ? QString()
                                    : transactionIdForStatement(nextId));
  emit changed();
}

int BookingViewModel::transactionStatusIndex() const {
  if (!workspace_) {
    return 0;
  }
  const QVariantMap tx = normalizeTransaction(transactionData());
  const int index =
      indexOfKeyValue(transactionStatusOptions(), payload::keys::common::kValue,
                      tx.value(payload::keys::common::kStatus,
                               payload::transaction_status::kNeutral));
  return index >= 0 ? index : 0;
}

void BookingViewModel::setTransactionStatusIndex(int index) {
  const QVariantList options = transactionStatusOptions();
  const QVariantMap option = options.value(index).toMap();
  setTransactionValue(
      payload::keys::common::kStatus,
      option.value(payload::keys::common::kValue,
                   payload::transaction_status::kNeutral));
}

QVariantList BookingViewModel::actorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList();
}

QVariantList BookingViewModel::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList();
}

QVariantList BookingViewModel::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList();
}

QVariantList BookingViewModel::selectedPropertyIds() const {
  return normalizeTransaction(transactionData())
      .value(payload::keys::transaction::kPropertyIds)
      .toList();
}

QVariantList BookingViewModel::transactionStatusOptions() const {
  return payload::transaction_status::options(
      tr("Neutral"), tr("Unverified"), tr("Verified"), tr("Completed"));
}

QVariantList BookingViewModel::actorDisplayRows() const {
  return workspace_ ? displayRowsWithEmpty(actorRows(), tr("No actor"),
                                           payload::keys::common::kDisplay)
                    : QVariantList();
}

QVariantList BookingViewModel::contractDisplayRows() const {
  return workspace_ ? displayRowsWithEmpty(contractRows(), tr("No contract"),
                                           payload::keys::common::kDisplay)
                    : QVariantList();
}

int BookingViewModel::selectedIndexFor(const QVariantList &rows,
                                   const QString &id) const {
  if (!workspace_) {
    return 0;
  }
  const int index = indexOfId(rows, id);
  return index >= 0 ? index : 0;
}

int BookingViewModel::selectedActorIndex() const {
  return selectedIndexFor(actorDisplayRows(),
                          value(payload::keys::transaction::kActorId)
                              .toString());
}

int BookingViewModel::selectedContractIndex() const {
  return selectedIndexFor(contractDisplayRows(),
                          value(payload::keys::transaction::kContractId)
                              .toString());
}

bool BookingViewModel::isPropertySelected(const QString &propertyId) const {
  return containsTrimmed(selectedPropertyIds(), propertyId);
}

void BookingViewModel::setPropertySelected(const QString &propertyId,
                                       bool selected) {
  if (!workspace_) {
    return;
  }
  QVariantList propertyIds =
      normalizeTransaction(transactionData())
          .value(payload::keys::transaction::kPropertyIds)
          .toList();
  if (!setSelectedId(propertyIds, propertyId, selected)) {
    return;
  }
  applyTransactionDraftChange(
      {{payload::keys::transaction::kPropertyIds, propertyIds}});
}

void BookingViewModel::selectActorIndex(int index) {
  const QVariantList rows = actorDisplayRows();
  const QString id =
      rows.value(index).toMap().value(payload::keys::common::kId).toString();
  applyTransactionDraftChange({{payload::keys::transaction::kActorId, id}});
}

void BookingViewModel::selectContractIndex(int index) {
  const QVariantList rows = contractDisplayRows();
  const QString id =
      rows.value(index).toMap().value(payload::keys::common::kId).toString();
  applyTransactionDraftChange({{payload::keys::transaction::kContractId, id}});
}

} // namespace ui
