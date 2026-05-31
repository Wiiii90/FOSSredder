/**
 * @file ui/include/ui/state/session/AnalysisState.h
 * @brief Declares the UI state adapter for the Analysis view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace ui {

class AnalysisWorkflow;
class SettingsViewModel;
class WorkspaceFacade;

class AnalysisState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(AnalysisState)

  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(ui::AnalysisWorkflow *analysisWorkflow READ analysisWorkflow WRITE
                 setAnalysisWorkflow NOTIFY changed)
  Q_PROPERTY(ui::SettingsViewModel *settingsViewModel READ settingsViewModel
                 WRITE setSettingsViewModel NOTIFY changed)
  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(QVariantList analysisRows READ analysisRows NOTIFY changed)
  Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(int mainTypeIndex READ mainTypeIndex WRITE setMainTypeIndex NOTIFY
                 changed)
  Q_PROPERTY(int plotSubtypeIndex READ plotSubtypeIndex WRITE setPlotSubtypeIndex
                 NOTIFY changed)
  Q_PROPERTY(QVariantList plotTypeOptions READ plotTypeOptions CONSTANT)
  Q_PROPERTY(QVariantList exportFormatOptions READ exportFormatOptions NOTIFY
                 changed)
  Q_PROPERTY(QString exportFormat READ exportFormat WRITE setExportFormat NOTIFY
                 changed)
  Q_PROPERTY(int exportFormatIndex READ exportFormatIndex WRITE
                 setExportFormatIndex NOTIFY changed)
  Q_PROPERTY(bool includeCalcAdjustments READ includeCalcAdjustments WRITE
                 setIncludeCalcAdjustments NOTIFY changed)
  Q_PROPERTY(QString exportStateJson READ exportStateJson WRITE setExportStateJson
                 NOTIFY changed)
  Q_PROPERTY(bool filterEditMode READ filterEditMode WRITE setFilterEditMode
                 NOTIFY changed)
  Q_PROPERTY(int filterWorkspaceIndex READ filterWorkspaceIndex NOTIFY changed)
  Q_PROPERTY(int dateFieldIndex READ dateFieldIndex WRITE setDateFieldIndex NOTIFY
                 changed)
  Q_PROPERTY(int dateModeIndex READ dateModeIndex WRITE setDateModeIndex NOTIFY
                 changed)
  Q_PROPERTY(QString yearValue READ yearValue WRITE setYearValue NOTIFY changed)
  Q_PROPERTY(QString dateFromValue READ dateFromValue WRITE setDateFromValue
                 NOTIFY changed)
  Q_PROPERTY(QString dateToValue READ dateToValue WRITE setDateToValue NOTIFY
                 changed)
  Q_PROPERTY(QVariantList propertyFilterRows READ propertyFilterRows NOTIFY
                 changed)
  Q_PROPERTY(QVariantList contractTypeRows READ contractTypeRows NOTIFY changed)
  Q_PROPERTY(QVariantList selectedPropertyIds READ selectedPropertyIds WRITE
                 setSelectedPropertyIds NOTIFY changed)
  Q_PROPERTY(QVariantList selectedContractTypes READ selectedContractTypes WRITE
                 setSelectedContractTypes NOTIFY changed)
  Q_PROPERTY(QString allocatableMode READ allocatableMode WRITE setAllocatableMode
                 NOTIFY changed)
  Q_PROPERTY(QVariantList previewTransactions READ previewTransactions NOTIFY
                 changed)
  Q_PROPERTY(QVariantList previewTransactionRows READ previewTransactionRows NOTIFY
                 changed)
  Q_PROPERTY(QVariantMap previewMetrics READ previewMetrics NOTIFY changed)
  Q_PROPERTY(QString previewStatementCountText READ previewStatementCountText
                 NOTIFY changed)
  Q_PROPERTY(QString previewTransactionCountText READ previewTransactionCountText
                 NOTIFY changed)
  Q_PROPERTY(QString previewAmountSumText READ previewAmountSumText NOTIFY
                 changed)
  Q_PROPERTY(QVariantList selectedAdjustmentTxIds READ selectedAdjustmentTxIds
                 WRITE setSelectedAdjustmentTxIds NOTIFY changed)
  Q_PROPERTY(QVariantMap adjustmentAmountsById READ adjustmentAmountsById NOTIFY
                 changed)
  Q_PROPERTY(QString calcName READ calcName WRITE setCalcName NOTIFY changed)
  Q_PROPERTY(QString calcPercentText READ calcPercentText WRITE setCalcPercentText
                 NOTIFY changed)
  Q_PROPERTY(QString pendingAdjustmentsJson READ pendingAdjustmentsJson NOTIFY
                 changed)
  Q_PROPERTY(QString currentResultType READ currentResultType NOTIFY changed)
  Q_PROPERTY(bool currentResultIsTable READ currentResultIsTable NOTIFY changed)
  Q_PROPERTY(QString renderedPreviewSource READ renderedPreviewSource NOTIFY
                 changed)
  Q_PROPERTY(QVariantList tableContractTypes READ tableContractTypes NOTIFY
                 changed)
  Q_PROPERTY(QVariantList tablePropertyRows READ tablePropertyRows NOTIFY
                 changed)
  Q_PROPERTY(double tableGrandTotal READ tableGrandTotal NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)
  Q_PROPERTY(bool hasRows READ hasRows NOTIFY changed)

public:
  explicit AnalysisState(QObject *parent = nullptr);

  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  AnalysisWorkflow *analysisWorkflow() const noexcept { return analysisWorkflow_; }
  SettingsViewModel *settingsViewModel() const noexcept {
    return settingsViewModel_;
  }

  void setWorkspace(WorkspaceFacade *value);
  void setAnalysisWorkflow(AnalysisWorkflow *value);
  void setSettingsViewModel(SettingsViewModel *value);

  bool isEdit() const;
  QString name() const { return name_; }
  void setName(const QString &value);
  int mainTypeIndex() const noexcept { return mainTypeIndex_; }
  void setMainTypeIndex(int value);
  int plotSubtypeIndex() const noexcept { return plotSubtypeIndex_; }
  void setPlotSubtypeIndex(int value);
  QVariantList plotTypeOptions() const;
  QVariantList exportFormatOptions() const;
  QString exportFormat() const { return exportFormat_; }
  void setExportFormat(const QString &value);
  int exportFormatIndex() const;
  void setExportFormatIndex(int value);
  bool includeCalcAdjustments() const noexcept {
    return includeCalcAdjustments_;
  }
  void setIncludeCalcAdjustments(bool value);
  QString exportStateJson() const { return exportStateJson_; }
  void setExportStateJson(const QString &value);
  bool filterEditMode() const noexcept { return filterEditMode_; }
  void setFilterEditMode(bool value);
  int filterWorkspaceIndex() const noexcept { return filterWorkspaceIndex_; }
  int dateFieldIndex() const noexcept { return dateFieldIndex_; }
  void setDateFieldIndex(int value);
  int dateModeIndex() const noexcept { return dateModeIndex_; }
  void setDateModeIndex(int value);
  QString yearValue() const { return yearValue_; }
  void setYearValue(const QString &value);
  QString dateFromValue() const { return dateFromValue_; }
  void setDateFromValue(const QString &value);
  QString dateToValue() const { return dateToValue_; }
  void setDateToValue(const QString &value);
  QVariantList propertyFilterRows() const { return propertyFilterRows_; }
  QVariantList contractTypeRows() const { return contractTypeRows_; }
  QVariantList selectedPropertyIds() const { return selectedPropertyIds_; }
  void setSelectedPropertyIds(const QVariantList &value);
  QVariantList selectedContractTypes() const { return selectedContractTypes_; }
  void setSelectedContractTypes(const QVariantList &value);
  QString allocatableMode() const { return allocatableMode_; }
  void setAllocatableMode(const QString &value);
  QVariantList previewTransactions() const { return previewTransactions_; }
  QVariantList previewTransactionRows() const;
  QVariantMap previewMetrics() const { return previewMetrics_; }
  QString previewStatementCountText() const;
  QString previewTransactionCountText() const;
  QString previewAmountSumText() const;
  QVariantList selectedAdjustmentTxIds() const { return selectedAdjustmentTxIds_; }
  void setSelectedAdjustmentTxIds(const QVariantList &value);
  QVariantMap adjustmentAmountsById() const { return adjustmentAmountsById_; }
  QString calcName() const { return calcName_; }
  void setCalcName(const QString &value);
  QString calcPercentText() const { return calcPercentText_; }
  void setCalcPercentText(const QString &value);
  QString pendingAdjustmentsJson() const { return pendingAdjustmentsJson_; }
  QString currentResultType() const;
  bool currentResultIsTable() const;
  QString renderedPreviewSource() const { return renderedPreviewSource_; }
  QVariantList tableContractTypes() const { return tableContractTypes_; }
  QVariantList tablePropertyRows() const { return tablePropertyRows_; }
  double tableGrandTotal() const noexcept { return tableGrandTotal_; }
  bool canSubmit() const;
  bool hasRows() const;

  Q_INVOKABLE void refreshFromSelection();
  Q_INVOKABLE void refreshPreview();
  Q_INVOKABLE void requestPreviewRefresh();
  Q_INVOKABLE void applySelectedCalc();
  Q_INVOKABLE bool isPropertySelected(const QString &id) const;
  Q_INVOKABLE void setPropertySelected(const QString &id, bool selected);
  Q_INVOKABLE void selectAllProperties();
  Q_INVOKABLE void selectNoProperties();
  Q_INVOKABLE void selectUnassignedProperties();
  Q_INVOKABLE bool isContractTypeSelected(const QString &type) const;
  Q_INVOKABLE void setContractTypeSelected(const QString &type, bool selected);
  Q_INVOKABLE void selectAllContractTypes();
  Q_INVOKABLE void selectNoContractTypes();
  Q_INVOKABLE void selectUnassignedContractTypes();
  Q_INVOKABLE bool isAdjustmentTransactionSelected(const QString &id) const;
  Q_INVOKABLE void setAdjustmentTransactionSelected(const QString &id,
                                                    bool selected);
  Q_INVOKABLE void setAllocatableModeIndex(int index);
  Q_INVOKABLE void toggleFilterWorkspace();
  Q_INVOKABLE void clearFilters();
  Q_INVOKABLE void submitCreate();
  Q_INVOKABLE void submitUpdate();
  Q_INVOKABLE void deleteCurrent();
  Q_INVOKABLE void navigate(int delta);
  Q_INVOKABLE void selectAnalysis(const QString &id);

signals:
  void changed();

private:
  QString selectedAnalysisId() const;
  QVariantList analysisRows() const;
  QVariantMap analysisRowById(const QString &id) const;
  QVariantMap analysisRowAt(int index) const;
  int analysisIndexById(const QString &id) const;
  QString uiType() const;
  QString strategyType() const;
  QString currentDateField() const;
  QString currentDateMode() const;
  QString defaultAnalysisDateMode() const;
  QString defaultAnalysisYear() const;
  QStringList stringList(const QVariantList &values) const;
  QVariantList allPropertyIds() const;
  QVariantList allContractTypes() const;
  QVariantList pruneSelection(const QVariantList &values,
                              const QVariantList &availableRows,
                              const QString &idField = {}) const;
  bool isAllSelected(const QVariantList &selectedIds,
                     const QVariantList &availableRows,
                     const QString &idField = {}) const;
  QStringList effectiveSelectedPropertyIds() const;
  QStringList effectiveSelectedContractTypes() const;
  QString currentFilterSpec() const;
  QString normalizedExportFormat(const QString &value, const QString &uiType) const;
  QString normalizedExportStateJson(const QString &value) const;
  QVariantMap parseJsonObject(const QString &value) const;
  QString previewSnapshotJson() const;
  QVariantList adjustmentIds(const QVariantMap &adjustments) const;
  void ensureChoices();
  void resetAdjustments();
  void refreshPendingAdjustmentsFromCalcSelection();
  void refreshAnalysisResult();
  void loadSelectedAnalysis();
  void loadCreateState();
  void captureSavedState();
  void updateResultState();
  void rebuildTableState();
  void emitChanged();
  void bindWorkspace(WorkspaceFacade *value);

  WorkspaceFacade *workspace_ = nullptr;
  AnalysisWorkflow *analysisWorkflow_ = nullptr;
  SettingsViewModel *settingsViewModel_ = nullptr;
  QTimer previewDebounce_;

  QString name_;
  int mainTypeIndex_ = 0;
  int plotSubtypeIndex_ = 0;
  QString exportFormat_;
  bool includeCalcAdjustments_ = true;
  QString exportStateJson_ = QStringLiteral("{}");
  QString snapshotTransactionsJson_ = QStringLiteral("{}");
  bool filterEditMode_ = true;
  int filterWorkspaceIndex_ = 0;
  int dateFieldIndex_ = 0;
  int dateModeIndex_ = 0;
  QString yearValue_;
  QString dateFromValue_;
  QString dateToValue_;
  QVariantList propertyFilterRows_;
  QVariantList contractTypeRows_;
  QVariantList selectedPropertyIds_;
  QVariantList selectedContractTypes_;
  QString allocatableMode_ = QStringLiteral("all");
  QVariantList previewTransactions_;
  QVariantMap previewMetrics_;
  QVariantList selectedAdjustmentTxIds_;
  QVariantMap adjustmentAmountsById_;
  QString calcName_;
  QString calcPercentText_;
  QString pendingAdjustmentsJson_ = QStringLiteral("{}");
  QString activeResultType_ = QStringLiteral("plot");
  QString renderedPreviewSource_;
  int renderedPreviewRevision_ = 0;
  QVariantList tableContractTypes_;
  QVariantList tablePropertyRows_;
  double tableGrandTotal_ = 0.0;

  QVariantMap savedState_;
};

} // namespace ui
