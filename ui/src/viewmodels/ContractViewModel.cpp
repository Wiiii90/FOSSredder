/**
 * @file ui/src/viewmodels/ContractViewModel.cpp
 * @brief Implements the QML API for editable contract state.
 */

#include "ui/viewmodels/ContractViewModel.h"

#include <QStringList>

#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

QString stringValue(const QVariant& value) {
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

QVariantList normalizeStringValues(const QVariantList& values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto& value : values) {
    out.push_back(stringValue(value));
  }
  return out;
}

QString rowIdAt(const QVariantList& rows, int index,
                const QString& idKey = payload::keys::common::kId) {
  if (index < 0 || index >= rows.size()) {
    return {};
  }
  return rows.at(index).toMap().value(idKey).toString();
}

int indexOfId(const QVariantList& rows, const QString& id,
              const QString& idKey = payload::keys::common::kId) {
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

QVariantMap rowById(const QVariantList& rows, const QString& id,
                    const QString& idKey = payload::keys::common::kId) {
  const int index = indexOfId(rows, id.trimmed(), idKey);
  return index >= 0 ? rows.at(index).toMap() : QVariantMap{};
}

QString
navigatedSelectionId(const QVariantList& rows, const QString& currentId,
                     int delta, int defaultIndex = 0,
                     const QString& idKey = payload::keys::common::kId) {
  const int currentIndex = indexOfId(rows, currentId, idKey);
  if (currentIndex < 0) {
    if (rows.isEmpty()) {
      return {};
    }
    if (delta > 0) {
      return rowIdAt(rows, 0, idKey);
    }
    if (delta < 0) {
      return rowIdAt(rows, rows.size() - 1, idKey);
    }
    return rowIdAt(rows, defaultIndex, idKey);
  }
  if (delta > 0) {
    return currentIndex >= rows.size() - 1
               ? QString()
               : rowIdAt(rows, currentIndex + 1, idKey);
  }
  if (delta < 0) {
    return currentIndex <= 0 ? QString()
                             : rowIdAt(rows, currentIndex - 1, idKey);
  }
  return rowIdAt(rows, currentIndex, idKey);
}

QString
deleteNextSelectionId(const QVariantList& rows, const QString& removedId,
                      int defaultIndex = 0,
                      const QString& idKey = payload::keys::common::kId) {
  if (rows.isEmpty()) {
    return {};
  }
  const int removedIndex = indexOfId(rows, removedId, idKey);
  const int nextIndex = removedIndex >= 0 ? removedIndex + 1 : defaultIndex;
  const int wrapped = nextIndex % rows.size();
  return rowIdAt(rows, wrapped < 0 ? wrapped + rows.size() : wrapped, idKey);
}

QVariantList displayRowsWithEmpty(const QVariantList& rows,
                                  const QString& emptyDisplay,
                                  const QString& displayKey) {
  QVariantList out;
  QVariantMap emptyRow;
  emptyRow.insert(payload::keys::common::kId, QString());
  emptyRow.insert(payload::keys::common::kDisplay, emptyDisplay);
  out.push_back(emptyRow);

  for (const auto& rowValue : rows) {
    const QVariantMap row = rowValue.toMap();
    QString display = row.value(displayKey).toString();
    if (display.isEmpty()) {
      display = row.value(payload::keys::common::kDisplay).toString();
    }
    if (display.isEmpty()) {
      display = row.value(payload::keys::common::kName).toString();
    }
    QVariantMap displayRow;
    displayRow.insert(payload::keys::common::kId,
                      row.value(payload::keys::common::kId).toString());
    displayRow.insert(payload::keys::common::kDisplay, display);
    out.push_back(displayRow);
  }
  return out;
}

bool canAddText(const QString& value) {
  return !value.trimmed().isEmpty();
}

bool canRemoveAt(const QVariantList& values, int index) {
  return index >= 0 && index < values.size();
}

bool containsTrimmed(const QVariantList& values, const QString& id) {
  const QString nextId = id.trimmed();
  if (nextId.isEmpty()) {
    return false;
  }
  for (const auto& value : values) {
    if (stringValue(value).trimmed() == nextId) {
      return true;
    }
  }
  return false;
}

QVariantList addUniqueTrimmed(const QVariantList& values,
                              const QString& value) {
  const QString next = value.trimmed();
  QVariantList out = normalizeStringValues(values);
  if (next.isEmpty() || containsTrimmed(out, next)) {
    return out;
  }
  out.push_back(next);
  return out;
}

QVariantList removeTrimmed(const QVariantList& values, const QString& value) {
  const QString target = value.trimmed();
  if (target.isEmpty()) {
    return values;
  }
  QVariantList out;
  out.reserve(values.size());
  for (const auto& entry : values) {
    const QString current = stringValue(entry).trimmed();
    if (current != target) {
      out.push_back(current);
    }
  }
  return out;
}

QVariantList removeAt(const QVariantList& values, int index) {
  QVariantList out = normalizeStringValues(values);
  if (!canRemoveAt(out, index)) {
    return out;
  }
  out.removeAt(index);
  return out;
}

int aliasIndexAfterRemoval(const QVariantList& aliases, int removedIndex) {
  if (aliases.isEmpty()) {
    return -1;
  }
  return removedIndex < aliases.size() ? removedIndex : aliases.size() - 1;
}

bool appendAliasValue(QVariantList& aliases, QString& aliasInputText,
                      int& aliasIndex, const QString& value) {
  const QVariantList next = addUniqueTrimmed(aliases, value);
  if (next == aliases) {
    return false;
  }
  aliases = next;
  aliasIndex = aliases.isEmpty() ? -1 : aliases.size() - 1;
  aliasInputText.clear();
  return true;
}

bool removeAliasValue(QVariantList& aliases, int& aliasIndex, int index) {
  const QVariantList next = removeAt(aliases, index);
  if (next == aliases) {
    return false;
  }
  aliases = next;
  aliasIndex = aliasIndexAfterRemoval(aliases, index);
  return true;
}

bool setSelectedId(QVariantList& selectedIds, const QString& id,
                   bool selected) {
  const QString normalizedId = id.trimmed();
  if (normalizedId.isEmpty()) {
    return false;
  }
  const QVariantList next = selected
                                ? addUniqueTrimmed(selectedIds, normalizedId)
                                : removeTrimmed(selectedIds, normalizedId);
  if (next == selectedIds) {
    return false;
  }
  selectedIds = next;
  return true;
}

QStringList normalizedStringList(const QVariantList& values) {
  const QVariantList normalized = normalizeStringValues(values);
  QStringList out;
  out.reserve(normalized.size());
  for (const auto& value : normalized) {
    out.push_back(value.toString());
  }
  out.sort();
  return out;
}

QString normalizedAllocatableMode(const QString& value) {
  const QString next = value.trimmed();
  return next.isEmpty() ? QStringLiteral("mixed") : next;
}

QVariantMap contractBaseFormStateFromRow(const QString& name,
                                         const QVariantList& aliases) {
  QVariantMap out;
  const QVariantList normalizedAliases = normalizeStringValues(aliases);
  out.insert(payload::keys::common::kName, name);
  out.insert(payload::keys::contract::kAliases, normalizedAliases);
  out.insert(payload::keys::state::kAliasInputText, QString());
  out.insert(payload::keys::state::kAliasIndex,
             normalizedAliases.isEmpty() ? -1 : 0);
  return out;
}

QVariantMap contractFormState(const QString& name, const QString& type,
                              const QString& allocatableMode,
                              const QVariantList& actorIds,
                              const QVariantList& propertyIds,
                              const QVariantList& aliases) {
  QVariantMap out = contractBaseFormStateFromRow(name, aliases);
  const QVariantList normalizedActorIds = normalizeStringValues(actorIds);
  QVariantList singleActorId;
  if (!normalizedActorIds.isEmpty()) {
    singleActorId.push_back(normalizedActorIds.first());
  }

  out.insert(payload::keys::common::kType, type);
  out.insert(payload::keys::contract::kAllocatableMode,
             normalizedAllocatableMode(allocatableMode));
  out.insert(payload::keys::state::kSelectedActorIds, singleActorId);
  out.insert(payload::keys::state::kSelectedPropertyIds,
             normalizeStringValues(propertyIds));
  return out;
}

} // namespace

