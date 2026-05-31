/**
 * @file ui/src/state/session/PropertyState.cpp
 * @brief Implements the selection-aware property UI state wrapper.
 */

#include "ui/state/session/PropertyState.h"

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

PropertyState::PropertyState(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  bindSignals();
  reloadFromSelection(true);
}

QString PropertyState::currentId() const {
  return workspace_ ? workspace_->selectedPropertyId() : QString();
}

QString PropertyState::name() const { return name_; }

void PropertyState::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  dirty_ = true;
  emit changed();
}

QVariantList PropertyState::aliases() const { return aliases_; }

void PropertyState::setAliases(const QVariantList &value) {
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

QString PropertyState::aliasInputText() const { return aliasInputText_; }

void PropertyState::setAliasInputText(const QString &value) {
  if (aliasInputText_ == value) {
    return;
  }
  aliasInputText_ = value;
  emit changed();
}

int PropertyState::aliasIndex() const { return aliasIndex_; }

void PropertyState::setAliasIndex(int value) {
  if (aliasIndex_ == value) {
    return;
  }
  aliasIndex_ = value;
  emit changed();
}

QVariantList PropertyState::selectedContractIds() const {
  return selectedContractIds_;
}

void PropertyState::setSelectedContractIds(const QVariantList &value) {
  if (selectedContractIds_ == value) {
    return;
  }
  selectedContractIds_ = value;
  dirty_ = true;
  emit changed();
}

QVariantList PropertyState::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList{};
}

QVariantList PropertyState::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList{};
}

bool PropertyState::isEdit() const { return !currentId().isEmpty(); }

bool PropertyState::hasChanges() const {
  if (!isEdit()) {
    return !name_.trimmed().isEmpty();
  }
  if (!workspace_ || !workspace_->session()) {
    return false;
  }
  return dirty_ || workspace_->session()->formStateChanged(
                       savedName_, savedAliases_, savedSelectedContractIds_,
                       name_, aliases_, selectedContractIds_);
}

bool PropertyState::canSubmit() const { return !name_.trimmed().isEmpty(); }

bool PropertyState::canAddAlias(const QString &value) const {
  return !value.trimmed().isEmpty();
}

bool PropertyState::canRemoveSelectedAlias() const {
  return aliasIndex_ >= 0 && aliasIndex_ < aliases_.size();
}

bool PropertyState::isAliasSelected(int index) const {
  return aliasIndex_ == index;
}

bool PropertyState::isContractSelected(const QString &contractId) const {
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

void PropertyState::clearFormState() {
  name_.clear();
  aliases_.clear();
  aliasInputText_.clear();
  aliasIndex_ = -1;
  selectedContractIds_.clear();
  dirty_ = false;
  emit changed();
}

void PropertyState::clear() {
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void PropertyState::enterCreateMode() {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedPropertyId(QString());
  }
  currentOwnerId_.clear();
  clearFormState();
  captureSavedState();
}

void PropertyState::addAlias(const QString &value) {
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

void PropertyState::removeAlias(int index) {
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

void PropertyState::selectAlias(int index) { setAliasIndex(index); }

void PropertyState::selectProperty(const QString &id) {
  if (workspace_ && workspace_->selection()) {
    workspace_->selection()->setSelectedPropertyId(id.trimmed());
  }
}

void PropertyState::requestRemoveSelectedAlias() {
  if (!canRemoveSelectedAlias()) {
    return;
  }
  removeAlias(aliasIndex_);
}

void PropertyState::setContractSelected(const QString &contractId,
                                        bool selected) {
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
  dirty_ = true;
  emit changed();
}

void PropertyState::previous() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->propertyRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), -1, rows.size() - 1);
  workspace_->selection()->setSelectedPropertyId(nextId);
}

void PropertyState::next() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = workspace_->propertyRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString nextId = workspace_->session()->navigatedSelectionId(
      rows, isEdit() ? currentId() : QString(), 1, 0);
  workspace_->selection()->setSelectedPropertyId(nextId);
}

QString PropertyState::submit() {
  if (!workspace_) {
    return {};
  }
  const QStringList aliasValues = toQStringList(aliases_);
  const QStringList contractIds = toQStringList(selectedContractIds_);
  const QString id = workspace_->saveProperty(
      isEdit() ? currentId() : QString(), name_, aliasValues, contractIds);
  if (workspace_->selection() && !id.isEmpty()) {
    workspace_->selection()->setSelectedPropertyId(id);
  }
  captureSavedState();
  return id;
}

void PropertyState::deleteCurrent() {
  if (!workspace_ || currentId().isEmpty()) {
    return;
  }
  const QString removedId = currentId();
  workspace_->deleteProperty(removedId);
  if (!workspace_->selection() || !workspace_->session()) {
    return;
  }
  const QString nextId = workspace_->session()->deleteNextSelectionId(
      workspace_->propertyRows(), removedId, 0, QStringLiteral("id"));
  workspace_->selection()->setSelectedPropertyId(nextId);
}

void PropertyState::bindSignals() {
  if (!workspace_) {
    return;
  }

  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
                   [this]() { reloadFromSelection(true); });
  QObject::connect(workspace_->selection(),
                   &SessionSelection::selectedPropertyIdChanged, this,
                   [this]() { reloadFromSelection(false); });
  QObject::connect(workspace_->selectedProperty(), &PropertySelection::changed,
                   this, [this]() { reloadFromSelection(true); });
}

void PropertyState::applyFormState(const QVariantMap &state) {
  const QVariantMap next = state;
  name_ = next.value(QStringLiteral("name")).toString();
  aliases_ = next.value(QStringLiteral("aliases")).toList();
  aliasInputText_ = next.value(QStringLiteral("aliasInputText")).toString();
  aliasIndex_ =
      next.value(QStringLiteral("aliasIndex"), aliases_.isEmpty() ? -1 : 0)
          .toInt();
  selectedContractIds_ = next.value(QStringLiteral("selectedIds")).toList();
  dirty_ = false;
  emit changed();
}

void PropertyState::captureSavedState() {
  savedName_ = name_;
  savedAliases_ = aliases_;
  savedSelectedContractIds_ = selectedContractIds_;
  dirty_ = false;
  emit changed();
}

void PropertyState::reloadFromSelection(bool forceReload) {
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

  PropertySelection *current = workspace_->selectedProperty();
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
