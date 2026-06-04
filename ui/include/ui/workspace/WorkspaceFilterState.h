/**
 * @file ui/include/ui/workspace/WorkspaceFilterState.h
 * @brief Declares cached transaction filters owned by WorkspaceCache.
 */

#pragma once

#include <QHash>
#include <QString>

class QObject;

namespace ui {

class TransactionListModel;
class TransactionFilter;

class WorkspaceFilterState {
public:
  explicit WorkspaceFilterState(QObject *owner = nullptr);

  TransactionFilter *statementTransactions(const QString &statementId,
                                           TransactionListModel &sourceModel);
  TransactionFilter *propertyTransactions(const QString &propertyId,
                                          TransactionListModel &sourceModel);

  void clear();
  void removeStatement(const QString &statementId);
  void removeProperty(const QString &propertyId);

private:
  QObject *owner_ = nullptr;
  QHash<QString, TransactionFilter *> statementFilters_;
  QHash<QString, TransactionFilter *> propertyFilters_;
};

} // namespace ui