ContractViewModel::ContractViewModel(WorkspaceStore* store,
                                     WorkspaceCommands* commands,
                                     WorkspaceSelection* selection,
                                     WorkspaceSelectors* selectors,
                                     QObject* parent)
    : QObject(parent), store_(store), commands_(commands),
      selection_(selection), selectors_(selectors) {
  bindSignals();
  reloadFromSelection(true);
}

void ContractViewModel::setName(const QString& value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emit changed();
}

void ContractViewModel::setAliases(const QVariantList& value) {
  if (aliases_ == value) {
    return;
  }
  aliases_ = value;
  if (aliasIndex_ >= aliases_.size()) {
    aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  }
  emit changed();
}

void ContractViewModel::setAliasInputText(const QString& value) {
  if (aliasInputText_ == value) {
    return;
  }
  aliasInputText_ = value;
  emit changed();
}

void ContractViewModel::setAliasIndex(int value) {
  if (aliasIndex_ == value) {
    return;
  }
  aliasIndex_ = value;
  emit changed();
}

bool ContractViewModel::canAddAlias(const QString& value) const {
  return canAddText(value);
}

bool ContractViewModel::hasValidAliasSelection() const {
  return canRemoveAt(aliases_, aliasIndex_);
}

void ContractViewModel::addAlias(const QString& value) {
  if (!appendAliasValue(aliases_, aliasInputText_, aliasIndex_, value)) {
    return;
  }
  emit changed();
}

