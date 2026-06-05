/**
 * @file ui/include/ui/viewmodels/BookingViewModel.h
 * @brief Declares the QML API for the Booking view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace ui {

class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;

/**
 * @brief Owns Booking view form state and delegates statement and transaction
 * CRUD to workspace.
 */
class BookingViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isCreateMode READ isCreateMode NOTIFY changed)
  Q_PROPERTY(QString statementName READ statementName WRITE setStatementName
                 NOTIFY changed)
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
  /**
   * @brief Creates the booking view model backed by workspace roles.
   * @param parent Optional Qt parent object.
   */
  explicit BookingViewModel(WorkspaceStore *store, WorkspaceCommands *commands,
                            WorkspaceSelection *selection,
                            WorkspaceSelectors *selectors,
                            QObject *parent = nullptr);

  /**
   * @brief Returns whether the form currently creates a new statement.
   * @return True when no existing statement is selected.
   */
  bool isCreateMode() const;

  /**
   * @brief Returns the statement name shown in the form.
   * @return Current create or edit statement name.
   */
  QString statementName() const;

  /**
   * @brief Updates the statement name shown in the form.
   * @param value New statement name.
   */
  void setStatementName(const QString &value);

  /**
   * @brief Returns the current transaction name field.
   * @return Current transaction name.
   */
  QString transactionName() const;

  /**
   * @brief Updates the current transaction name field.
   * @param value New transaction name.
   */
  void setTransactionName(const QString &value);

  /**
   * @brief Returns the current transaction booking date field.
   * @return Current booking date text.
   */
  QString transactionBookingDate() const;

  /**
   * @brief Updates the current transaction booking date field.
   * @param value New booking date text.
   */
  void setTransactionBookingDate(const QString &value);

  /**
   * @brief Returns the current transaction valuta field.
   * @return Current valuta text.
   */
  QString transactionValuta() const;

  /**
   * @brief Updates the current transaction valuta field.
   * @param value New valuta text.
   */
  void setTransactionValuta(const QString &value);

  /**
   * @brief Returns the current amount field as text.
   * @return Amount text as entered or loaded for the current transaction.
   */
  QString transactionAmountText() const;

  /**
   * @brief Updates the current amount field.
   * @param value New amount text.
   */
  void setTransactionAmountText(const QString &value);

  /**
   * @brief Returns the selected transaction status option index.
   * @return Index into transactionStatusOptions().
   */
  int transactionStatusIndex() const;

  /**
   * @brief Updates the current transaction status from an option index.
   * @param index Index into transactionStatusOptions().
   */
  void setTransactionStatusIndex(int index);

  /**
   * @brief Returns whether the current transaction is allocatable.
   * @return True when the current transaction has the allocatable flag set.
   */
  bool transactionAllocatable() const;

  /**
   * @brief Updates the current transaction allocatable flag.
   * @param value New allocatable flag.
   */
  void setTransactionAllocatable(bool value);

  /**
   * @brief Returns property rows for the property checkbox list.
   * @return Workspace property rows.
   */
  QVariantList propertyRows() const;

  /**
   * @brief Returns property ids selected for the current transaction.
   * @return Selected property id list.
   */
  QVariantList selectedPropertyIds() const;

  /**
   * @brief Returns selectable transaction status options.
   * @return Status option rows with display labels and values.
   */
  QVariantList transactionStatusOptions() const;

  /**
   * @brief Returns statements shown in the booking sidebar.
   * @return Statement rows including transaction children.
   */
  QVariantList statementRows() const;

  /**
   * @brief Returns the selected statement id.
   * @return Workspace selected statement id or an empty string.
   */
  QString selectedStatementId() const;

  /**
   * @brief Returns the selected transaction id.
   * @return Workspace selected transaction id or an empty string.
   */
  QString selectedTransactionId() const;

  /**
   * @brief Returns actor rows prepared for the actor dropdown.
   * @return Actor display rows including the empty option.
   */
  QVariantList actorDisplayRows() const;

  /**
   * @brief Returns contract rows prepared for the contract dropdown.
   * @return Contract display rows including the empty option.
   */
  QVariantList contractDisplayRows() const;

  /**
   * @brief Returns the selected actor dropdown index.
   * @return Index into actorDisplayRows().
   */
  int selectedActorIndex() const;

  /**
   * @brief Returns the selected contract dropdown index.
   * @return Index into contractDisplayRows().
   */
  int selectedContractIndex() const;

  /**
   * @brief Returns the transaction position text shown in the form.
   * @return Localized transaction counter text.
   */
  QString transactionInfoText() const;

  /**
   * @brief Returns whether statement rows are available.
   * @return True when the sidebar has statement rows.
   */
  bool hasStatements() const;

  /**
   * @brief Returns whether the current statement has multiple transactions.
   * @return True when previous/next transaction navigation is useful.
   */
  bool hasMultipleTransactions() const;

  /**
   * @brief Returns whether the create form can be submitted.
   * @return True when the local form state passes the quick UI checks.
   */
  bool canCreate() const;

  /**
   * @brief Returns whether the selected statement or transaction changed.
   * @return True when the edit form can be updated.
   */
  bool canUpdate() const;

  /**
   * @brief Returns whether a transaction can be added to the current statement.
   * @return True in create mode or when a statement is selected.
   */
  bool canAddTransaction() const;

  /**
   * @brief Returns whether the current transaction can be deleted.
   * @return True when deleting would leave at least one transaction.
   */
  bool canDeleteTransaction() const;

  /**
   * @brief Clears the create form and creates a single empty transaction state.
   */
  Q_INVOKABLE void resetCreateState();

  /**
   * @brief Selects the previous statement or the create form.
   */
  Q_INVOKABLE void previousStatement();

  /**
   * @brief Selects the next statement or the create form.
   */
  Q_INVOKABLE void nextStatement();

  /**
   * @brief Selects the previous transaction in the current statement.
   */
  Q_INVOKABLE void previousTransaction();

  /**
   * @brief Selects the next transaction in the current statement.
   */
  Q_INVOKABLE void nextTransaction();

  /**
   * @brief Adds a transaction after the current transaction.
   */
  Q_INVOKABLE void addTransactionAfterCurrent();

  /**
   * @brief Deletes the current transaction when at least one remains.
   */
  Q_INVOKABLE void deleteCurrentTransaction();

  /**
   * @brief Updates whether a property is selected for the current transaction.
   * @param propertyId Property id from the checkbox row.
   * @param selected True to select the property, false to remove it.
   */
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
                                       bool selected);

  /**
   * @brief Selects an actor from the actor dropdown.
   * @param index Index into actorDisplayRows().
   */
  Q_INVOKABLE void selectActorIndex(int index);

  /**
   * @brief Selects a contract from the contract dropdown.
   * @param index Index into contractDisplayRows().
   */
  Q_INVOKABLE void selectContractIndex(int index);

  /**
   * @brief Selects a statement from the sidebar.
   * @param statementId Statement id to select.
   */
  Q_INVOKABLE void selectStatement(const QString &statementId);

  /**
   * @brief Selects a transaction from the sidebar.
   * @param statementId Parent statement id.
   * @param transactionId Transaction id to select.
   */
  Q_INVOKABLE void selectTransaction(const QString &statementId,
                                     const QString &transactionId);

  /**
   * @brief Creates a statement and its transaction form states through workspace.
   * @return Created statement id or an empty string when creation failed.
   */
  Q_INVOKABLE QString submit();

  /**
   * @brief Updates the selected statement and transaction through workspace.
   */
  Q_INVOKABLE void updateCurrent();

  /**
   * @brief Deletes the selected statement through workspace.
   */
  Q_INVOKABLE void deleteCurrentStatement();

