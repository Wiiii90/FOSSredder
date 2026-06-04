/**
 * @file ui/src/workspace/WorkspaceFilterState.cpp
 * @brief Implements cached transaction filters owned by WorkspaceCache.
 */

#include "ui/workspace/WorkspaceFilterState.h"

#include <functional>

#include <QObject>

#include "ui/workspace/TransactionFilterModel.h"
#include "ui/workspace/TransactionListModel.h"

namespace ui {

namespace {

TransactionFilter *
ensureFilter(QHash<QString, TransactionFilter *> &filters, const QString &key,
             TransactionListModel &sourceModel, QObject *owner,
             const std::function<void(TransactionFilter &)> &configure) {
  if (key.isEmpty() || !owner) {
    return nullptr;
  }
  if (filters.contains(key)) {
    return filters.value(key);
  }

  auto *proxy = new TransactionFilter(owner);
  proxy->setSourceModel(&sourceModel);
  configure(*proxy);
  filters.insert(key, proxy);
  return proxy;
}

void deleteFilters(QHash<QString, TransactionFilter *> &filters) {
  for (auto *filter : filters) {
    if (filter) {
      filter->deleteLater();
    }
  }
  filters.clear();
}

void removeFilter(QHash<QString, TransactionFilter *> &filters,
                  const QString &key) {
  if (key.isEmpty()) {
    return;
  }
  if (auto *filter = filters.value(key, nullptr)) {
    filter->deleteLater();
  }
  filters.remove(key);
}

} // namespace

WorkspaceFilterState::WorkspaceFilterState(QObject *owner) : owner_(owner) {}

TransactionFilter *
WorkspaceFilterState::statementTransactions(const QString &statementId,
                                            TransactionListModel &sourceModel) {
  return ensureFilter(statementFilters_, statementId, sourceModel, owner_,
                      [&statementId](TransactionFilter &filter) {
                        filter.setStatementId(statementId);
                      });
}

TransactionFilter *
WorkspaceFilterState::propertyTransactions(const QString &propertyId,
                                           TransactionListModel &sourceModel) {
  return ensureFilter(propertyFilters_, propertyId, sourceModel, owner_,
                      [&propertyId](TransactionFilter &filter) {
                        filter.setPropertyId(propertyId);
                      });
}

void WorkspaceFilterState::clear() {
  deleteFilters(statementFilters_);
  deleteFilters(propertyFilters_);
}

void WorkspaceFilterState::removeStatement(const QString &statementId) {
  removeFilter(statementFilters_, statementId);
}

void WorkspaceFilterState::removeProperty(const QString &propertyId) {
  removeFilter(propertyFilters_, propertyId);
}

} // namespace ui
