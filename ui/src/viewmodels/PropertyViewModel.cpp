/**
 * @file ui/src/viewmodels/PropertyViewModel.cpp
 * @brief Implements the selection-aware property UI state wrapper.
 */

#include "ui/viewmodels/PropertyViewModel.h"

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

bool formStateChanged(const QString &savedName,
                      const QVariantList &savedAliases,
                      const QVariantList &savedContractIds,
                      const QString &currentName,
                      const QVariantList &aliases,
                      const QVariantList &contractIds) {
  return savedName != currentName ||
         normalizedStringListKey(savedAliases) !=
             normalizedStringListKey(aliases) ||
         normalizedStringListKey(savedContractIds) !=
             normalizedStringListKey(contractIds);
}

QVariantMap basicFormState(const QString &name, const QVariantList &aliases,
                           const QVariantList &selectedIds = {}) {
  QVariantMap out;
  const QVariantList normalizedAliases = normalizeStringValues(aliases);
  out.insert(payload::keys::common::kName, name);
  out.insert(payload::keys::property::kAliases, normalizedAliases);
  out.insert(payload::keys::state::kAliasInputText, QString());
  out.insert(payload::keys::state::kAliasIndex,
             normalizedAliases.isEmpty() ? -1 : 0);
  out.insert(payload::keys::state::kSelectedIds,
             normalizeStringValues(selectedIds));
  return out;
}

} // namespace

PropertyViewModel::PropertyViewModel(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  bindSignals();
  reloadFromSelection(true);
}

void PropertyViewModel::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emit changed();
}

void PropertyViewModel::setAliases(const QVariantList &value) {
  if (aliases_ == value) {
    return;
  }
  aliases_ = value;
  if (aliasIndex_ >= aliases_.size()) {
    aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  }
  emit changed();
}

void PropertyViewModel::setAliasInputText(const QString &value) {
  if (aliasInputText_ == value) {
    return;
  }
  aliasInputText_ = value;
  emit changed();
}

void PropertyViewModel::setAliasIndex(int value) {
  if (aliasIndex_ == value) {
    return;
  }
  aliasIndex_ = value;
  emit changed();
}

void PropertyViewModel::setSelectedContractIds(const QVariantList &value) {
  if (selectedContractIds_ == value) {
    return;
  }
  selectedContractIds_ = value;
  emit changed();
}

bool PropertyViewModel::isEdit() const { return !currentId().isEmpty(); }

bool PropertyViewModel::canSubmit() const { return !name_.trimmed().isEmpty(); }

bool PropertyViewModel::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  return formStateChanged(savedName_, savedAliases_, savedSelectedContractIds_,
                          name_, aliases_, selectedContractIds_);
}

bool PropertyViewModel::canAddAlias(const QString &value) const {
  return canAddText(value);
}

bool PropertyViewModel::canRemoveSelectedAlias() const {
  return canRemoveAt(aliases_, aliasIndex_);
}

bool PropertyViewModel::isAliasSelected(int index) const {
  return aliasIndex_ == index;
}

void PropertyViewModel::addAlias(const QString &value) {
  if (!appendAliasValue(aliases_, aliasInputText_, aliasIndex_, value)) {
    return;
  }
  emit changed();
}

void PropertyViewModel::removeAlias(int index) {
  if (!removeAliasValue(aliases_, aliasIndex_, index)) {
    return;
  }
  emit changed();
}

void PropertyViewModel::selectAlias(int index) { setAliasIndex(index); }

void PropertyViewModel::requestRemoveSelectedAlias() {
  if (canRemoveSelectedAlias()) {
    removeAlias(aliasIndex_);
  }
}

bool PropertyViewModel::isContractSelected(const QString &contractId) const {
  return containsTrimmed(selectedContractIds_, contractId);
}