signals:
  void changed();

private:
  /**
   * @brief Creates an empty transaction form state.
   * @return Empty transaction map with all expected form keys.
   */
  QVariantMap emptyTransaction() const;

  /**
   * @brief Normalizes a transaction form map to the expected key/value shape.
   * @param tx Raw transaction form map.
   * @return Normalized transaction form map.
   */
  QVariantMap normalizeTransaction(const QVariantMap &tx) const;

  /**
   * @brief Returns the current transaction form state.
   * @return Current create form state or edit transaction data.
   */
  QVariantMap transactionData() const;

  /**
   * @brief Replaces the current transaction form state.
   * @param value New transaction form state.
   */
  void setTransactionData(const QVariantMap &value);

  /**
   * @brief Reads a field from the current transaction form state.
   * @param key Field key.
   * @return Current field value.
   */
  QVariant transactionField(const QString &key) const;

  /**
   * @brief Updates one field in the current transaction form state.
   * @param key Field key.
   * @param value New field value.
   */
  void setTransactionField(const QString &key, const QVariant &value);

  /**
   * @brief Applies dependent actor, contract, and property changes to the form.
   * @param changes Partial transaction form changes.
   */
  void applyTransactionFormChange(const QVariantMap &changes);

  /**
   * @brief Resolves the selected index for an id in a row list.
   * @param rows Rows with id fields.
   * @param id Selected id.
   * @return Matching row index or zero for the empty option.
   */
  int selectedIndexFor(const QVariantList &rows, const QString &id) const;

  /**
   * @brief Builds the current edit transaction selection state.
   * @return Selection state containing rows, order ids, index, and id.
   */
  QVariantMap editTransactionState() const;

  /**
   * @brief Loads a transaction row by id and adapts it to form state.
   * @param txId Transaction id.
   * @return Transaction form state or an empty transaction.
   */
  QVariantMap transactionById(const QString &txId) const;

  /**
   * @brief Returns transaction rows for a statement.
   * @param statementId Statement id whose transactions should be returned.
   * @return Transaction rows belonging to the statement.
   */
  QVariantList statementTransactionRows(const QString &statementId) const;

  /**
   * @brief Normalizes transaction form state for dirty-state comparison.
   * @param data Transaction form state.
   * @return Normalized transaction form state.
   */
  QVariantMap normalizedTransactionState(const QVariantMap &data) const;

  /**
   * @brief Captures the current edit state for dirty-state comparison.
   */
  void captureEditState();

  /**
   * @brief Remembers the selected transaction for the current statement.
   */
  void rememberSelectedTransaction();

  /**
   * @brief Resolves the transaction id to select for a statement.
   * @param statementId Statement id.
   * @return Remembered transaction id or the first transaction id.
   */
  QString transactionIdForStatement(const QString &statementId) const;

  /**
   * @brief Ensures the workspace selection points at a valid transaction.
   * @return True when the current selection is already valid.
   */
  bool ensureSelectedTransactionForStatement();

  /**
   * @brief Synchronizes the current create form state from the state list.
   */
  void syncCurrentCreateTransaction();

  /**
   * @brief Synchronizes edit form state from the workspace selection.
   */
  void syncEditState();

  /**
   * @brief Connects workspace change signals to local state synchronization.
   */
  void bindSignals();

  WorkspaceStore *store_ = nullptr;
  WorkspaceCommands *commands_ = nullptr;
  WorkspaceSelection *selection_ = nullptr;
  WorkspaceSelectors *selectors_ = nullptr;
  QString createStatementName_;
  QVariantList createTransactionStates_;
  int createTransactionIndex_ = 0;
  QString editStatementName_;
  QVariantMap editTransactionData_;
  int editTransactionIndex_ = -1;
  QVariantList editTransactionOrderIds_;
  QString savedEditStatementName_;
  QVariantMap savedEditTransactionState_;
  QVariantMap currentTransactionFormState_;
  QVariantMap lastTransactionIdByStatementId_;
};

} // namespace ui
