/**
 * @file ui/src/state/session/ActorState.cpp
 * @brief Implements the selection-aware actor UI state wrapper.
 */

#include "ui/state/session/ActorState.h"

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

ActorState::ActorState(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  bindSignals();
  reloadFromSelection(true);
}

QString ActorState::currentId() const {
  return workspace_ ? workspace_->selectedActorId() : QString();
}

QString ActorState::name() const { return name_; }

void ActorState::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emit changed();
}

QVariantList ActorState::aliases() const { return aliases_; }

void ActorState::setAliases(const QVariantList &value) {
  if (aliases_ == value) {
    return;
  }
  aliases_ = value;
  if (aliasIndex_ >= aliases_.size()) {
    aliasIndex_ = aliases_.isEmpty() ? -1 : aliases_.size() - 1;
  }
  emit changed();
}

QString ActorState::aliasInputText() const { return aliasInputText_; }

void ActorState::setAliasInputText(const QString &value) {
  if (aliasInputText_ == value) {
    return;
  }
  aliasInputText_ = value;
  emit changed();
}

int ActorState::aliasIndex() const { return aliasIndex_; }

void ActorState::setAliasIndex(int value) {
  if (aliasIndex_ == value) {
    return;
  }
  aliasIndex_ = value;
  emit changed();
}

QVariantList ActorState::selectedContractIds() const {
  return selectedContractIds_;
}

void ActorState::setSelectedContractIds(const QVariantList &value) {
  if (selectedContractIds_ == value) {
    return;
  }
  selectedContractIds_ = value;
  emit changed();
}

QVariantList ActorState::actorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList{};
}

QVariantList ActorState::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList{};
}

bool ActorState::isEdit() const { return !currentId().isEmpty(); }

bool ActorState::hasChanges() const {
  if (!isEdit()) {
    return !name_.trimmed().isEmpty();
  }
  if (!workspace_ || !workspace_->session()) {
    return false;
  }
  return workspace_->session()->formStateChanged(
      savedName_, savedAliases_, savedSelectedContractIds_, name_, aliases_,
      selectedContractIds_);
}

bool ActorState::canSubmit() const { return !name_.trimmed().isEmpty(); }

bool ActorState::canAddAlias(const QString &value) const {
  return !value.trimmed().isEmpty();
}

bool ActorState::canRemoveSelectedAlias() const {
  return aliasIndex_ >= 0 && aliasIndex_ < aliases_.size();
}

bool ActorState::isAliasSelected(int index) const {
  return aliasIndex_ == index;
}

bool ActorState::isContractSelected(const QString &contractId) const {
  const QString nextId = contractId.trimmed();
  if (nextId.isEmpty()) {
    return false;
  }
  for (const auto &value : selectedContractIds_) {
    if (value.toString().trimmed() == nextId) {
      return true;
    }
  }
  return false;
}

void ActorState::clearFormState() {
  name_.clear();
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
  selectedContractIds_.clear();
  emit changed();
}

void ActorState::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ActorState::enterCreateMode() {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedActorId(QString());
  }
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void ActorState::addAlias(const QString &value) {
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
  emit changed();
}

void ActorState::removeAlias(int index) {
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
  emit changed();
}

void ActorState::selectAlias(int index) { setAliasIndex(index); }

void ActorState::selectActor(const QString &id) {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedActorId(id.trimmed());
  }
}

void ActorState::requestRemoveSelectedAlias() {
  if (!canRemoveSelectedAlias()) {
    return;
  }
  removeAlias(aliasIndex_);
}

void ActorState::setContractSelected(const QString &contractId, bool selected) {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QString normalizedId = contractId.trimmed();
  if (normalizedId.isEmpty()) {
    return;
  }
  const QVariantList next = selected ? workspace_->session()->addUniqueTrimmed(
                                           selectedContractIds_, normalizedId)
                                     : workspace_->session()->removeString(
                                           selectedContractIds_, normalizedId);
  if (next == selectedContractIds_) {
    return;
  }
  selectedContractIds_ = next;
  emit changed();
}

void ActorState::previous() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->actorRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), -1, rows.size() - 1);
  workspace_->selection()->setSelectedActorId(nextId);
}

void ActorState::next() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->actorRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), 1, 0);
  workspace_->selection()->setSelectedActorId(nextId);
}

QString ActorState::submit() {
  if (!workspace_) {
    return {};
  }
  const QStringList aliasValues = toQStringList(aliases_);
  const QStringList contractIds = toQStringList(selectedContractIds_);
  const QString id = workspace_->saveActor(isEdit() ? currentId() : QString(),
                                           name_, aliasValues, contractIds);
  if (workspace_->selection() && !id.isEmpty()) {
    workspace_->selection()->setSelectedActorId(id);
  }
  captureSavedState();
  return id;
}

void ActorState::deleteCurrent() {
  if (!workspace_ || currentId().isEmpty()) {
    return;
  }
  const QString removedId = currentId();
  workspace_->deleteActor(removedId);
  if (!workspace_->selection() || !workspace_->session()) {
    return;
  }
  const QString nextId = workspace_->session()->deleteNextSelectionId(
      workspace_->actorRows(), removedId, 0, QStringLiteral("id"));
  workspace_->selection()->setSelectedActorId(nextId);
}

void ActorState::bindSignals() {
  if (!workspace_) {
    return;
  }

  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() { reloadFromSelection(true); });
  QObject::connect(workspace_->selection(),
                   &SessionSelection::selectedActorIdChanged, this,
                   [this]() { reloadFromSelection(false); });
  QObject::connect(workspace_->selectedActor(), &ActorSelection::changed, this,
                   [this]() { reloadFromSelection(true); });
}

void ActorState::applyFormState(const QVariantMap &state) {
  const QVariantMap next = state;
  name_ = next.value(QStringLiteral("name")).toString();
  aliases_ = next.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = next.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      next.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
  selectedContractIds_ = next.value(QStringLiteral("selectedIds")).toList();
  emit changed();
}

void ActorState::captureSavedState() {
  savedName_ = name_;
  savedAliases_ = aliases_;
  savedSelectedContractIds_ = selectedContractIds_;
  emit changed();
}

void ActorState::reloadFromSelection(bool forceReload) {
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

  ActorSelection *current = workspace_->selectedActor();
  if (!current) {
    clearFormState();
    captureSavedState();
    return;
  }

  const QVariantMap state = workspace_->session()->basicFormState(
      current->name(), toVariantList(current->aliases()),
      toVariantList(current->contractIds()));
  applyFormState(state);
  captureSavedState();
}

} // namespace ui