void PropertyViewModel::enterCreateMode() {
  setCurrentId({});
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void PropertyViewModel::previous() {
  const QVariantList rows = primaryRows();
  if (rows.isEmpty()) {
    return;
  }
  setCurrentId(navigatedSelectionId(rows, isEdit() ? currentId() : QString(),
                                    -1, rows.size() - 1));
}

void PropertyViewModel::next() {
  const QVariantList rows = primaryRows();
  if (rows.isEmpty()) {
    return;
  }
  setCurrentId(
      navigatedSelectionId(rows, isEdit() ? currentId() : QString(), 1, 0));
}

QString PropertyViewModel::submit() {
  if (!workspace_) {
    return {};
  }
  const QString id = saveLinkedCatalogObject(
      isEdit() ? currentId() : QString(), name_,
      payload::mapper::toQStringList(aliases_),
      payload::mapper::toQStringList(selectedContractIds_));
  if (!id.isEmpty()) {
    setCurrentId(id);
  }
  captureSavedState();
  return id;
}

void PropertyViewModel::deleteCurrent() {
  if (!workspace_ || currentId().isEmpty()) {
    return;
  }
  const QString removedId = currentId();
  deleteLinkedCatalogObject(removedId);
  setCurrentId(deleteNextSelectionId(primaryRows(), removedId, 0,
                                     QStringLiteral("id")));
}

void PropertyViewModel::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void PropertyViewModel::setContractSelected(const QString &contractId,
                                            bool selected) {
  if (!setSelectedId(selectedContractIds_, contractId, selected)) {
    return;
  }
  emit changed();
}

QString PropertyViewModel::currentId() const {
  return workspace_ ? workspace_->selectedPropertyId() : QString();
}

QVariantList PropertyViewModel::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList{};
}

QVariantList PropertyViewModel::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList{};
}

QVariantList PropertyViewModel::primaryRows() const { return propertyRows(); }

void PropertyViewModel::setCurrentId(const QString &id) {
  if (workspace_) {
    workspace_->selectProperty(id);
  }
}

void PropertyViewModel::selectProperty(const QString &id) { setCurrentId(id); }

QString PropertyViewModel::saveLinkedCatalogObject(
    const QString &id, const QString &name, const QStringList &aliases,
    const QStringList &contractIds) {
  if (!workspace_) {
    return {};
  }
  return workspace_->saveProperty(id, name, aliases, contractIds);
}

void PropertyViewModel::deleteLinkedCatalogObject(const QString &id) {
  if (workspace_) {
    workspace_->deleteProperty(id);
  }
}

void PropertyViewModel::bindSignals() {
  if (!workspace_) {
    return;
  }

  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() { reloadFromSelection(true); });
  QObject::connect(workspace_, &WorkspaceFacade::selectedPropertyIdChanged, this,
                   [this]() { reloadFromSelection(false); });
}

void PropertyViewModel::reloadFromSelection(bool forceReload) {
  if (!workspace_) {
    return;
  }

  const QString selectedId = currentId();
  if (selectedId.isEmpty()) {
    reloadLinkedFormState(forceReload, {});
    return;
  }

  const QVariantMap row = rowById(propertyRows(), selectedId);
  const QVariantMap state =
      row.isEmpty()
          ? QVariantMap{}
          : basicFormState(row.value(payload::keys::common::kName).toString(),
                           row.value(payload::keys::property::kAliases)
                               .toList(),
                           row.value(payload::keys::state::kSelectedIds)
                               .toList());
  reloadLinkedFormState(forceReload, state);
}

void PropertyViewModel::clearFormState() {
  name_.clear();
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
  selectedContractIds_.clear();
  emit changed();
}

void PropertyViewModel::captureSavedState() {
  savedName_ = name_;
  savedAliases_ = aliases_;
  savedSelectedContractIds_ = selectedContractIds_;
  emit changed();
}

void PropertyViewModel::applyFormState(const QVariantMap &state) {
  name_ = state.value(QStringLiteral("name")).toString();
  aliases_ = state.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = state.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      state.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
  selectedContractIds_ = state.value(QStringLiteral("selectedIds")).toList();
  emit changed();
}

void PropertyViewModel::reloadLinkedFormState(bool forceReload,
                                              const QVariantMap &state) {
  const QString nextOwnerId = currentId();
  if (!forceReload && currentOwnerId_ == nextOwnerId) {
    return;
  }

  currentOwnerId_ = nextOwnerId;
  if (nextOwnerId.isEmpty() || state.isEmpty()) {
    clearFormState();
    captureSavedState();
    return;
  }

  applyFormState(state);
  captureSavedState();
}

} // namespace ui
