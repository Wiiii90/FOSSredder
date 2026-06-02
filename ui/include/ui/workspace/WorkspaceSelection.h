/**
 * @file ui/include/ui/workspace/WorkspaceSelection.h
 * @brief Declares the workspace selection state synchronized with cache models.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include "ui/workspace/WorkspaceCacheModels.h"

namespace ui {

/**
 * @brief Tracks the currently selected entities across all UI model
 * collections.
 */
class WorkspaceSelection : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE
                 setSelectedActorId NOTIFY selectedActorIdChanged)
  Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE
                 setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
  Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE
                 setSelectedContractId NOTIFY selectedContractIdChanged)
  Q_PROPERTY(QString selectedStatementId READ selectedStatementId WRITE
                 setSelectedStatementId NOTIFY selectedStatementIdChanged)
  Q_PROPERTY(QString selectedTransactionId READ selectedTransactionId WRITE
                 setSelectedTransactionId NOTIFY selectedTransactionIdChanged)
  Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId WRITE
                 setSelectedAnalysisId NOTIFY selectedAnalysisIdChanged)
  Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId WRITE
                 setSelectedAnnualId NOTIFY selectedAnnualIdChanged)

  Q_PROPERTY(QVariant lastAnalysisResult READ lastAnalysisResult WRITE
                 setLastAnalysisResult NOTIFY lastAnalysisResultChanged)

public:
  /** @brief Creates the selection state wrapper for the supplied cache models. */
  explicit WorkspaceSelection(WorkspaceCacheModels &models,
                              QObject *parent = nullptr);

  /** @brief Refreshes selection objects after the underlying models were
   * reloaded. */
  void loadFromState();

  QString selectedActorId() const;
  QString selectedPropertyId() const;
  QString selectedContractId() const;
  QString selectedStatementId() const;
  QString selectedTransactionId() const;
  QString selectedAnalysisId() const;
  QString selectedAnnualId() const;

  void setSelectedActorId(const QString &id);
  void setSelectedPropertyId(const QString &id);
  void setSelectedContractId(const QString &id);
  void setSelectedStatementId(const QString &id);
  void setSelectedTransactionId(const QString &id);
  void setSelectedAnalysisId(const QString &id);
  void setSelectedAnnualId(const QString &id);

  QVariant lastAnalysisResult() const { return lastAnalysisResult_; }
  /** @brief Stores the latest analysis payload tied to the active analysis
   * selection. */
  void setLastAnalysisResult(const QVariant &value);

signals:
  void selectedActorIdChanged();
  void selectedPropertyIdChanged();
  void selectedContractIdChanged();
  void selectedStatementIdChanged();
  void selectedTransactionIdChanged();
  void selectedAnalysisIdChanged();
  void selectedAnnualIdChanged();
  void lastAnalysisResultChanged();

private:
  void bindModelSignals();
  void refreshSelections();
  void validateSelections();
  void clearAnalysisResult();

  WorkspaceCacheModels &models_;
  QString selectedActorId_;
  QString selectedPropertyId_;
  QString selectedContractId_;
  QString selectedStatementId_;
  QString selectedTransactionId_;
  QString selectedAnalysisId_;
  QString selectedAnnualId_;
  QVariant lastAnalysisResult_;
};

} // namespace ui
