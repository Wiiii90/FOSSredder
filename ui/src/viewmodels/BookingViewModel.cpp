/**
 * @file ui/src/viewmodels/BookingViewModel.cpp
 * @brief Implements the QML-facing BookingViewModel API.
 */

#include "ui/viewmodels/BookingViewModel.h"

#include <algorithm>
#include <cmath>

#include <QMetaType>

#include "ui/presentation/PayloadMapper.h"
#include "ui/observability/Trace.h"
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

QString rowIdAt(const QVariantList &rows, int index,
                const QString &idKey = QStringLiteral("id")) {
  if (index < 0 || index >= rows.size()) {
    return {};
  }
  return rows.at(index).toMap().value(idKey).toString();
}

int indexOfId(const QVariantList &rows, const QString &id,
              const QString &idKey = QStringLiteral("id")) {
  if (id.isEmpty()) {
    return -1;
  }
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(idKey).toString() == id) {
      return i;
    }
  }
  return -1;
}

int indexOfKeyValue(const QVariantList &rows, const QString &key,
                    const QVariant &value) {
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(key) == value) {
      return i;
    }
  }
  return -1;
}

QVariantMap rowById(const QVariantList &rows, const QString &id,
                    const QString &idKey = QStringLiteral("id")) {
  const int index = indexOfId(rows, id.trimmed(), idKey);
  return index >= 0 ? rows.at(index).toMap() : QVariantMap{};
}

QString deleteNextSelectionId(const QVariantList &rows, const QString &removedId,
                              int defaultIndex = 0,
                              const QString &idKey = QStringLiteral("id")) {
  if (rows.isEmpty()) {
    return {};
  }
  const int removedIndex = indexOfId(rows, removedId, idKey);
  const int nextIndex = removedIndex >= 0 ? removedIndex + 1 : defaultIndex;
  const int wrapped = nextIndex % rows.size();
  return rowIdAt(rows, wrapped < 0 ? wrapped + rows.size() : wrapped, idKey);
}

int wrappedIndex(int index, int count) {
  if (count <= 0) {
    return -1;
  }
  const int normalized = index % count;
  return normalized < 0 ? normalized + count : normalized;
}

QVariantList rowIds(const QVariantList &rows,
                    const QString &idKey = QStringLiteral("id")) {
  QVariantList out;
  out.reserve(rows.size());
  for (const auto &rowValue : rows) {
    const QString id = rowValue.toMap().value(idKey).toString();
    if (!id.isEmpty()) {
      out.push_back(id);
    }
  }
  return out;
}

bool rowHasId(const QVariantList &rows, const QString &id,
              const QString &idKey = QStringLiteral("id")) {
  return indexOfId(rows, id.trimmed(), idKey) >= 0;
}

QVariantList pruneAndAppendMissingIds(const QVariantList &preferredIds,
                                      const QVariantList &availableIds) {
  QVariantList out;
  for (const auto &id : preferredIds) {
    if (availableIds.contains(id) && !out.contains(id)) {
      out.push_back(id);
    }
  }
  for (const auto &id : availableIds) {
    if (!out.contains(id)) {
      out.push_back(id);
    }
  }
  return out;
}

QVariantList orderRowsByIds(const QVariantList &rows,
                            const QVariantList &orderIds,
                            const QString &idKey = QStringLiteral("id")) {
  QVariantList out;
  out.reserve(rows.size());
  for (const auto &orderId : orderIds) {
    for (const auto &rowValue : rows) {
      if (rowValue.toMap().value(idKey).toString() == orderId.toString() &&
          !out.contains(rowValue)) {
        out.push_back(rowValue);
        break;
      }
    }
  }
  for (const auto &rowValue : rows) {
    if (!out.contains(rowValue)) {
      out.push_back(rowValue);
    }
  }
  return out;
}

QVariantList orderWithInsertedId(const QVariantList &currentOrder,
                                 const QVariantList &availableIds,
                                 const QString &insertedId,
                                 int insertAfterIndex) {
  QVariantList out = pruneAndAppendMissingIds(currentOrder, availableIds);
  if (!insertedId.isEmpty() && !out.contains(insertedId)) {
    const int insertIndex =
        std::clamp(insertAfterIndex + 1, 0, static_cast<int>(out.size()));
    out.insert(insertIndex, insertedId);
  }
  return out;
}

