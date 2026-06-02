/**
 * @file ui/include/ui/viewmodels/BookingViewModel.h
 * @brief Declares the UI statement booking state owned by the catalog workspace.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace ui {

class WorkspaceFacade;

class BookingViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isCreateMode READ isCreateMode NOTIFY changed)
  Q_PROPERTY(QString statementName READ statementName WRITE setStatementName
                 NOTIFY changed)
  Q_PROPERTY(QVariantMap transactionData READ transactionData WRITE
                 setTransactionData NOTIFY changed)
  Q_PROPERTY(QString transactionName READ transactionName WRITE
                 setTransactionName NOTIFY changed)
  Q_PROPERTY(QString transactionBookingDate READ transactionBookingDate WRITE
                 setTransactionBookingDate NOTIFY changed)
  Q_PROPERTY(QString transactionValuta READ transactionValuta WRITE
                 setTransactionValuta NOTIFY changed)
  Q_PROPERTY(QString transactionAmountText READ transactionAmountText WRITE
                 setTransactionAmountText NOTIFY changed)
  Q_PROPERTY(int transactionStatusIndex READ transactionStatusIndex WRITE
                 setTransactionStatusIndex NOTIFY changed)
  Q_PROPERTY(bool transactionAllocatable READ transactionAllocatable WRITE
                 setTransactionAllocatable NOTIFY changed)
  Q_PROPERTY(QVariantList actorRows READ actorRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractRows READ contractRows NOTIFY changed)
  Q_PROPERTY(QVariantList propertyRows READ propertyRows NOTIFY changed)
  Q_PROPERTY(
      QVariantList selectedPropertyIds READ selectedPropertyIds NOTIFY changed)
  Q_PROPERTY(QVariantList transactionStatusOptions READ transactionStatusOptions
                 NOTIFY changed)
  Q_PROPERTY(QVariantList statementRows READ statementRows NOTIFY changed)
  Q_PROPERTY(
      QString selectedStatementId READ selectedStatementId NOTIFY changed)
  Q_PROPERTY(
      QString selectedTransactionId READ selectedTransactionId NOTIFY changed)
  Q_PROPERTY(QVariantList actorDisplayRows READ actorDisplayRows NOTIFY changed)
  Q_PROPERTY(
      QVariantList contractDisplayRows READ contractDisplayRows NOTIFY changed)
  Q_PROPERTY(int selectedActorIndex READ selectedActorIndex NOTIFY changed)
  Q_PROPERTY(
      int selectedContractIndex READ selectedContractIndex NOTIFY changed)
  Q_PROPERTY(
      QString transactionInfoText READ transactionInfoText NOTIFY changed)
  Q_PROPERTY(bool hasStatements READ hasStatements NOTIFY changed)
  Q_PROPERTY(
      bool hasMultipleTransactions READ hasMultipleTransactions NOTIFY changed)
  Q_PROPERTY(bool canCreate READ canCreate NOTIFY changed)
  Q_PROPERTY(bool canUpdate READ canUpdate NOTIFY changed)
  Q_PROPERTY(bool canAddTransaction READ canAddTransaction NOTIFY changed)
  Q_PROPERTY(bool canDeleteTransaction READ canDeleteTransaction NOTIFY changed)

public:
  explicit BookingViewModel(WorkspaceFacade *workspace, QObject *parent = nullptr);

  bool isCreateMode() const;
  QString statementName() const;
  void setStatementName(const QString &value);
  QVariantMap transactionData() const;
  void setTransactionData(const QVariantMap &value);
  QString transactionName() const;
  void setTransactionName(const QString &value);
  QString transactionBookingDate() const;
  void setTransactionBookingDate(const QString &value);
  QString transactionValuta() const;
  void setTransactionValuta(const QString &value);
  QString transactionAmountText() const;
  void setTransactionAmountText(const QString &value);
  int transactionStatusIndex() const;
  void setTransactionStatusIndex(int index);
  bool transactionAllocatable() const;
  void setTransactionAllocatable(bool value);
  QVariantList actorRows() const;
  QVariantList contractRows() const;
  QVariantList propertyRows() const;
  QVariantList selectedPropertyIds() const;
  QVariantList transactionStatusOptions() const;
  QVariantList statementRows() const;
  QString selectedStatementId() const;
  QString selectedTransactionId() const;
  QVariantList actorDisplayRows() const;
  QVariantList contractDisplayRows() const;
  int selectedActorIndex() const;
  int selectedContractIndex() const;
  QString transactionInfoText() const;
  bool hasStatements() const;
  bool hasMultipleTransactions() const;
  bool canCreate() const;
  bool canUpdate() const;
  bool canAddTransaction() const;
  bool canDeleteTransaction() const;

  Q_INVOKABLE void resetCreateState();
  Q_INVOKABLE void previousStatement();
  Q_INVOKABLE void nextStatement();
  Q_INVOKABLE void previousTransaction();
  Q_INVOKABLE void nextTransaction();
  Q_INVOKABLE void addTransactionAfterCurrent();
  Q_INVOKABLE void deleteCurrentTransaction();
  Q_INVOKABLE bool isPropertySelected(const QString &propertyId) const;
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
                                       bool selected);
  Q_INVOKABLE void selectActorIndex(int index);
  Q_INVOKABLE void selectContractIndex(int index);
  Q_INVOKABLE QVariantList
  statementTransactionRows(const QString &statementId) const;
  Q_INVOKABLE void selectStatement(const QString &statementId);
  Q_INVOKABLE void selectTransaction(const QString &statementId,
                                     const QString &transactionId);
  Q_INVOKABLE QString submit();
  Q_INVOKABLE void updateCurrent();
  Q_INVOKABLE void deleteCurrentStatement();

signals:
  void changed();

private:
  QVariantMap emptyTransaction() const;
  QVariantMap normalizeTransaction(const QVariantMap &tx) const;
  QVariant value(const QString &key) const;
  void setTransactionValue(const QString &key, const QVariant &value);
  void applyTransactionDraftChange(const QVariantMap &changes);
  int selectedIndexFor(const QVariantList &rows, const QString &id) const;
  QVariantMap editTransactionState() const;
  QVariantMap transactionById(const QString &txId) const;
  QString transactionSnapshot(const QVariantMap &data) const;
  bool transactionDraftCanSubmit(const QVariantMap &draft) const;
  QVariantList submittableTransactionDrafts() const;
  void captureEditState();
  void rememberSelectedTransaction();
  QString transactionIdForStatement(const QString &statementId) const;
  bool ensureSelectedTransactionForStatement();
  void currentCreateTransaction();
  void syncEditState();
  void bindSignals();

  WorkspaceFacade *workspace_ = nullptr;
  QString createStatementName_;
  QVariantList createTransactions_;
  int createTransactionIndex_ = 0;
  QString editStatementName_;
  QVariantMap editTransactionData_;
  int editTransactionIndex_ = -1;
  QVariantList editTransactionOrderIds_;
  QString savedEditStatementName_;
  QString savedEditTransactionJson_ = QStringLiteral("{}");
  QVariantMap currentTransactionDraft_;
  QVariantMap lastTransactionIdByStatementId_;
};

} // namespace ui
