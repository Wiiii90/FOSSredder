/**
 * @file ui/include/ui/workspace/WorkspaceCache.h
 * @brief Declares the UI workspace cache that owns models, filters and derived
 * metrics.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/WorkspaceFilterState.h"
#include "ui/workspace/WorkspaceCacheModels.h"
#include "ui/workspace/TransactionFilterModel.h"

namespace ui {

/**
 * @brief Owns the UI-facing workspace cache models and derived lookup/filter
 * state.
 */
class WorkspaceCache : public QObject {
  Q_OBJECT
  Q_PROPERTY(int dataRevision READ dataRevision NOTIFY dataRevisionChanged)

public:
  /** @brief Creates the workspace cache and connects derived-state
   * recomputation.
   */
  explicit WorkspaceCache(QObject *parent = nullptr);

  WorkspaceCacheModels &models() noexcept { return models_; }
  const WorkspaceCacheModels &models() const noexcept { return models_; }

  /** @brief Replaces all UI model data from a domain application state
   * snapshot. */
  void loadFromState(const core::ports::workspace::WorkspaceSnapshot &state);
  /** @brief Returns a live filter over transactions for a statement. */
  TransactionFilter *statementTransactions(const QString &statementId);
  /** @brief Returns a live filter over transactions for a property. */
  TransactionFilter *propertyTransactions(const QString &propertyId);
  /** @brief Applies domain deletion effects to cached models. */
  void
  applyDeletionImpact(const core::ports::workspace::DeletionImpact &impact);
  /** @brief Applies an immediate property reassignment for a single
   * transaction. */
  void setTransactionPropertyIdsImmediate(const QString &txId,
                                          const QStringList &propertyIds);
  int dataRevision() const noexcept { return dataRevision_; }

  double amountForTransactionCommit(const QVariant &rawAmount,
                                    const QString &transactionId,
                                    double fallbackAmount) const;

signals:
  void dataRevisionChanged();

private:
  void bumpDataRevision();

  WorkspaceFilterState filters_;
  WorkspaceCacheModels models_;
  int dataRevision_ = 0;
};

} // namespace ui
