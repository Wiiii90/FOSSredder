/**
 * @file ui/src/workspace/WorkspaceSelection.cpp
 * @brief Implements synchronized workspace selection tracking.
 */

#include "ui/workspace/WorkspaceSelection.h"

namespace ui {

namespace {

struct SelectionIdsSnapshot {
  QString actorId;
  QString propertyId;
  QString contractId;
  QString statementId;
  QString transactionId;
  QString analysisId;
  QString annualId;
};

SelectionIdsSnapshot captureSelectionIds(const WorkspaceSelection& selection) {
  return {selection.selectedActorId(),       selection.selectedPropertyId(),
          selection.selectedContractId(),    selection.selectedStatementId(),
          selection.selectedTransactionId(), selection.selectedAnalysisId(),
          selection.selectedAnnualId()};
}

void emitSelectionChanges(WorkspaceSelection& selection,
                          const SelectionIdsSnapshot& before) {
  if (selection.selectedActorId() != before.actorId)
    emit selection.selectedActorIdChanged();
  if (selection.selectedPropertyId() != before.propertyId)
    emit selection.selectedPropertyIdChanged();
  if (selection.selectedContractId() != before.contractId)
    emit selection.selectedContractIdChanged();
  if (selection.selectedStatementId() != before.statementId)
    emit selection.selectedStatementIdChanged();
  if (selection.selectedTransactionId() != before.transactionId)
    emit selection.selectedTransactionIdChanged();
  if (selection.selectedAnalysisId() != before.analysisId)
    emit selection.selectedAnalysisIdChanged();
  if (selection.selectedAnnualId() != before.annualId)
    emit selection.selectedAnnualIdChanged();
}

bool updateSelectedId(QString& targetId, const QString& newId) {
  const QString normalized = newId.trimmed();
  if (targetId == normalized) {
    return false;
  }
  targetId = normalized;
  return true;
}

} // namespace

WorkspaceSelection::WorkspaceSelection(WorkspaceStore& store,
                                       WorkspaceSelectors& selectors,
                                       QObject* parent)
    : QObject(parent), store_(store), selectors_(selectors) {
  connect(&store_, &WorkspaceStore::dataRevisionChanged, this,
          &WorkspaceSelection::refreshSelections);
}

QString WorkspaceSelection::selectedActorId() const {
  return selectedActorId_;
}
QString WorkspaceSelection::selectedPropertyId() const {
  return selectedPropertyId_;
}
QString WorkspaceSelection::selectedContractId() const {
  return selectedContractId_;
}
QString WorkspaceSelection::selectedStatementId() const {
  return selectedStatementId_;
}
QString WorkspaceSelection::selectedTransactionId() const {
  return selectedTransactionId_;
}
QString WorkspaceSelection::selectedAnalysisId() const {
  return selectedAnalysisId_;
}
QString WorkspaceSelection::selectedAnnualId() const {
  return selectedAnnualId_;
}

void WorkspaceSelection::setSelectedActorId(const QString& id) {
  if (updateSelectedId(selectedActorId_, id)) {
    validateSelections();
    emit selectedActorIdChanged();
  }
}

void WorkspaceSelection::setSelectedPropertyId(const QString& id) {
  if (updateSelectedId(selectedPropertyId_, id)) {
    validateSelections();
    emit selectedPropertyIdChanged();
  }
}

void WorkspaceSelection::setSelectedContractId(const QString& id) {
  if (updateSelectedId(selectedContractId_, id)) {
    validateSelections();
    emit selectedContractIdChanged();
  }
}

void WorkspaceSelection::setSelectedStatementId(const QString& id) {
  if (updateSelectedId(selectedStatementId_, id)) {
    validateSelections();
    emit selectedStatementIdChanged();
  }
}

void WorkspaceSelection::setSelectedAnalysisId(const QString& id) {
  if (updateSelectedId(selectedAnalysisId_, id)) {
    validateSelections();
    emit selectedAnalysisIdChanged();
  }
}

void WorkspaceSelection::setSelectedAnnualId(const QString& id) {
  if (updateSelectedId(selectedAnnualId_, id)) {
    validateSelections();
    emit selectedAnnualIdChanged();
  }
}

void WorkspaceSelection::selectActor(const QString& id) {
  setSelectedActorId(id);
}

void WorkspaceSelection::selectProperty(const QString& id) {
  setSelectedPropertyId(id);
}

void WorkspaceSelection::selectContract(const QString& id) {
  setSelectedContractId(id);
}

void WorkspaceSelection::selectStatement(const QString& id) {
  setSelectedStatementId(id);
}

void WorkspaceSelection::selectTransaction(const QString& statementId,
                                           const QString& id) {
  const auto before = captureSelectionIds(*this);
  selectedStatementId_ = statementId.trimmed();
  selectedTransactionId_ = id.trimmed();
  validateSelections();
  emitSelectionChanges(*this, before);
}

void WorkspaceSelection::selectAnalysis(const QString& id) {
  setSelectedAnalysisId(id);
}

void WorkspaceSelection::selectAnnual(const QString& id) {
  setSelectedAnnualId(id);
}

void WorkspaceSelection::validateSelections() {
  if (!selectedActorId_.isEmpty() && !selectors_.hasActorId(selectedActorId_)) {
    selectedActorId_.clear();
  }
  if (!selectedPropertyId_.isEmpty() &&
      !selectors_.hasPropertyId(selectedPropertyId_)) {
    selectedPropertyId_.clear();
  }
  if (!selectedContractId_.isEmpty() &&
      !selectors_.hasContractId(selectedContractId_)) {
    selectedContractId_.clear();
  }
  if (!selectedStatementId_.isEmpty() &&
      !selectors_.hasStatementId(selectedStatementId_)) {
    selectedStatementId_.clear();
    selectedTransactionId_.clear();
  }
  if (!selectedTransactionId_.isEmpty() &&
      !selectors_.hasTransactionId(selectedTransactionId_)) {
    selectedTransactionId_.clear();
  }
  if (!selectedAnalysisId_.isEmpty() &&
      !selectors_.hasAnalysisId(selectedAnalysisId_)) {
    selectedAnalysisId_.clear();
  }
  if (!selectedAnnualId_.isEmpty() &&
      !selectors_.hasAnnualId(selectedAnnualId_)) {
    selectedAnnualId_.clear();
  }
}

void WorkspaceSelection::loadFromState() {
  refreshSelections();
}

void WorkspaceSelection::refreshSelections() {
  const auto before = captureSelectionIds(*this);
  validateSelections();
  emitSelectionChanges(*this, before);
}

} // namespace ui