void ContractViewModel::requestRemoveSelectedAlias() {
  if (!hasValidAliasSelection() ||
      !removeAliasValue(aliases_, aliasIndex_, aliasIndex_)) {
    return;
  }
  emit changed();
}

QString ContractViewModel::currentId() const {
  return selection_ ? selection_->selectedContractId() : QString();
}

QString ContractViewModel::type() const {
  return type_;
}

void ContractViewModel::setType(const QString& value) {
  if (type_ == value) {
    return;
  }
  type_ = value;
  emit changed();
}

QString ContractViewModel::allocatableMode() const {
  return allocatableMode_;
}

void ContractViewModel::setAllocatableMode(const QString& value) {
  const QString next = normalizedAllocatableMode(value);
  if (allocatableMode_ == next) {
    return;
  }
  allocatableMode_ = next;
  emit changed();
}

QVariantList ContractViewModel::selectedActorIds() const {
  return selectedActorIds_;
}

void ContractViewModel::setSelectedActorIds(const QVariantList& value) {
  if (selectedActorIds_ == value) {
    return;
  }
  selectedActorIds_ = value;
  emit changed();
}

QVariantList ContractViewModel::selectedPropertyIds() const {
  return selectedPropertyIds_;
}

void ContractViewModel::setSelectedPropertyIds(const QVariantList& value) {
  if (selectedPropertyIds_ == value) {
    return;
  }
  selectedPropertyIds_ = value;
  emit changed();
}

QVariantList ContractViewModel::actorRows() const {
  return selectors_ ? selectors_->actorRows() : QVariantList{};
}

QVariantList ContractViewModel::actorDisplayRows() const {
  return selectors_ ? displayRowsWithEmpty(actorRows(), tr("No actor"),
                                           QStringLiteral("name"))
                    : QVariantList{};
}

QVariantList ContractViewModel::contractRows() const {
  return selectors_ ? selectors_->contractRows() : QVariantList{};
}

QVariantList ContractViewModel::propertyRows() const {
  return selectors_ ? selectors_->propertyRows() : QVariantList{};
}

int ContractViewModel::selectedActorIndex() const {
  if (!selectors_) {
    return 0;
  }
  const QString selectedActorId = selectedActorIds_.isEmpty()
                                      ? QString()
                                      : selectedActorIds_.front().toString();
  const int index = indexOfId(actorDisplayRows(), selectedActorId);
  return index >= 0 ? index : 0;
}

bool ContractViewModel::isEdit() const {
  return !currentId().isEmpty();
}

bool ContractViewModel::canSubmit() const {
  return !name_.trimmed().isEmpty() && !type_.trimmed().isEmpty() &&
         (!selectedActorIds_.isEmpty() || !selectedPropertyIds_.isEmpty());
}

void ContractViewModel::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractViewModel::enterCreateMode() {
  if (selection_) {
    selection_->selectContract(QString());
  }
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractViewModel::selectContract(const QString& id) {
  if (selection_) {
    selection_->selectContract(id);
  }
}

void ContractViewModel::selectPrimaryActor(const QString& actorId) {
  const QString nextId = actorId.trimmed();
  QVariantList next;
  if (!nextId.isEmpty()) {
    next.push_back(nextId);
  }
  setSelectedActorIds(next);
}

void ContractViewModel::setPropertySelected(const QString& propertyId,
                                            bool selected) {
  if (!selection_) {
    return;
  }
  if (!setSelectedId(selectedPropertyIds_, propertyId, selected)) {
    return;
  }
  emit changed();
}

void ContractViewModel::previous() {
  if (!selection_) {
    return;
  }
  const QVariantList rows = contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), -1, rows.size() - 1);
  selection_->selectContract(nextId);
}

void ContractViewModel::next() {
  if (!selection_) {
    return;
  }
  const QVariantList rows = contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId =
      navigatedSelectionId(rows, isEdit() ? currentId() : QString(), 1, 0);
  selection_->selectContract(nextId);
}

