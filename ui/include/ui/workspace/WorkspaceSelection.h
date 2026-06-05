/**
 * @file ui/include/ui/workspace/WorkspaceSelection.h
 * @brief Declares the workspace selection state synchronized with the store.
 */

#pragma once

#include <QObject>
#include <QString>

#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

/**
 * @brief Tracks the currently selected workspace entities.
 */
class WorkspaceSelection : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Creates the selection state for the supplied workspace roles.
   * @param store Workspace store used for selection validation.
   * @param selectors Workspace selectors used for existence checks.
   * @param parent Optional Qt parent.
   */
  explicit WorkspaceSelection(WorkspaceStore &store, WorkspaceSelectors &selectors,
                              QObject *parent = nullptr);

  /** @brief Refreshes selection objects after the underlying store reloaded. */
  void loadFromState();

  /** @brief Returns the selected actor id. @return Actor id or empty. */
  QString selectedActorId() const;
  /** @brief Returns the selected property id. @return Property id or empty. */
  QString selectedPropertyId() const;
  /** @brief Returns the selected contract id. @return Contract id or empty. */
  QString selectedContractId() const;
  /** @brief Returns the selected statement id. @return Statement id or empty. */
  QString selectedStatementId() const;
  /** @brief Returns the selected transaction id. @return Transaction id or empty. */
  QString selectedTransactionId() const;
  /** @brief Returns the selected analysis id. @return Analysis id or empty. */
  QString selectedAnalysisId() const;
  /** @brief Returns the selected annual id. @return Annual id or empty. */
  QString selectedAnnualId() const;

  /** @brief Selects an actor id. @param id Actor id. */
  void setSelectedActorId(const QString &id);
  /** @brief Selects a property id. @param id Property id. */
  void setSelectedPropertyId(const QString &id);
  /** @brief Selects a contract id. @param id Contract id. */
  void setSelectedContractId(const QString &id);
  /** @brief Selects a statement id. @param id Statement id. */
  void setSelectedStatementId(const QString &id);
  /** @brief Selects an analysis id. @param id Analysis id. */
  void setSelectedAnalysisId(const QString &id);
  /** @brief Selects an annual id. @param id Annual id. */
  void setSelectedAnnualId(const QString &id);

  /** @brief Selects an actor. @param id Actor id. */
  void selectActor(const QString &id);
  /** @brief Selects a property. @param id Property id. */
  void selectProperty(const QString &id);
  /** @brief Selects a contract. @param id Contract id. */
  void selectContract(const QString &id);
  /** @brief Selects a statement. @param id Statement id. */
  void selectStatement(const QString &id);
  /**
   * @brief Selects a statement and transaction.
   * @param statementId Statement id.
   * @param id Transaction id.
   */
  void selectTransaction(const QString &statementId, const QString &id);
  /** @brief Selects an analysis. @param id Analysis id. */
  void selectAnalysis(const QString &id);
  /** @brief Selects an annual. @param id Annual id. */
  void selectAnnual(const QString &id);

signals:
  /** @brief Emitted when the selected actor id changed. */
  void selectedActorIdChanged();
  /** @brief Emitted when the selected property id changed. */
  void selectedPropertyIdChanged();
  /** @brief Emitted when the selected contract id changed. */
  void selectedContractIdChanged();
  /** @brief Emitted when the selected statement id changed. */
  void selectedStatementIdChanged();
  /** @brief Emitted when the selected transaction id changed. */
  void selectedTransactionIdChanged();
  /** @brief Emitted when the selected analysis id changed. */
  void selectedAnalysisIdChanged();
  /** @brief Emitted when the selected annual id changed. */
  void selectedAnnualIdChanged();

private:
  /** @brief Emits changed signals after selection normalization. */
  void refreshSelections();
  /** @brief Clears selections that no longer exist in the workspace store. */
  void validateSelections();

  WorkspaceStore &store_;
  WorkspaceSelectors &selectors_;
  QString selectedActorId_;
  QString selectedPropertyId_;
  QString selectedContractId_;
  QString selectedStatementId_;
  QString selectedTransactionId_;
  QString selectedAnalysisId_;
  QString selectedAnnualId_;
};

} // namespace ui