QVariantMap orderedSelectionState(const QVariantList &rows,
                                  const QVariantList &preferredOrder,
                                  int currentIndex,
                                  const QString &selectedId,
                                  const QString &idKey = QStringLiteral("id")) {
  const QVariantList orderIds =
      pruneAndAppendMissingIds(preferredOrder, rowIds(rows, idKey));
  const QVariantList orderedRows = orderRowsByIds(rows, orderIds, idKey);
  if (orderedRows.isEmpty()) {
    return {{QStringLiteral("rows"), orderedRows},
            {QStringLiteral("orderIds"), orderIds},
            {QStringLiteral("index"), -1},
            {QStringLiteral("id"), QString()}};
  }
  const int selectedIndex = indexOfId(orderedRows, selectedId, idKey);
  const int lastIndex = static_cast<int>(orderedRows.size()) - 1;
  const int resolvedIndex =
      selectedIndex >= 0
          ? selectedIndex
          : std::clamp(currentIndex, 0, lastIndex);
  const QString resolvedId = selectedIndex >= 0
                                 ? selectedId
                                 : rowIdAt(orderedRows, resolvedIndex, idKey);
  return {{QStringLiteral("rows"), orderedRows},
          {QStringLiteral("orderIds"), orderIds},
          {QStringLiteral("index"), resolvedIndex},
          {QStringLiteral("id"), resolvedId}};
}

QVariantMap navigateSelectionState(const QVariantList &rows, int currentIndex,
                                   const QString &selectedId, int delta,
                                   int defaultIndex = 0,
                                   const QString &idKey = QStringLiteral("id")) {
  const int selectedIndex = indexOfId(rows, selectedId, idKey);
  const int baseIndex = selectedIndex >= 0 ? selectedIndex : defaultIndex;
  const int nextIndex = wrappedIndex(baseIndex + delta, rows.size());
  return {{QStringLiteral("index"), nextIndex},
          {QStringLiteral("id"), rowIdAt(rows, nextIndex, idKey)}};
}

QVariantMap deleteReselectionState(const QVariantList &rows,
                                   const QVariantList &preferredOrder,
                                   int currentIndex,
                                   const QString &removedId,
                                   const QString &idKey = QStringLiteral("id")) {
  QVariantList remainingRows;
  for (const auto &rowValue : rows) {
    if (rowValue.toMap().value(idKey).toString() != removedId) {
      remainingRows.push_back(rowValue);
    }
  }
  QVariantList remainingOrder;
  for (const auto &id : preferredOrder) {
    if (id.toString() != removedId) {
      remainingOrder.push_back(id);
    }
  }
  return orderedSelectionState(remainingRows, remainingOrder, currentIndex, {},
                               idKey);
}

