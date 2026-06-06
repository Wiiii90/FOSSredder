/**
 * @file ui/include/ui/workspace/WorkspaceSelectors.h
 * @brief Declares read-only workspace selectors for QML payloads.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "ui/workspace/WorkspaceStore.h"

namespace ui {

/**
 * @brief Projects workspace store data into QML-facing read models.
 */
class WorkspaceSelectors : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Creates selectors for a workspace store.
   * @param store Workspace store dependency.
   * @param parent Optional Qt parent.
   */
  explicit WorkspaceSelectors(WorkspaceStore& store, QObject* parent = nullptr);

  /**
   * @brief Returns the current workspace data revision.
   * @return Monotonic workspace revision.
   */
  int dataRevision() const noexcept;

  /**
   * @brief Returns actor rows for QML tables and dropdowns.
   * @return Actor rows projected from the workspace snapshot.
   */
  QVariantList actorRows() const;
  /**
   * @brief Returns property rows for QML tables and dropdowns.
   * @return Property rows projected from the workspace snapshot.
   */
  QVariantList propertyRows() const;
  /**
   * @brief Returns contract rows for QML tables and dropdowns.
   * @return Contract rows projected from the workspace snapshot.
   */
  QVariantList contractRows() const;
  /**
   * @brief Returns analysis rows for QML views.
   * @return Analysis rows projected from the workspace snapshot.
   */
  QVariantList analysisRows() const;
  /**
   * @brief Returns annual rows for QML views.
   * @return Annual rows projected from the workspace snapshot.
   */
  QVariantList annualRows() const;
  /**
   * @brief Returns statement rows without embedded transactions.
   * @return Statement rows projected from the workspace snapshot.
   */
  QVariantList statementRows() const;
  /**
   * @brief Returns statement rows with their transaction rows embedded.
   * @return Statement rows containing a transactions list.
   */
  QVariantList statementRowsWithTransactions() const;
  /**
   * @brief Returns transactions belonging to a statement.
   * @param statementId Statement id used for filtering.
   * @return Transaction rows for the statement.
   */
  QVariantList statementTransactionRows(const QString& statementId) const;
  /**
   * @brief Looks up a transaction row by id.
   * @param id Transaction id.
   * @return Transaction row or an empty map.
   */
  QVariantMap transactionRowById(const QString& id) const;
  /**
   * @brief Applies catalog-dependent transaction form selection changes.
   * @param transaction Current transaction form payload.
   * @param changes Field changes from the form.
   * @return Updated transaction form payload.
   */
  QVariantMap
  transactionFormWithCatalogSelection(const QVariantMap& transaction,
                                      const QVariantMap& changes) const;
  /**
   * @brief Returns actor dropdown rows including the empty option.
   * @return Actor dropdown rows.
   */
  QVariantList actorDropdownRows() const;
  /**
   * @brief Returns property dropdown rows.
   * @return Property dropdown rows.
   */
  QVariantList propertyDropdownRows() const;
  /**
   * @brief Returns contract dropdown rows including the empty option.
   * @return Contract dropdown rows.
   */
  QVariantList contractDropdownRows() const;
  /**
   * @brief Returns import log rows for the import sidebar.
   * @return Import log rows.
   */
  QVariantList importLogRows() const;
  /**
   * @brief Returns export log rows for the export sidebar.
   * @return Export log rows.
   */
  QVariantList exportLogRows() const;
  /**
   * @brief Returns ids of import logs that have attached statement drafts.
   * @return Draft ids or log ids usable for draft restoration.
   */
  QStringList attachedImportDraftIds() const;
  /**
   * @brief Resolves an export log target path.
   * @param id Export log id.
   * @return Target path or an empty string.
   */
  QString exportLogTargetPath(const QString& id) const;

  /**
   * @brief Looks up an actor id by display name.
   * @param name Actor name.
   * @return Actor id or an empty string.
   */
  QString actorIdByName(const QString& name) const;
  /**
   * @brief Looks up a property id by display name.
   * @param name Property name.
   * @return Property id or an empty string.
   */
  QString propertyIdByName(const QString& name) const;
  /**
   * @brief Looks up a contract id by unique signature.
   * @param name Contract name.
   * @param type Contract type.
   * @param actorIds Actor ids linked to the contract.
   * @param propertyIds Property ids linked to the contract.
   * @return Contract id or an empty string.
   */
  QString contractIdBySignature(const QString& name, const QString& type,
                                const QStringList& actorIds,
                                const QStringList& propertyIds) const;
  /**
   * @brief Returns the next generated contract name.
   * @return Generated contract name.
   */
  QString nextContractName() const;

  /** @brief Checks whether an actor id exists. @param id Actor id. @return True
   * when found. */
  bool hasActorId(const QString& id) const;
  /** @brief Checks whether a property id exists. @param id Property id. @return
   * True when found. */
  bool hasPropertyId(const QString& id) const;
  /** @brief Checks whether a contract id exists. @param id Contract id. @return
   * True when found. */
  bool hasContractId(const QString& id) const;
  /** @brief Checks whether a statement id exists. @param id Statement id.
   * @return True when found. */
  bool hasStatementId(const QString& id) const;
  /** @brief Checks whether a transaction id exists. @param id Transaction id.
   * @return True when found. */
  bool hasTransactionId(const QString& id) const;
  /** @brief Checks whether an analysis id exists. @param id Analysis id.
   * @return True when found. */
  bool hasAnalysisId(const QString& id) const;
  /** @brief Checks whether an annual id exists. @param id Annual id. @return
   * True when found. */
  bool hasAnnualId(const QString& id) const;

private:
  /**
   * @brief Returns the contract type for a contract id.
   * @param contractId Contract id.
   * @return Contract type or empty.
   */
  QString contractTypeForId(const QString& contractId) const;

  WorkspaceStore& store_;
};

} // namespace ui
