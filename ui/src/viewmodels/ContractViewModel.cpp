/**
 * @file ui/src/viewmodels/ContractViewModel.cpp
 * @brief Implements the selection-aware contract UI state wrapper.
 */

#include "ui/viewmodels/ContractViewModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

#include "ui/shared/payload/PayloadKeys.h"
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

bool canAddText(const QString &value) { return !value.trimmed().isEmpty(); }

bool canRemoveAt(const QVariantList &values, int index) {
  return index >= 0 && index < values.size();
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

QVariantList removeAt(const QVariantList &values, int index) {
  QVariantList out = normalizeStringValues(values);
  if (!canRemoveAt(out, index)) {
    return out;
  }
  out.removeAt(index);
  return out;
}

int aliasIndexAfterRemoval(const QVariantList &aliases, int removedIndex) {
  if (aliases.isEmpty()) {
    return -1;
  }
  return removedIndex < aliases.size() ? removedIndex : aliases.size() - 1;
}

bool appendAliasValue(QVariantList &aliases, QString &aliasInputText,
                      int &aliasIndex, const QString &value) {
  const QVariantList next = addUniqueTrimmed(aliases, value);
  if (next == aliases) {
    return false;
  }
  aliases = next;
  aliasIndex = aliases.isEmpty() ? -1 : aliases.size() - 1;
  aliasInputText.clear();
  return true;
}

bool removeAliasValue(QVariantList &aliases, int &aliasIndex, int index) {
  const QVariantList next = removeAt(aliases, index);
  if (next == aliases) {
    return false;
  }
  aliases = next;
  aliasIndex = aliasIndexAfterRemoval(aliases, index);
  return true;
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

QString normalizedStringListKey(const QVariantList &values) {
  const QVariantList normalized = normalizeStringValues(values);
  QStringList out;
  out.reserve(normalized.size());
  for (const auto &value : normalized) {
    out.push_back(value.toString());
  }
  out.sort();

  QJsonArray array;
  for (const auto &value : out) {
    array.append(value);
  }
  return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QString normalizedAllocatableMode(const QString &value) {
  const QString next = value.trimmed();
  return next.isEmpty() ? QStringLiteral("mixed") : next;
}

QVariantMap basicFormState(const QString &name, const QVariantList &aliases) {
  QVariantMap out;
  const QVariantList normalizedAliases = normalizeStringValues(aliases);
  out.insert(payload::keys::common::kName, name);
  out.insert(payload::keys::actor::kAliases, normalizedAliases);
  out.insert(payload::keys::state::kAliasInputText, QString());
  out.insert(payload::keys::state::kAliasIndex,
             normalizedAliases.isEmpty() ? -1 : 0);
  return out;
}

QVariantMap contractFormState(const QString &name, const QString &type,
                              const QVariantList &actorIds,
                              const QVariantList &propertyIds,
                              const QVariantList &aliases) {
  QVariantMap out = basicFormState(name, aliases);
  const QVariantList normalizedActorIds = normalizeStringValues(actorIds);
  QVariantList singleActorId;
  if (!normalizedActorIds.isEmpty()) {
    singleActorId.push_back(normalizedActorIds.first());
  }

  out.insert(payload::keys::common::kType, type);
  out.insert(payload::keys::state::kSelectedActorIds, singleActorId);
  out.insert(payload::keys::state::kSelectedPropertyIds,
             normalizeStringValues(propertyIds));
  return out;
}

} // namespace

ContractViewModel::ContractViewModel(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  bindSignals();
  reloadFromSelection(true);
}

void ContractViewModel::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emit changed();
}

void ContractViewModel::setAliases(const QVariantList &value) {
  if (aliases_ == value) {
    return;
  }
  aliases_ = value;
  if (aliasIndex_ >= aliases_.size()) {
    aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  }
  emit changed();
}

void ContractViewModel::setAliasInputText(const QString &value) {
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

bool ContractViewModel::canAddAlias(const QString &value) const {
  return canAddText(value);
}

bool ContractViewModel::canRemoveSelectedAlias() const {
  return canRemoveAt(aliases_, aliasIndex_);
}

bool ContractViewModel::isAliasSelected(int index) const {
  return aliasIndex_ == index;
}

void ContractViewModel::addAlias(const QString &value) {
  if (!appendAliasValue(aliases_, aliasInputText_, aliasIndex_, value)) {
    return;
  }
  emit changed();
}

void ContractViewModel::removeAlias(int index) {
  if (!removeAliasValue(aliases_, aliasIndex_, index)) {
    return;
  }
  emit changed();
}

void ContractViewModel::selectAlias(int index) { setAliasIndex(index); }

void ContractViewModel::requestRemoveSelectedAlias() {
  if (canRemoveSelectedAlias()) {
    removeAlias(aliasIndex_);
  }
}

QString ContractViewModel::currentId() const {
  return workspace_ ? workspace_->selectedContractId() : QString();
}

QString ContractViewModel::type() const { return type_; }

void ContractViewModel::setType(const QString &value) {
  if (type_ == value) {
    return;
  }
  type_ = value;
  emit changed();
}

QString ContractViewModel::allocatableMode() const { return allocatableMode_; }

void ContractViewModel::setAllocatableMode(const QString &value) {
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

void ContractViewModel::setSelectedActorIds(const QVariantList &value) {
  if (selectedActorIds_ == value) {
    return;
  }
  selectedActorIds_ = value;
  emit changed();
}

QVariantList ContractViewModel::selectedPropertyIds() const {
  return selectedPropertyIds_;
}

void ContractViewModel::setSelectedPropertyIds(const QVariantList &value) {
  if (selectedPropertyIds_ == value) {
    return;
  }
  selectedPropertyIds_ = value;
  emit changed();
}

QVariantList ContractViewModel::actorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList{};
}

QVariantList ContractViewModel::actorDisplayRows() const {
  return workspace_ ? displayRowsWithEmpty(actorRows(), tr("No actor"),
                                           QStringLiteral("name"))
                    : QVariantList{};
}

QVariantList ContractViewModel::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList{};
}

QVariantList ContractViewModel::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList{};
}