QString rememberedOrFirstRowId(const QVariantList &rows,
                               const QVariantMap &rememberedIds,
                               const QString &ownerId,
                               const QString &idKey = QStringLiteral("id")) {
  const QString remembered =
      rememberedIds.value(ownerId.trimmed()).toString().trimmed();
  if (rowHasId(rows, remembered, idKey)) {
    return remembered;
  }
  return rows.isEmpty() ? QString() : rowIdAt(rows, 0, idKey);
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

QVariantMap emptyTransactionFormState() {
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

QVariantMap normalizeTransactionFormState(const QVariantMap &tx) {
  QVariantMap out = emptyTransactionFormState();
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

QVariantList normalizeTransactionFormStates(const QVariantList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(normalizeTransactionFormState(value.toMap()));
  }
  return out;
}

bool transactionFormStateHasContent(const QVariantMap &tx) {
  const QVariantMap normalized = normalizeTransactionFormState(tx);
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

QVariantMap createFormStateList(const QVariantList &states, int currentIndex,
                                const QVariantMap &emptyState) {
  QVariantList normalizedStates = normalizeTransactionFormStates(states);
  if (normalizedStates.isEmpty()) {
    normalizedStates.push_back(normalizeTransactionFormState(emptyState));
  }
  const int lastIndex = static_cast<int>(normalizedStates.size()) - 1;
  const int index = std::clamp(currentIndex, 0, lastIndex);
  return {{QStringLiteral("states"), normalizedStates},
          {QStringLiteral("index"), index}};
}

QVariantMap insertFormStateAfterCurrent(const QVariantList &states,
                                        int currentIndex,
                                        const QVariantMap &emptyState) {
  const QVariantMap base =
      createFormStateList(states, currentIndex, emptyState);
  QVariantList normalizedStates = base.value(QStringLiteral("states")).toList();
  const int index = base.value(QStringLiteral("index")).toInt();
  const int insertIndex =
      std::clamp(index + 1, 0, static_cast<int>(normalizedStates.size()));
  normalizedStates.insert(insertIndex,
                          normalizeTransactionFormState(emptyState));
  return {{QStringLiteral("states"), normalizedStates},
          {QStringLiteral("index"), insertIndex}};
}

QVariantMap removeFormStateAt(const QVariantList &states, int currentIndex,
                              const QVariantMap &emptyState) {
  const QVariantMap base = createFormStateList(states, currentIndex, emptyState);
  QVariantList normalizedStates = base.value(QStringLiteral("states")).toList();
  int index = base.value(QStringLiteral("index")).toInt();
  if (index < 0 || index >= normalizedStates.size()) {
    index = 0;
  }
  normalizedStates.removeAt(index);
  if (normalizedStates.isEmpty()) {
    normalizedStates.push_back(normalizeTransactionFormState(emptyState));
    index = 0;
  } else {
    index = std::clamp(index, 0,
                       static_cast<int>(normalizedStates.size()) - 1);
  }
  return {{QStringLiteral("states"), normalizedStates},
          {QStringLiteral("index"), index}};
}

QVariantMap currentFormState(const QVariantList &states, int currentIndex,
                             const QVariantMap &emptyState) {
  const QVariantMap base = createFormStateList(states, currentIndex, emptyState);
  const QVariantList normalizedStates =
      base.value(QStringLiteral("states")).toList();
  const int index = base.value(QStringLiteral("index")).toInt();
  return {{QStringLiteral("states"), normalizedStates},
          {QStringLiteral("index"), index},
          {QStringLiteral("state"),
           normalizeTransactionFormState(normalizedStates.at(index).toMap())}};
}

QVariantMap setCurrentRawFormState(const QVariantList &states, int currentIndex,
                                   const QVariantMap &formState,
                                   const QVariantMap &emptyState) {
  QVariantMap state = currentFormState(states, currentIndex, emptyState);
  QVariantList nextStates = state.value(QStringLiteral("states")).toList();
  const int index = state.value(QStringLiteral("index")).toInt();
  if (index >= 0 && index < nextStates.size()) {
    nextStates[index] = formState;
  }
  state.insert(QStringLiteral("states"), nextStates);
  state.insert(QStringLiteral("state"), formState);
  return state;
}

bool bookingEditStateChanged(const QString &savedStatementName,
                             const QVariantMap &savedTransactionState,
                             const QString &currentStatementName,
                             const QVariantMap &transaction) {
  return savedStatementName != currentStatementName ||
         savedTransactionState != normalizeTransactionFormState(transaction);
}

} // namespace
BookingViewModel::BookingViewModel(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  createTransactionStates_ = QVariantList{emptyTransaction()};
  currentTransactionFormState_ = emptyTransaction();
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
    return createTransactionStates_.isEmpty() ? tr("No transactions")
                                               : tr("Transaction %1 / %2")
                                                   .arg(createTransactionIndex_ + 1)
                                               .arg(createTransactionStates_
                                                        .size());
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
  return isCreateMode() ? createTransactionStates_.size() > 1
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
    return createTransactionStates_.size() > 1;
  }
  return hasMultipleTransactions() &&
         !editTransactionData_.value(QStringLiteral("id")).toString().isEmpty();
}

QVariantMap BookingViewModel::emptyTransaction() const {
  return emptyTransactionFormState();
}

QVariantMap BookingViewModel::normalizeTransaction(const QVariantMap &tx) const {
  return normalizeTransactionFormState(tx);
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

QVariantMap BookingViewModel::normalizedTransactionState(
    const QVariantMap &data) const {
  return normalizeTransactionFormState(data);
}

QVariantMap BookingViewModel::transactionData() const {
  return isCreateMode() ? currentTransactionFormState_ : editTransactionData_;
}

void BookingViewModel::setTransactionData(const QVariantMap &value) {
  if (isCreateMode()) {
    if (!workspace_) {
      return;
    }
    const QVariantMap formState = value.isEmpty() ? emptyTransaction() : value;
    const QVariantMap state = setCurrentRawFormState(
        createTransactionStates_, createTransactionIndex_, formState,
        emptyTransaction());
    createTransactionStates_ = state.value(QStringLiteral("states")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    syncCurrentCreateTransaction();
  } else {
    if (editTransactionData_ == value) {
      return;
    }
    editTransactionData_ = value.isEmpty() ? emptyTransaction() : value;
  }
  emit changed();
}

QVariant BookingViewModel::transactionField(const QString &key) const {
  return transactionData().value(key);
}

void BookingViewModel::setTransactionField(const QString &key,
                                           const QVariant &value) {
  QVariantMap next = transactionData();
  next.insert(key, value);
  setTransactionData(next);
}

void BookingViewModel::applyTransactionFormChange(const QVariantMap &changes) {
  if (!workspace_) {
    return;
  }
  setTransactionData(
      workspace_->transactionFormWithCatalogSelection(transactionData(), changes));
}

QString BookingViewModel::transactionName() const {
  return transactionField(QStringLiteral("name")).toString();
}

void BookingViewModel::setTransactionName(const QString &value) {
  setTransactionField(QStringLiteral("name"), value);
}

QString BookingViewModel::transactionBookingDate() const {
  return transactionField(QStringLiteral("bookingDate")).toString();
}

void BookingViewModel::setTransactionBookingDate(const QString &value) {
  setTransactionField(QStringLiteral("bookingDate"), value);
}

QString BookingViewModel::transactionValuta() const {
  return transactionField(QStringLiteral("valuta")).toString();
}

void BookingViewModel::setTransactionValuta(const QString &value) {
  setTransactionField(QStringLiteral("valuta"), value);
}

QString BookingViewModel::transactionAmountText() const {
  const QVariant amount = transactionField(QStringLiteral("amount"));
  return amount.isValid() ? amount.toString() : QString();
}

void BookingViewModel::setTransactionAmountText(const QString &value) {
  setTransactionField(QStringLiteral("amount"), value);
}

bool BookingViewModel::transactionAllocatable() const {
  return transactionField(QStringLiteral("allocatable")).toBool();
}

void BookingViewModel::setTransactionAllocatable(bool value) {
  setTransactionField(QStringLiteral("allocatable"), value);
}

void BookingViewModel::captureEditState() {
  savedEditStatementName_ = editStatementName_;
  savedEditTransactionState_ = normalizedTransactionState(editTransactionData_);
}

void BookingViewModel::syncCurrentCreateTransaction() {
  if (!workspace_) {
    createTransactionStates_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionFormState_ = emptyTransaction();
    return;
  }
  const QVariantMap state = currentFormState(
      createTransactionStates_, createTransactionIndex_, emptyTransaction());
  createTransactionStates_ = state.value(QStringLiteral("states")).toList();
  createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
  currentTransactionFormState_ = state.value(QStringLiteral("state")).toMap();
}

void BookingViewModel::resetCreateState() {
  createStatementName_.clear();
  if (workspace_) {
    const QVariantMap state = createFormStateList(
        QVariantList(), 0, emptyTransaction());
    createTransactionStates_ = state.value(QStringLiteral("states")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentTransactionFormState_ = state.value(QStringLiteral("state")).toMap();
  } else {
    createTransactionStates_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionFormState_ = emptyTransaction();
  }
  emit changed();
}

void BookingViewModel::addTransactionAfterCurrent() {
  if (!workspace_) {
    return;
  }
  if (isCreateMode()) {
    const QVariantMap state = insertFormStateAfterCurrent(
        createTransactionStates_, createTransactionIndex_, emptyTransaction());
    createTransactionStates_ = state.value(QStringLiteral("states")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    syncCurrentCreateTransaction();
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
    const QVariantMap state = removeFormStateAt(
        createTransactionStates_, createTransactionIndex_, emptyTransaction());
    createTransactionStates_ = state.value(QStringLiteral("states")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    syncCurrentCreateTransaction();
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
    if (createTransactionStates_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        wrappedIndex(createTransactionIndex_ - 1,
                     createTransactionStates_.size());
    syncCurrentCreateTransaction();
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
    if (createTransactionStates_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        wrappedIndex(createTransactionIndex_ + 1,
                     createTransactionStates_.size());
    syncCurrentCreateTransaction();
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

bool BookingViewModel::transactionFormStateCanSubmit(
    const QVariantMap &state) const {
  if (!workspace_) {
    return false;
  }

  const QVariantMap tx = normalizeTransaction(state);
  if (!transactionFormStateHasContent(tx)) {
    return true;
  }

  const QString bookingDate =
      tx.value(QStringLiteral("bookingDate")).toString().trimmed();
  if (bookingDate.isEmpty()) {
    return false;
  }

  const double amount = workspace_->amountForTransactionCommit(
      state.value(QStringLiteral("amount")), QString(),
      tx.value(QStringLiteral("amount")).toDouble());
  return std::isfinite(amount);
}

QVariantList BookingViewModel::submittableTransactionFormStates() const {
  QVariantList out;
  if (!workspace_) {
    return out;
  }

  for (const QVariant &stateValue : createTransactionStates_) {
    const QVariantMap state = stateValue.toMap();
    const QVariantMap normalized = normalizeTransaction(state);
    if (!transactionFormStateHasContent(normalized)) {
      continue;
    }
    if (!transactionFormStateCanSubmit(state)) {
      continue;
    }
    out.push_back(state);
  }
  return out;
}

bool BookingViewModel::canCreate() const {
  if (createStatementName_.trimmed().isEmpty()) {
    return false;
  }
  for (const QVariant &stateValue : createTransactionStates_) {
    if (!transactionFormStateCanSubmit(stateValue.toMap())) {
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
      savedEditStatementName_, savedEditTransactionState_, editStatementName_,
      editTransactionData_);
}

QString BookingViewModel::submit() {
  if (!workspace_ || !canCreate()) {
    return QString();
  }
  observability::traceViewModel(
      "BookingViewModel::submit", "Booking create submitted",
      {{observability::context::kName, createStatementName_.toStdString()},
       {"transactionCount",
        std::to_string(submittableTransactionFormStates().size())}});
  const QString statementName = createStatementName_;
  const QVariantList transactionStates = submittableTransactionFormStates();
  const QString statementId = workspace_->addStatement(statementName);
  if (statementId.isEmpty()) {
    return QString();
  }
  for (const QVariant &stateValue : transactionStates) {
    const QVariantMap rawTx = stateValue.toMap();
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
  observability::traceViewModel(
      "BookingViewModel::updateCurrent", "Booking update submitted",
      {{"statementId", selectedStatementId().toStdString()},
       {"transactionId", selectedTransactionId().toStdString()}});
  const QString statementId = selectedStatementId();
  const bool statementChanged = savedEditStatementName_ != editStatementName_;
  const QString statementName = editStatementName_;
  const QVariantMap transactionData = editTransactionData_;
  const QString txId = transactionData.value(QStringLiteral("id")).toString();
  const bool transactionChanged =
      savedEditTransactionState_ != normalizedTransactionState(transactionData);
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
  observability::traceViewModel(
      "BookingViewModel::deleteCurrentStatement",
      "Statement delete submitted",
      {{"statementId", selectedStatementId().toStdString()}});
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
  setTransactionField(
      payload::keys::common::kStatus,
      option.value(payload::keys::common::kValue,
                   payload::transaction_status::kNeutral));
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
  return workspace_ ? workspace_->actorDropdownRows() : QVariantList();
}

QVariantList BookingViewModel::contractDisplayRows() const {
  return workspace_ ? workspace_->contractDropdownRows() : QVariantList();
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
                          transactionField(payload::keys::transaction::kActorId)
                              .toString());
}

int BookingViewModel::selectedContractIndex() const {
  return selectedIndexFor(contractDisplayRows(),
                          transactionField(payload::keys::transaction::kContractId)
                              .toString());
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
  applyTransactionFormChange(
      {{payload::keys::transaction::kPropertyIds, propertyIds}});
}

void BookingViewModel::selectActorIndex(int index) {
  const QVariantList rows = actorDisplayRows();
  const QString id =
      rows.value(index).toMap().value(payload::keys::common::kId).toString();
  applyTransactionFormChange({{payload::keys::transaction::kActorId, id}});
}

void BookingViewModel::selectContractIndex(int index) {
  const QVariantList rows = contractDisplayRows();
  const QString id =
      rows.value(index).toMap().value(payload::keys::common::kId).toString();
  applyTransactionFormChange({{payload::keys::transaction::kContractId, id}});
}

} // namespace ui
