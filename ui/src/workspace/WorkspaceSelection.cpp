/**
 * @file ui/src/workspace/WorkspaceSelection.cpp
 * @brief Implements synchronized workspace selection tracking.
 */

#include "ui/workspace/WorkspaceSelection.h"

#include <cstddef>

#include <QAbstractItemModel>

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

template <typename Item> bool isMissingItem(const Item &item) {
  if constexpr (requires { !item; }) {
    return !item;
  } else {
    return false;
  }
}

template <typename Items, typename FindRow>
void validateSelectedId(QString &selectedId, const Items &items,
                        FindRow &&findRow) {
  if (selectedId.isEmpty()) {
    return;
  }

  const int row = findRow(selectedId);
  if (row < 0) {
    selectedId.clear();
    return;
  }

  if (row >= static_cast<int>(items.size())) {
    selectedId.clear();
    return;
  }

  if (isMissingItem(items[static_cast<std::size_t>(row)])) {
    selectedId.clear();
  }
}

template <typename RefreshFn>
bool updateSelectedId(QString &targetId, const QString &newId,
                      RefreshFn &&refresh) {
  if (targetId == newId) {
    return false;
  }
  targetId = newId;
  refresh();
  return true;
}

SelectionIdsSnapshot captureSelectionIds(const WorkspaceSelection &selection) {
  return {selection.selectedActorId(),      selection.selectedPropertyId(),
          selection.selectedContractId(),   selection.selectedStatementId(),
          selection.selectedTransactionId(), selection.selectedAnalysisId(),
          selection.selectedAnnualId()};
}

void emitSelectionChanges(WorkspaceSelection &selection,
                          const SelectionIdsSnapshot &before) {
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

template <typename RefreshFn>
void bindSelectionRefreshesForModel(QObject *owner, RefreshFn &&refresh,
                                    QAbstractItemModel &model) {
  QObject::connect(&model, &QAbstractItemModel::modelReset, owner, refresh);
  QObject::connect(&model, &QAbstractItemModel::rowsInserted, owner, refresh);
  QObject::connect(&model, &QAbstractItemModel::rowsRemoved, owner, refresh);
  QObject::connect(&model, &QAbstractItemModel::rowsMoved, owner, refresh);
  QObject::connect(&model, &QAbstractItemModel::layoutChanged, owner, refresh);
  QObject::connect(&model, &QAbstractItemModel::dataChanged, owner,
                   [refresh](const QModelIndex &, const QModelIndex &,
                             const QVector<int> &) { refresh(); });
}

} // namespace

WorkspaceSelection::WorkspaceSelection(WorkspaceCacheModels &models,
                                       QObject *parent)
    : QObject(parent), models_(models) {
  bindModelSignals();
}

QString WorkspaceSelection::selectedActorId() const { return selectedActorId_; }
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

void WorkspaceSelection::setSelectedActorId(const QString &id) {
  if (updateSelectedId(selectedActorId_, id, [this]() {
        validateSelectedId(selectedActorId_, models_.actorSnapshots(),
                           [this](const QString &value) {
                             return models_.findActorRowById(value);
                           });
      }))
    emit selectedActorIdChanged();
}

void WorkspaceSelection::setSelectedPropertyId(const QString &id) {
  if (updateSelectedId(selectedPropertyId_, id, [this]() {
        validateSelectedId(selectedPropertyId_, models_.propertySnapshots(),
                           [this](const QString &value) {
                             return models_.findPropertyRowById(value);
                           });
      }))
    emit selectedPropertyIdChanged();
}

void WorkspaceSelection::setSelectedContractId(const QString &id) {
  if (updateSelectedId(selectedContractId_, id, [this]() {
        validateSelectedId(selectedContractId_, models_.contractSnapshots(),
                           [this](const QString &value) {
                             return models_.findContractRowById(value);
                           });
      }))
    emit selectedContractIdChanged();
}

void WorkspaceSelection::setSelectedStatementId(const QString &id) {
  if (updateSelectedId(selectedStatementId_, id, [this]() {
        validateSelectedId(selectedStatementId_, models_.statements().statements(),
                           [this](const QString &value) {
                             return models_.statements().findRowById(value);
                           });
      }))
    emit selectedStatementIdChanged();
}

void WorkspaceSelection::setSelectedTransactionId(const QString &id) {
  if (updateSelectedId(selectedTransactionId_, id, [this]() {
        validateSelectedId(selectedTransactionId_,
                           models_.transactions().transactions(),
                           [this](const QString &value) {
                             return models_.transactions().findRowById(value);
                           });
      }))
    emit selectedTransactionIdChanged();
}

void WorkspaceSelection::setSelectedAnalysisId(const QString &id) {
  if (!updateSelectedId(selectedAnalysisId_, id, [this]() {
        validateSelectedId(selectedAnalysisId_, models_.analyses().analyses(),
                           [this](const QString &value) {
                             return models_.analyses().findRowById(value);
                           });
      }))
    return;
  emit selectedAnalysisIdChanged();
}

void WorkspaceSelection::setSelectedAnnualId(const QString &id) {
  if (updateSelectedId(selectedAnnualId_, id, [this]() {
        validateSelectedId(selectedAnnualId_, models_.annuals().annuals(),
                           [this](const QString &value) {
                             return models_.annuals().findRowById(value);
                           });
      }))
    emit selectedAnnualIdChanged();
}

void WorkspaceSelection::validateSelections() {
  validateSelectedId(selectedActorId_, models_.actorSnapshots(),
                     [this](const QString &value) {
                       return models_.findActorRowById(value);
                     });
  validateSelectedId(selectedPropertyId_, models_.propertySnapshots(),
                     [this](const QString &value) {
                       return models_.findPropertyRowById(value);
                     });
  validateSelectedId(selectedContractId_, models_.contractSnapshots(),
                     [this](const QString &value) {
                       return models_.findContractRowById(value);
                     });
  validateSelectedId(selectedStatementId_, models_.statements().statements(),
                     [this](const QString &value) {
                       return models_.statements().findRowById(value);
                     });
  validateSelectedId(selectedTransactionId_, models_.transactions().transactions(),
                     [this](const QString &value) {
                       return models_.transactions().findRowById(value);
                     });
  validateSelectedId(selectedAnalysisId_, models_.analyses().analyses(),
                     [this](const QString &value) {
                       return models_.analyses().findRowById(value);
                     });
  validateSelectedId(selectedAnnualId_, models_.annuals().annuals(),
                     [this](const QString &value) {
                       return models_.annuals().findRowById(value);
                     });
}

void WorkspaceSelection::loadFromState() {
  refreshSelections();
}

void WorkspaceSelection::bindModelSignals() {
  auto refresh = [this]() { refreshSelections(); };
  bindSelectionRefreshesForModel(this, refresh, models_.actorModel());
  bindSelectionRefreshesForModel(this, refresh, models_.propertyModel());
  bindSelectionRefreshesForModel(this, refresh, models_.contractModel());
  bindSelectionRefreshesForModel(this, refresh, models_.statements());
  bindSelectionRefreshesForModel(this, refresh, models_.transactions());
  bindSelectionRefreshesForModel(this, refresh, models_.analyses());
  bindSelectionRefreshesForModel(this, refresh, models_.annuals());
}

void WorkspaceSelection::refreshSelections() {
  const auto before = captureSelectionIds(*this);
  validateSelections();

  emitSelectionChanges(*this, before);
}

} // namespace ui
