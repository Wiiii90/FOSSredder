/**
 * @file ui/include/ui/workspace/TransactionFilterModel.h
 * @brief Declares transaction proxy filters owned by WorkspaceCache.
 */

#pragma once

#include <QSortFilterProxyModel>
#include <QString>

namespace ui {

class TransactionFilter : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString statementId READ statementId WRITE setStatementId NOTIFY
                 statementIdChanged)
  Q_PROPERTY(QString propertyId READ propertyId WRITE setPropertyId NOTIFY
                 propertyIdChanged)
  Q_PROPERTY(QString txType READ txType WRITE setTxType NOTIFY txTypeChanged)

public:
  explicit TransactionFilter(QObject *parent = nullptr);

  QString statementId() const { return statementId_; }
  void setStatementId(const QString &id);

  QString propertyId() const { return propertyId_; }
  void setPropertyId(const QString &id);

  QString txType() const { return txType_; }
  void setTxType(const QString &t);

signals:
  void statementIdChanged();
  void propertyIdChanged();
  void txTypeChanged();

protected:
  bool filterAcceptsRow(int sourceRow,
                        const QModelIndex &sourceParent) const override;

private:
  QString statementId_;
  QString propertyId_;
  QString txType_;
};

} // namespace ui