int ContractViewModel::selectedActorIndex() const {
  if (!workspace_) {
    return 0;
  }
  const QString selectedActorId = selectedActorIds_.isEmpty()
                                      ? QString()
                                      : selectedActorIds_.front().toString();
  const int index = indexOfId(actorDisplayRows(), selectedActorId);
  return index >= 0 ? index : 0;
}

bool ContractViewModel::isEdit() const { return !currentId().isEmpty(); }

bool ContractViewModel::canSubmit() const {
  return !name_.trimmed().isEmpty() && !type_.trimmed().isEmpty() &&
         (!selectedActorIds_.isEmpty() || !selectedPropertyIds_.isEmpty());
}

bool ContractViewModel::isPropertySelected(const QString &propertyId) const {
  return containsTrimmed(selectedPropertyIds_, propertyId);
}

void ContractViewModel::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractViewModel::enterCreateMode() {
  if (workspace_) {
    workspace_->selectContract(QString());
  }
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractViewModel::selectContract(const QString &id) {
  if (workspace_) {
    workspace_->selectContract(id);
  }
}

void ContractViewModel::selectPrimaryActor(const QString &actorId) {
  const QString nextId = actorId.trimmed();
  QVariantList next;
  if (!nextId.isEmpty()) {
    next.push_back(nextId);
  }
  setSelectedActorIds(next);
}

void ContractViewModel::setPropertySelected(const QString &propertyId,
                                            bool selected) {
  if (!workspace_) {
    return;
  }
  if (!setSelectedId(selectedPropertyIds_, propertyId, selected)) {
    return;
  }
  emit changed();
}

void ContractViewModel::previous() {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), -1, rows.size() - 1);
  workspace_->selectContract(nextId);
}

void ContractViewModel::next() {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId =
      navigatedSelectionId(rows, isEdit() ? currentId() : QString(), 1, 0);
  workspace_->selectContract(nextId);
}

