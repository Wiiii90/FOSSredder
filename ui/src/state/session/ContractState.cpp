/**
 * @file ui/src/state/session/ContractState.cpp
 * @brief Implements the selection-aware contract UI state wrapper.
 */

#include "ui/state/session/ContractState.h"

#include "ui/state/session/WorkspaceSessionSelection.h"
#include "ui/state/session/WorkspaceSessionState.h"
#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>

#include <QStringList>

namespace ui {

namespace {

QStringList toQStringList(const QVariantList &values) {
  QStringList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(value.toString());
  }
  return out;
}

QVariantList toVariantList(const QStringList &values) {
  QVariantList out;
  out.reserve(values.size());
  for (const auto &value : values) {
    out.push_back(value);
  }
  return out;
}

} // namespace

ContractState::ContractState(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  bindSignals();
  reloadFromSelection(true);
}

QString ContractState::currentId() const {
  return workspace_ ? workspace_->selectedContractId() : QString();
}

QString ContractState::name() const { return name_; }

void ContractState::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  dirty_ = true;
  emit changed();
}

QString ContractState::type() const { return type_; }

void ContractState::setType(const QString &value) {
  if (type_ == value) {
    return;
  }
  type_ = value;
  dirty_ = true;
  emit changed();
}

QString ContractState::allocatableMode() const { return allocatableMode_; }

void ContractState::setAllocatableMode(const QString &value) {
  const QString next =
      value.trimmed().isEmpty() ? QStringLiteral("mixed") : value.trimmed();
  if (allocatableMode_ == next) {
    return;
  }
  allocatableMode_ = next;
  dirty_ = true;
  emit changed();
}

QVariantList ContractState::aliases() const { return aliases_; }

void ContractState::setAliases(const QVariantList &value) {
  if (aliases_ == value) {
    return;
  }
  aliases_ = value;
  if (aliasIndex_ >= aliases_.size()) {
    aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  }
  dirty_ = true;
  emit changed();
}

QString ContractState::aliasInputText() const { return aliasInputText_; }

void ContractState::setAliasInputText(const QString &value) {
  if (aliasInputText_ == value) {
    return;
  }
  aliasInputText_ = value;
  emit changed();
}

int ContractState::aliasIndex() const { return aliasIndex_; }

void ContractState::setAliasIndex(int value) {
  if (aliasIndex_ == value) {
    return;
  }
  aliasIndex_ = value;
  emit changed();
}

QVariantList ContractState::selectedActorIds() const {
  return selectedActorIds_;
}

void ContractState::setSelectedActorIds(const QVariantList &value) {
  if (selectedActorIds_ == value) {
    return;
  }
  selectedActorIds_ = value;
  dirty_ = true;
  emit changed();
}

QVariantList ContractState::selectedPropertyIds() const {
  return selectedPropertyIds_;
}

void ContractState::setSelectedPropertyIds(const QVariantList &value) {
  if (selectedPropertyIds_ == value) {
    return;
  }
  selectedPropertyIds_ = value;
  dirty_ = true;
  emit changed();
}

QVariantList ContractState::actorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList{};
}

QVariantList ContractState::actorDisplayRows() const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->displayRowsWithEmpty(
                   actorRows(), tr("No actor"), QStringLiteral("name"))
             : QVariantList{};
}

QVariantList ContractState::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList{};
}

QVariantList ContractState::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList{};
}

int ContractState::selectedActorIndex() const {
  if (!workspace_ || !workspace_->session()) {
    return 0;
  }
  const QString selectedActorId = selectedActorIds_.isEmpty()
                                      ? QString()
                                      : selectedActorIds_.front().toString();
  const int index =
      workspace_->session()->indexOfId(actorDisplayRows(), selectedActorId);
  return index >= 0 ? index : 0;
}

bool ContractState::isEdit() const { return !currentId().isEmpty(); }

bool ContractState::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  if (!workspace_ || !workspace_->session()) {
    return false;
  }
  return dirty_ || savedName_ != name_ || savedType_ != type_ ||
         savedAllocatableMode_ != allocatableMode_ ||
         workspace_->session()->normalizedStringListKey(savedAliases_) !=
             workspace_->session()->normalizedStringListKey(aliases_) ||
         workspace_->session()->normalizedStringListKey(
             savedSelectedActorIds_) !=
             workspace_->session()->normalizedStringListKey(
                 selectedActorIds_) ||
         workspace_->session()->normalizedStringListKey(
             savedSelectedPropertyIds_) !=
             workspace_->session()->normalizedStringListKey(
                 selectedPropertyIds_);
}

bool ContractState::canSubmit() const {
  return !name_.trimmed().isEmpty() && !type_.trimmed().isEmpty() &&
         (!selectedActorIds_.isEmpty() || !selectedPropertyIds_.isEmpty());
}

bool ContractState::canAddAlias(const QString &value) const {
  return !value.trimmed().isEmpty();
}

bool ContractState::canRemoveSelectedAlias() const {
  return aliasIndex_ >= 0 && aliasIndex_ < aliases_.size();
}

bool ContractState::isAliasSelected(int index) const {
  return aliasIndex_ == index;
}

bool ContractState::isPropertySelected(const QString &propertyId) const {
  const QString nextId = propertyId.trimmed();
  if (nextId.isEmpty()) {
    return false;
  }
  for (const auto &value : selectedPropertyIds_) {
    if (value.toString().trimmed() == nextId) {
      return true;
    }
  }
  return false;
}

void ContractState::clearFormState() {
  name_.clear();
  type_.clear();
  allocatableMode_ = QStringLiteral("mixed");
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
  selectedActorIds_.clear();
  selectedPropertyIds_.clear();
  dirty_ = false;
  emit changed();
}

