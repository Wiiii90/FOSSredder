/**
 * @file ui/include/ui/viewmodels/AnnualViewModel.h
 * @brief Declares the UI state adapter for the Annual view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace ui {

class AnnualWorkflow;
class WorkspaceFacade;

class AnnualViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(AnnualViewModel)

  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(QVariantList annualRows READ annualRows NOTIFY changed)
  Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(int year READ year WRITE setYear NOTIFY changed)
  Q_PROPERTY(QVariantList analysisIds READ analysisIds NOTIFY changed)
  Q_PROPERTY(QVariantList availableAnalysisRows READ availableAnalysisRows
                 NOTIFY changed)
  Q_PROPERTY(QVariantList assignedAnalysisRows READ assignedAnalysisRows NOTIFY
                 changed)
  Q_PROPERTY(int workspaceIndex READ workspaceIndex NOTIFY changed)
  Q_PROPERTY(
      QVariantList annualTransactions READ annualTransactions NOTIFY changed)
  Q_PROPERTY(
      QVariantList transactionSections READ transactionSections NOTIFY changed)
  Q_PROPERTY(QVariantList verificationRows READ verificationRows NOTIFY changed)
  Q_PROPERTY(QString statusSummaryText READ statusSummaryText NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)
  Q_PROPERTY(bool hasRows READ hasRows NOTIFY changed)
  Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY changed)

public:
  explicit AnnualViewModel(QObject *parent = nullptr);

  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  AnnualWorkflow *annualWorkflow() const noexcept { return annualWorkflow_; }
  void setWorkspace(WorkspaceFacade *value);
  void setAnnualWorkflow(AnnualWorkflow *value);

  bool isEdit() const;
  QVariantList annualRows() const;
  QString selectedAnnualId() const;
  QString name() const { return name_; }
  void setName(const QString &value);
  int year() const noexcept { return year_; }
  void setYear(int value);
  QVariantList analysisIds() const;
  QVariantList availableAnalysisRows() const { return availableAnalysisRows_; }
  QVariantList assignedAnalysisRows() const { return assignedAnalysisRows_; }
  int workspaceIndex() const noexcept { return workspaceIndex_; }
  QVariantList annualTransactions() const { return annualTransactions_; }
  QVariantList transactionSections() const;
  QVariantList verificationRows() const;
  QString statusSummaryText() const;
  bool canSubmit() const noexcept { return year_ > 0; }
  bool hasRows() const;
  bool hasChanges() const;

  Q_INVOKABLE void refreshFromSelection();
  Q_INVOKABLE void resetCreateState();
  Q_INVOKABLE void submitCreate();
  Q_INVOKABLE void submitUpdate();
  Q_INVOKABLE void deleteCurrent();
  Q_INVOKABLE void navigate(int delta);
  Q_INVOKABLE void selectAnnual(const QString &id);
  Q_INVOKABLE void toggleWorkspace();
  Q_INVOKABLE void stepYear(int delta);
  Q_INVOKABLE void addAvailableAnalysisAtIndex(int index);
  Q_INVOKABLE void removeAnalysis(const QString &id);
  Q_INVOKABLE void setAnalysisExportFormat(const QString &id,
                                           const QString &exportFormat);
  Q_INVOKABLE void toggleTransactionSection(const QString &key);
  Q_INVOKABLE bool isTransactionSectionExpanded(const QString &key) const;

signals:
  void changed();

private:
  QStringList normalizedAnalysisIds(const QVariant &values) const;
  QString analysisIdFromVariant(const QVariant &value) const;
  QVariantMap annualRowById(const QString &id) const;
  QVariantMap analysisRowById(const QString &id) const;
  QVariantMap normalizeAnalysisRow(const QVariant &value) const;
  QVariantList normalizedAnalysisRows() const;
  QVariantList buildAssignedAnalysisRows() const;
  QVariantList buildAvailableAnalysisRows() const;
  QVariantMap augmentAnalysisRow(const QVariantMap &row) const;
  QVariantList exportOptionsForType(const QString &type) const;
  int exportFormatIndex(const QVariantList &options,
                        const QString &exportFormat) const;
  QString normalizedExportFormat(const QString &value,
                                 const QString &type) const;
  int defaultYear() const;
  void loadCreateState();
  void loadSelectedAnnual();
  void refreshChoices();
  void rebuildAnnualResultState();
  void applyAnnualResult(const QVariantMap &result);
  void captureSavedState();
  void setAnalysisIds(const QStringList &ids);
  QVariantMap emptyIssues() const;
  QVariantMap emptyStatusMetrics() const;
  QVariantList rowsFromResultBucket(const QVariantList &rows) const;
  QVariantList missingLiveRows() const;
  QVariantMap transactionRow(const QVariantMap &source) const;
  QVariantMap transactionSection(const QString &key, const QString &title,
                                 const QVariantList &rows) const;
  int groupedCount() const;
  int workspaceRevision() const;
  void bindWorkspace(WorkspaceFacade *value);
  void emitChanged();

  WorkspaceFacade *workspace_ = nullptr;
  AnnualWorkflow *annualWorkflow_ = nullptr;
  QString name_;
  int year_ = 0;
  QStringList analysisIds_;
  QVariantList availableAnalysisRows_;
  QVariantList assignedAnalysisRows_;
  int workspaceIndex_ = 0;
  QVariantMap verificationIssues_;
  QVariantMap statusMetrics_;
  QVariantList annualTransactions_;
  QVariantMap annualTransactionGroups_;
  QVariantMap sectionExpanded_;
  QString savedName_;
  int savedYear_ = 0;
  QStringList savedAnalysisIds_;
  QString lastPreviewAnnualId_;
  int lastPreviewYear_ = 0;
  QStringList lastPreviewAnalysisIds_;
  int lastPreviewRevision_ = -1;
  bool analysisMetadataDirty_ = false;
};

} // namespace ui