QString ContractViewModel::submit() {
  if (!workspace_) {
    return {};
  }
  const QStringList aliasValues = payload::mapper::toQStringList(aliases_);
  const QStringList actorIds =
      payload::mapper::toQStringList(selectedActorIds_);
  const QStringList propertyIds =
      payload::mapper::toQStringList(selectedPropertyIds_);
  const QString id = workspace_->saveContract(
      isEdit() ? currentId() : QString(), name_, type_, actorIds, propertyIds,
      aliasValues, allocatableMode_);
  if (!id.isEmpty()) {
    workspace_->selectContract(id);
  }
  captureSavedState();
  return id;
}

void ContractViewModel::deleteCurrent() {
  if (!workspace_ || currentId().isEmpty()) {
    return;
  }
  const QString removedId = currentId();
  workspace_->deleteContract(removedId);
  if (!workspace_) {
    return;
  }
  const QString nextId =
      deleteNextSelectionId(contractRows(), removedId, 0, QStringLiteral("id"));
  workspace_->selectContract(nextId);
}

bool ContractViewModel::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  if (!workspace_) {
    return false;
  }
  return savedName_ != name_ || savedType_ != type_ ||
         savedAllocatableMode_ != allocatableMode_ ||
         normalizedStringListKey(savedAliases_) !=
             normalizedStringListKey(aliases_) ||
         normalizedStringListKey(savedSelectedActorIds_) !=
             normalizedStringListKey(selectedActorIds_) ||
         normalizedStringListKey(savedSelectedPropertyIds_) !=
             normalizedStringListKey(selectedPropertyIds_);
}

void ContractViewModel::clearFormState() {
  clearBaseFormState();
  type_.clear();
  allocatableMode_ = QStringLiteral("mixed");
  selectedActorIds_.clear();
  selectedPropertyIds_.clear();
  emit changed();
}

void ContractViewModel::bindSignals() {
  if (!workspace_) {
    return;
  }

  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() { reloadFromSelection(true); });
  QObject::connect(workspace_, &WorkspaceFacade::selectedContractIdChanged, this,
                   [this]() { reloadFromSelection(false); });
}

void ContractViewModel::applyFormState(const QVariantMap &state) {
  const QVariantMap next = state;
  applyBaseFormState(next);
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
  captureBaseSavedState();
  savedType_ = type_;
  savedAllocatableMode_ = allocatableMode_;
  savedAliases_ = aliases_;
  savedSelectedActorIds_ = selectedActorIds_;
  savedSelectedPropertyIds_ = selectedPropertyIds_;
  emit changed();
}

QString ContractViewModel::currentAllocatableMode() const {
  if (!workspace_) {
    return QStringLiteral("mixed");
  }
  const QString selectedId = currentId();
  for (const auto &value : contractRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() != selectedId) {
      continue;
    }
    const QString mode =
        row.value(payload::keys::contract::kAllocatableMode,
                  QStringLiteral("mixed"))
            .toString();
    return normalizedAllocatableMode(mode);
  }
  return QStringLiteral("mixed");
}

void ContractViewModel::clearBaseFormState() {
  name_.clear();
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
}

void ContractViewModel::captureBaseSavedState() {
  savedName_ = name_;
  savedAliases_ = aliases_;
}

void ContractViewModel::applyBaseFormState(const QVariantMap &state) {
  name_ = state.value(QStringLiteral("name")).toString();
  aliases_ = state.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = state.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      state.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
}

void ContractViewModel::reloadFromSelection(bool forceReload) {
  if (!workspace_) {
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

  QVariantMap state =
      contractFormState(row.value(payload::keys::common::kName).toString(),
                        row.value(payload::keys::common::kType).toString(),
                        row.value(payload::keys::contract::kActorIds).toList(),
                        row.value(payload::keys::contract::kPropertyIds)
                            .toList(),
                        row.value(payload::keys::contract::kAliases).toList());
  state.insert(payload::keys::contract::kAllocatableMode,
               currentAllocatableMode());
  applyFormState(state);
  captureSavedState();
}

} // namespace ui