QString ContractViewModel::submit() {
  if (!commands_ || !selection_) {
    return {};
  }
  observability::traceViewModel(
      "ContractViewModel::submit",
      isEdit() ? "Contract update submitted" : "Contract create submitted",
      {{observability::context::kId, currentId().toStdString()},
       {observability::context::kName, name_.toStdString()},
       {"type", type_.toStdString()}});
  const QStringList aliasValues = payload::mapper::toQStringList(aliases_);
  const QStringList actorIds =
      payload::mapper::toQStringList(selectedActorIds_);
  const QStringList propertyIds =
      payload::mapper::toQStringList(selectedPropertyIds_);
  const QString id = commands_->saveContract(
      isEdit() ? currentId() : QString(), name_, type_, actorIds, propertyIds,
      aliasValues, allocatableMode_);
  if (!id.isEmpty()) {
    selection_->selectContract(id);
  }
  captureSavedState();
  return id;
}

void ContractViewModel::deleteCurrent() {
  if (!commands_ || currentId().isEmpty()) {
    return;
  }
  observability::traceViewModel(
      "ContractViewModel::deleteCurrent", "Contract delete submitted",
      {{observability::context::kId, currentId().toStdString()}});
  const QString removedId = currentId();
  commands_->deleteContract(removedId);
  const QString nextId =
      deleteNextSelectionId(contractRows(), removedId, 0, QStringLiteral("id"));
  if (selection_)
    selection_->selectContract(nextId);
}

bool ContractViewModel::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  if (!selectors_) {
    return false;
  }
  return savedName_ != name_ || savedType_ != type_ ||
         savedAllocatableMode_ != allocatableMode_ ||
         normalizedStringList(savedAliases_) !=
             normalizedStringList(aliases_) ||
         normalizedStringList(savedSelectedActorIds_) !=
             normalizedStringList(selectedActorIds_) ||
         normalizedStringList(savedSelectedPropertyIds_) !=
             normalizedStringList(selectedPropertyIds_);
}

void ContractViewModel::clearFormState() {
  name_.clear();
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
  type_.clear();
  allocatableMode_ = QStringLiteral("mixed");
  selectedActorIds_.clear();
  selectedPropertyIds_.clear();
  emit changed();
}

void ContractViewModel::bindSignals() {
  if (!store_ || !selection_) {
    return;
  }

  QObject::connect(store_, &WorkspaceStore::dataRevisionChanged, this,
                   [this]() {
                     reloadFromSelection(true);
                   });
  QObject::connect(selection_, &WorkspaceSelection::selectedContractIdChanged,
                   this, [this]() {
                     reloadFromSelection(false);
                   });
}

void ContractViewModel::applyFormState(const QVariantMap& state) {
  const QVariantMap next = state;
  name_ = next.value(QStringLiteral("name")).toString();
  aliases_ = next.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = next.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      next.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
  type_ = next.value(QStringLiteral("type")).toString();
  allocatableMode_ = normalizedAllocatableMode(
      next.value(payload::keys::contract::kAllocatableMode,
                 QStringLiteral("mixed"))
          .toString());
  selectedActorIds_ = next.value(QStringLiteral("selectedActorIds")).toList();
  selectedPropertyIds_ =
      next.value(QStringLiteral("selectedPropertyIds")).toList();
  emit changed();
}

void ContractViewModel::captureSavedState() {
  savedName_ = name_;
  savedAliases_ = aliases_;
  savedType_ = type_;
  savedAllocatableMode_ = allocatableMode_;
  savedSelectedActorIds_ = selectedActorIds_;
  savedSelectedPropertyIds_ = selectedPropertyIds_;
  emit changed();
}

void ContractViewModel::reloadFromSelection(bool forceReload) {
  if (!selection_ || !selectors_) {
    return;
  }

  const QString nextOwnerId = currentId();
  if (!forceReload && currentOwnerId_ == nextOwnerId) {
    return;
  }

  currentOwnerId_ = nextOwnerId;
  if (nextOwnerId.isEmpty()) {
    clearFormState();
    captureSavedState();
    return;
  }

  const QVariantMap row = rowById(contractRows(), nextOwnerId);
  if (row.isEmpty()) {
    clearFormState();
    captureSavedState();
    return;
  }

  QVariantMap state = contractFormState(
      row.value(payload::keys::common::kName).toString(),
      row.value(payload::keys::common::kType).toString(),
      row.value(payload::keys::contract::kAllocatableMode,
                QStringLiteral("mixed"))
          .toString(),
      row.value(payload::keys::contract::kActorIds).toList(),
      row.value(payload::keys::contract::kPropertyIds).toList(),
      row.value(payload::keys::contract::kAliases).toList());
  applyFormState(state);
  captureSavedState();
}

} // namespace ui