void ContractState::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractState::enterCreateMode() {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedContractId(QString());
  }
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ContractState::addAlias(const QString &value) {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList next =
      workspace_->session()->addUniqueTrimmed(aliases_, value);
  if (next == aliases_) {
    return;
  }
  aliases_ = next;
  aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  aliasInputText_.clear();
  dirty_ = true;
  emit changed();
}

void ContractState::removeAlias(int index) {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList next = workspace_->session()->removeAt(aliases_, index);
  if (next == aliases_) {
    return;
  }
  aliases_ = next;
  aliasIndex_ = aliases_.isEmpty()
                    ? -1
                    : std::min(index, static_cast<int>(aliases_.size()) - 1);
  dirty_ = true;
  emit changed();
}

void ContractState::selectAlias(int index) { setAliasIndex(index); }

void ContractState::selectContract(const QString &id) {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedContractId(id.trimmed());
  }
}

void ContractState::requestRemoveSelectedAlias() {
  if (!canRemoveSelectedAlias()) {
    return;
  }
  removeAlias(aliasIndex_);
}

void ContractState::selectPrimaryActor(const QString &actorId) {
  const QString nextId = actorId.trimmed();
  QVariantList next;
  if (!nextId.isEmpty()) {
    next.push_back(nextId);
  }
  setSelectedActorIds(next);
}

void ContractState::setPropertySelected(const QString &propertyId,
                                        bool selected) {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QString normalizedId = propertyId.trimmed();
  if (normalizedId.isEmpty()) {
    return;
  }
  const QVariantList next = selected ? workspace_->session()->addUniqueTrimmed(
                                           selectedPropertyIds_, normalizedId)
                                     : workspace_->session()->removeString(
                                           selectedPropertyIds_, normalizedId);
  if (next == selectedPropertyIds_) {
    return;
  }
  selectedPropertyIds_ = next;
  dirty_ = true;
  emit changed();
}

void ContractState::previous() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), -1, rows.size() - 1);
  workspace_->selection()->setSelectedContractId(nextId);
}

void ContractState::next() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->contractRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), 1, 0);
  workspace_->selection()->setSelectedContractId(nextId);
}

QString ContractState::submit() {
  if (!workspace_) {
    return {};
  }
  const QStringList aliasValues = toQStringList(aliases_);
  const QStringList actorIds = toQStringList(selectedActorIds_);
  const QStringList propertyIds = toQStringList(selectedPropertyIds_);
  const QString id = workspace_->saveContract(
      isEdit() ? currentId() : QString(), name_, type_, actorIds, propertyIds,
      aliasValues, allocatableMode_);
  if (workspace_->selection() && !id.isEmpty()) {
    workspace_->selection()->setSelectedContractId(id);
  }
  captureSavedState();
  return id;
}

void ContractState::deleteCurrent() {
  if (!workspace_ || currentId().isEmpty()) {
    return;
  }
  const QString removedId = currentId();
  workspace_->deleteContract(removedId);
  if (!workspace_->selection() || !workspace_->session()) {
    return;
  }
  const QString nextId = workspace_->session()->deleteNextSelectionId(
      workspace_->contractRows(), removedId, 0, QStringLiteral("id"));
  workspace_->selection()->setSelectedContractId(nextId);
}

void ContractState::bindSignals() {
  if (!workspace_) {
    return;
  }

  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() { reloadFromSelection(true); });
  QObject::connect(workspace_->selection(),
                   &SessionSelection::selectedContractIdChanged, this,
                   [this]() { reloadFromSelection(false); });
  QObject::connect(workspace_->selectedContract(), &ContractSelection::changed,
                   this, [this]() { reloadFromSelection(true); });
}

void ContractState::applyFormState(const QVariantMap &state) {
  const QVariantMap next = state;
  name_ = next.value(QStringLiteral("name")).toString();
  type_ = next.value(QStringLiteral("type")).toString();
  allocatableMode_ =
      next.value(QStringLiteral("allocatableMode"), QStringLiteral("mixed"))
          .toString();
  aliases_ = next.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = next.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      next.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
  selectedActorIds_ = next.value(QStringLiteral("selectedActorIds")).toList();
  selectedPropertyIds_ =
      next.value(QStringLiteral("selectedPropertyIds")).toList();
  dirty_ = false;
  emit changed();
}

void ContractState::captureSavedState() {
  savedName_ = name_;
  savedType_ = type_;
  savedAllocatableMode_ = allocatableMode_;
  savedAliases_ = aliases_;
  savedSelectedActorIds_ = selectedActorIds_;
  savedSelectedPropertyIds_ = selectedPropertyIds_;
  dirty_ = false;
  emit changed();
}

QString ContractState::currentAllocatableMode() const {
  if (!workspace_) {
    return QStringLiteral("mixed");
  }
  const QString selectedId = currentId();
  for (const auto &value : workspace_->contractRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() != selectedId) {
      continue;
    }
    const QString mode =
        row.value(QStringLiteral("allocatableMode"), QStringLiteral("mixed"))
            .toString()
            .trimmed();
    return mode.isEmpty() ? QStringLiteral("mixed") : mode;
  }
  return QStringLiteral("mixed");
}

void ContractState::reloadFromSelection(bool forceReload) {
  if (!workspace_ || !workspace_->session()) {
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

  ContractSelection *current = workspace_->selectedContract();
  if (!current) {
    clearFormState();
    captureSavedState();
    return;
  }

  QVariantMap state = workspace_->session()->contractFormState(
      current->name(), current->type(), toVariantList(current->actorIds()),
      toVariantList(current->propertyIds()), toVariantList(current->aliases()));
  state.insert(QStringLiteral("allocatableMode"), currentAllocatableMode());
  applyFormState(state);
  captureSavedState();
}

} // namespace ui
