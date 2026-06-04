/**
 * @file ui/include/ui/viewmodels/AnalysisViewModel.h
 * @brief Declares the QML API for the Analysis view.
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
class Settings;
class WorkspaceFacade;

/**
 * @brief Owns Analysis view form state and delegates analysis CRUD and
 * computation to workspace and workflow.
 */
class AnalysisViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(AnalysisViewModel)

  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(QVariantList analysisRows READ analysisRows NOTIFY changed)
  Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(int mainTypeIndex READ mainTypeIndex WRITE setMainTypeIndex NOTIFY
                 changed)
  Q_PROPERTY(int plotSubtypeIndex READ plotSubtypeIndex WRITE
                 setPlotSubtypeIndex NOTIFY changed)
  Q_PROPERTY(QVariantList plotTypeOptions READ plotTypeOptions CONSTANT)
  Q_PROPERTY(
      QVariantList exportFormatOptions READ exportFormatOptions NOTIFY changed)
  Q_PROPERTY(int exportFormatIndex READ exportFormatIndex WRITE
                 setExportFormatIndex NOTIFY changed)
  Q_PROPERTY(bool includeAdjustments READ includeAdjustments WRITE
                 setIncludeAdjustments NOTIFY changed)
  Q_PROPERTY(bool filterEditMode READ filterEditMode WRITE setFilterEditMode
                 NOTIFY changed)
  Q_PROPERTY(int filterContentIndex READ filterContentIndex NOTIFY changed)
  Q_PROPERTY(int dateFieldIndex READ dateFieldIndex WRITE setDateFieldIndex
                 NOTIFY changed)
  Q_PROPERTY(int dateModeIndex READ dateModeIndex WRITE setDateModeIndex NOTIFY
                 changed)
  Q_PROPERTY(QString yearValue READ yearValue WRITE setYearValue NOTIFY changed)
  Q_PROPERTY(QString dateFromValue READ dateFromValue WRITE setDateFromValue
                 NOTIFY changed)
  Q_PROPERTY(
      QString dateToValue READ dateToValue WRITE setDateToValue NOTIFY changed)
  Q_PROPERTY(
      QVariantList propertyFilterRows READ propertyFilterRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractTypeRows READ contractTypeRows NOTIFY changed)
  Q_PROPERTY(QVariantList selectedPropertyIds READ selectedPropertyIds WRITE
                 setSelectedPropertyIds NOTIFY changed)
  Q_PROPERTY(QVariantList selectedContractTypes READ selectedContractTypes WRITE
                 setSelectedContractTypes NOTIFY changed)
  Q_PROPERTY(QString allocatableMode READ allocatableMode WRITE
                 setAllocatableMode NOTIFY changed)
  Q_PROPERTY(QVariantList previewTransactionRows READ previewTransactionRows
                 NOTIFY changed)
  Q_PROPERTY(QString previewStatementCountText READ previewStatementCountText
                 NOTIFY changed)
  Q_PROPERTY(QString previewTransactionCountText READ
                 previewTransactionCountText NOTIFY changed)
  Q_PROPERTY(
      QString previewAmountSumText READ previewAmountSumText NOTIFY changed)
  Q_PROPERTY(QVariantList selectedAdjustmentTxIds READ selectedAdjustmentTxIds
                 WRITE setSelectedAdjustmentTxIds NOTIFY changed)
  Q_PROPERTY(QString adjustmentName READ adjustmentName WRITE setAdjustmentName
                 NOTIFY changed)
  Q_PROPERTY(QString adjustmentPercentText READ adjustmentPercentText WRITE
                 setAdjustmentPercentText NOTIFY changed)
  Q_PROPERTY(bool currentResultIsTable READ currentResultIsTable NOTIFY changed)
  Q_PROPERTY(
      QString renderedPreviewSource READ renderedPreviewSource NOTIFY changed)
  Q_PROPERTY(
      QVariantList tableContractTypes READ tableContractTypes NOTIFY changed)
  Q_PROPERTY(
      QVariantList tablePropertyRows READ tablePropertyRows NOTIFY changed)
  Q_PROPERTY(double tableGrandTotal READ tableGrandTotal NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)
  Q_PROPERTY(bool hasRows READ hasRows NOTIFY changed)

public:
  /**
   * @brief Creates the analysis view model.
   * @param parent Optional Qt parent object.
   */
  explicit AnalysisViewModel(QObject* parent = nullptr);

  /**
   * @brief Sets the workspace API used for analysis CRUD and rows.
   * @param value Workspace facade or nullptr.
   */
  void setWorkspace(WorkspaceFacade* value);

  /**
   * @brief Sets the workflow used for analysis computation and rendering.
   * @param value Analysis workflow or nullptr.
   */
  void setAnalysisWorkflow(AnalysisWorkflow* value);

  /**
   * @brief Sets the settings source used for analysis defaults.
   * @param value Settings object or nullptr.
   */
  void setSettings(Settings* value);

  /**
   * @brief Returns whether a persisted analysis is selected.
   * @return True in edit mode, false in create mode.
   */
  bool isEdit() const;

  /**
   * @brief Returns the analysis name field.
   * @return Current analysis name.
   */
  QString name() const {
    return name_;
  }

  /**
   * @brief Updates the analysis name field.
   * @param value New analysis name.
   */
  void setName(const QString& value);

  /**
   * @brief Returns the selected main analysis type index.
   * @return Index of the main type option.
   */
  int mainTypeIndex() const noexcept {
    return mainTypeIndex_;
  }

  /**
   * @brief Updates the selected main analysis type index.
   * @param value New main type index.
   */
  void setMainTypeIndex(int value);

  /**
   * @brief Returns the selected plot subtype index.
   * @return Index of the plot subtype option.
   */
  int plotSubtypeIndex() const noexcept {
    return plotSubtypeIndex_;
  }

  /**
   * @brief Updates the selected plot subtype index.
   * @param value New plot subtype index.
   */
  void setPlotSubtypeIndex(int value);

  /**
   * @brief Returns selectable plot type options.
   * @return Plot type option rows.
   */
  QVariantList plotTypeOptions() const;

  /**
   * @brief Returns selectable export format options for the current type.
   * @return Export format option rows.
   */
  QVariantList exportFormatOptions() const;

  /**
   * @brief Returns the current export format key.
   * @return Export format key.
   */
  QString exportFormat() const {
    return exportFormat_;
  }

  /**
   * @brief Updates the current export format key.
   * @param value Export format key.
   */
  void setExportFormat(const QString& value);

  /**
   * @brief Returns the selected export format index.
   * @return Index into exportFormatOptions().
   */
  int exportFormatIndex() const;

  /**
   * @brief Updates the selected export format index.
   * @param value Index into exportFormatOptions().
   */
  void setExportFormatIndex(int value);

  /**
   * @brief Returns whether adjustment amounts are included.
   * @return True when adjusted values should be included.
   */
  bool includeAdjustments() const noexcept {
    return includeAdjustments_;
  }

  /**
   * @brief Updates whether adjustment amounts are included.
   * @param value True to include adjusted values.
   */
  void setIncludeAdjustments(bool value);

  /**
   * @brief Returns whether filter editing is shown in create mode.
   * @return True when the filter editor is visible.
   */
  bool filterEditMode() const noexcept {
    return filterEditMode_;
  }

  /**
   * @brief Updates whether filter editing is shown in create mode.
   * @param value True to show the filter editor.
   */
  void setFilterEditMode(bool value);

  /**
   * @brief Returns the active filter content stack index.
   * @return Filter content stack index.
   */
  int filterContentIndex() const noexcept {
    return filterContentIndex_;
  }

  /**
   * @brief Returns the selected date field index.
   * @return Index of the date field option.
   */
  int dateFieldIndex() const noexcept {
    return dateFieldIndex_;
  }

  /**
   * @brief Updates the selected date field index.
   * @param value New date field index.
   */
  void setDateFieldIndex(int value);

  /**
   * @brief Returns the selected date mode index.
   * @return Index of the date mode option.
   */
  int dateModeIndex() const noexcept {
    return dateModeIndex_;
  }

  /**
   * @brief Updates the selected date mode index.
   * @param value New date mode index.
   */
  void setDateModeIndex(int value);

  /**
   * @brief Returns the year filter field.
   * @return Year text.
   */
  QString yearValue() const {
    return yearValue_;
  }

  /**
   * @brief Updates the year filter field.
   * @param value New year text.
   */
  void setYearValue(const QString& value);

  /**
   * @brief Returns the start date filter field.
   * @return Start date text.
   */
  QString dateFromValue() const {
    return dateFromValue_;
  }

  /**
   * @brief Updates the start date filter field.
   * @param value New start date text.
   */
  void setDateFromValue(const QString& value);

  /**
   * @brief Returns the end date filter field.
   * @return End date text.
   */
  QString dateToValue() const {
    return dateToValue_;
  }

  /**
   * @brief Updates the end date filter field.
   * @param value New end date text.
   */
  void setDateToValue(const QString& value);

  /**
   * @brief Returns property rows for the analysis property filter.
   * @return Property filter rows.
   */
  QVariantList propertyFilterRows() const {
    return propertyFilterRows_;
  }

  /**
   * @brief Returns contract type rows for the contract type filter.
   * @return Contract type rows.
   */
  QVariantList contractTypeRows() const {
    return contractTypeRows_;
  }

  /**
   * @brief Returns selected property ids for filtering.
   * @return Selected property ids.
   */
  QVariantList selectedPropertyIds() const {
    return selectedPropertyIds_;
  }

  /**
   * @brief Updates selected property ids for filtering.
   * @param value New selected property ids.
   */
  void setSelectedPropertyIds(const QVariantList& value);

  /**
   * @brief Returns selected contract type keys for filtering.
   * @return Selected contract type keys.
   */
  QVariantList selectedContractTypes() const {
    return selectedContractTypes_;
  }

  /**
   * @brief Updates selected contract type keys for filtering.
   * @param value New selected contract type keys.
   */
  void setSelectedContractTypes(const QVariantList& value);

  /**
   * @brief Returns the selected allocatable filter mode.
   * @return Allocatable mode key.
   */
  QString allocatableMode() const {
    return allocatableMode_;
  }

  /**
   * @brief Updates the selected allocatable filter mode.
   * @param value Allocatable mode key.
   */
  void setAllocatableMode(const QString& value);

  /**
   * @brief Returns preview transactions prepared for QML.
   * @return QML transaction preview rows.
   */
  QVariantList previewTransactionRows() const;

  /**
   * @brief Returns localized statement count text for the preview.
   * @return Statement count text.
   */
  QString previewStatementCountText() const;

  /**
   * @brief Returns localized transaction count text for the preview.
   * @return Transaction count text.
   */
  QString previewTransactionCountText() const;

  /**
   * @brief Returns localized amount sum text for the preview.
   * @return Amount sum text.
   */
  QString previewAmountSumText() const;

  /**
   * @brief Returns transaction ids selected for adjustment.
   * @return Selected adjustment transaction ids.
   */
  QVariantList selectedAdjustmentTxIds() const {
    return selectedAdjustmentTxIds_;
  }

  /**
   * @brief Updates transaction ids selected for adjustment.
   * @param value Selected adjustment transaction ids.
   */
  void setSelectedAdjustmentTxIds(const QVariantList& value);

  /**
   * @brief Returns the adjustment name field.
   * @return Adjustment name.
   */
  QString adjustmentName() const {
    return adjustmentName_;
  }

  /**
   * @brief Updates the adjustment name field.
   * @param value New adjustment name.
   */
  void setAdjustmentName(const QString& value);

  /**
   * @brief Returns the adjustment percent field.
   * @return Percent text.
   */
  QString adjustmentPercentText() const {
    return adjustmentPercentText_;
  }

  /**
   * @brief Updates the adjustment percent field.
   * @param value New percent text.
   */
  void setAdjustmentPercentText(const QString& value);

  /**
   * @brief Returns the active analysis result type.
   * @return Result type key.
   */
  QString currentResultType() const;

  /**
   * @brief Returns whether the active result is a table.
   * @return True when the current result should be shown as a table.
   */
  bool currentResultIsTable() const;

  /**
   * @brief Returns the rendered plot preview image source.
   * @return QML image source string.
   */
  QString renderedPreviewSource() const {
    return renderedPreviewSource_;
  }

  /**
   * @brief Returns table column contract types.
   * @return Contract type rows for the table.
   */
  QVariantList tableContractTypes() const {
    return tableContractTypes_;
  }

  /**
   * @brief Returns table property rows.
   * @return Property rows for the table.
   */
  QVariantList tablePropertyRows() const {
    return tablePropertyRows_;
  }

  /**
   * @brief Returns the table grand total.
   * @return Grand total amount.
   */
  double tableGrandTotal() const noexcept {
    return tableGrandTotal_;
  }

  /**
   * @brief Returns whether the current form can be submitted.
   * @return True when quick UI checks pass.
   */
  bool canSubmit() const;

  /**
   * @brief Returns whether persisted analysis rows exist.
   * @return True when the sidebar has rows.
   */
  bool hasRows() const;

  /**
   * @brief Reloads create or edit state from the workspace selection.
   */
  Q_INVOKABLE void refreshFromSelection();

  /**
   * @brief Applies the entered adjustment to selected preview transactions.
   */
  Q_INVOKABLE void applySelectedAdjustment();

  /**
   * @brief Updates whether a property is selected for filtering.
   * @param id Property id.
   * @param selected True to select, false to remove.
   */
  Q_INVOKABLE void setPropertySelected(const QString& id, bool selected);

  /**
   * @brief Selects all available properties.
   */
  Q_INVOKABLE void selectAllProperties();

  /**
   * @brief Clears the property selection.
   */
  Q_INVOKABLE void selectNoProperties();

  /**
   * @brief Updates whether a contract type is selected for filtering.
   * @param type Contract type key.
   * @param selected True to select, false to remove.
   */
  Q_INVOKABLE void setContractTypeSelected(const QString& type, bool selected);

  /**
   * @brief Selects all available contract types.
   */
  Q_INVOKABLE void selectAllContractTypes();

  /**
   * @brief Clears the contract type selection.
   */
  Q_INVOKABLE void selectNoContractTypes();

  /**
   * @brief Updates whether a preview transaction is selected for adjustment.
   * @param id Transaction id.
   * @param selected True to select, false to remove.
   */
  Q_INVOKABLE void setAdjustmentTransactionSelected(const QString& id,
                                                    bool selected);

  /**
   * @brief Updates allocatable mode from a segmented control index.
   * @param index Allocatable mode index.
   */
  Q_INVOKABLE void setAllocatableModeIndex(int index);

  /**
   * @brief Toggles between filter fields and transaction preview.
   */
  Q_INVOKABLE void toggleFilterContent();

  /**
   * @brief Resets all filter fields and refreshes the preview.
   */
  Q_INVOKABLE void clearFilters();

  /**
   * @brief Creates an analysis through workspace.
   */
  Q_INVOKABLE void submitCreate();

  /**
   * @brief Updates the selected analysis through workspace.
   */
  Q_INVOKABLE void submitUpdate();

  /**
   * @brief Deletes the selected analysis through workspace.
   */
  Q_INVOKABLE void deleteCurrent();

  /**
   * @brief Navigates between analyses.
   * @param delta Relative movement, usually -1 or 1.
   */
  Q_INVOKABLE void navigate(int delta);

  /**
   * @brief Selects an analysis row.
   * @param id Analysis id or empty string for create mode.
   */
  Q_INVOKABLE void selectAnalysis(const QString& id);

signals:
  void changed();

private:
  /**
   * @brief Returns the selected analysis id from workspace.
   * @return Selected analysis id.
   */
  QString selectedAnalysisId() const;
  /**
   * @brief Returns analysis rows from workspace.
   * @return Analysis rows.
   */
  QVariantList analysisRows() const;
  /**
   * @brief Returns an analysis row by id.
   * @param id Analysis id.
   * @return Analysis row or empty map.
   */
  QVariantMap analysisRowById(const QString& id) const;
  /**
   * @brief Returns the UI analysis type key.
   * @return UI type key.
   */
  QString uiType() const;
  /**
   * @brief Returns the core strategy type key.
   * @return Strategy type key.
   */
  QString strategyType() const;
  /**
   * @brief Returns the selected date field key.
   * @return Date field key.
   */
  QString currentDateField() const;
  /**
   * @brief Returns the selected date mode key.
   * @return Date mode key.
   */
  QString currentDateMode() const;
  /**
   * @brief Returns the default analysis date mode from settings.
   * @return Date mode key.
   */
  QString defaultAnalysisDateMode() const;
  /**
   * @brief Returns the default analysis year.
   * @return Year text.
   */
  QString defaultAnalysisYear() const;
  /**
   * @brief Converts variant values to strings.
   * @param values Input values.
   * @return String list.
   */
  QStringList stringList(const QVariantList& values) const;
  /**
   * @brief Returns all property ids currently available for filtering.
   * @return Property ids.
   */
  QVariantList allPropertyIds() const;
  /**
   * @brief Returns all contract type keys currently available for filtering.
   * @return Contract type keys.
   */
  QVariantList allContractTypes() const;
  /**
   * @brief Removes selected ids that are no longer available.
   * @param values Selected values.
   * @param availableRows Available rows.
   * @param idField Optional id field to read from rows.
   * @return Pruned selection.
   */
  QVariantList pruneSelection(const QVariantList& values,
                              const QVariantList& availableRows,
                              const QString& idField = {}) const;
  /**
   * @brief Checks whether every available row is selected.
   * @param selectedIds Selected ids.
   * @param availableRows Available rows.
   * @param idField Optional id field to read from rows.
   * @return True when all rows are selected.
   */
  bool isAllSelected(const QVariantList& selectedIds,
                     const QVariantList& availableRows,
                     const QString& idField = {}) const;
  /**
   * @brief Returns selected property ids or all ids when selection is empty.
   * @return Effective property ids.
   */
  QStringList effectiveSelectedPropertyIds() const;
  /**
   * @brief Returns selected contract types or all types when selection is empty.
   * @return Effective contract type keys.
   */
  QStringList effectiveSelectedContractTypes() const;
  /**
   * @brief Builds the current filter specification through the workflow.
   * @return Filter specification payload.
   */
  QString currentFilterSpec() const;
  /**
   * @brief Normalizes an export format for the current UI type.
   * @param value Raw export format.
   * @param uiType UI type key.
   * @return Normalized export format.
   */
  QString normalizedExportFormat(const QString& value,
                                 const QString& uiType) const;
  /**
   * @brief Refreshes filter option rows from workspace and workflow data.
   */
  void refreshFilterRows();
  /**
   * @brief Clears adjustment form state.
   */
  void resetAdjustments();
  /**
   * @brief Rebuilds adjustment amounts from the current adjustment selection.
   */
  void refreshAdjustmentAmountsFromSelection();
  /**
   * @brief Runs the selected analysis and stores the result payload.
   */
  void refreshAnalysisResult();
  /**
   * @brief Recomputes the current preview through the analysis workflow.
   */
  void refreshPreview();
  /**
   * @brief Schedules a debounced preview refresh.
   */
  void requestPreviewRefresh();
  /**
   * @brief Loads edit state for the selected analysis.
   */
  void loadSelectedAnalysis();
  /**
   * @brief Loads default create-mode state.
   */
  void loadCreateState();
  /**
   * @brief Projects the current analysis result into QML fields.
   */
  void updateResultState();
  /**
   * @brief Emits the shared changed signal.
   */
  void emitChanged();
  /**
   * @brief Connects workspace signals.
   * @param value Workspace facade or nullptr.
   */
  void bindWorkspace(WorkspaceFacade* value);

  WorkspaceFacade* workspace_ = nullptr;
  AnalysisWorkflow* analysisWorkflow_ = nullptr;
  Settings* settings_ = nullptr;
  QTimer previewDebounce_;

  QString name_;
  int mainTypeIndex_ = 0;
  int plotSubtypeIndex_ = 0;
  QString exportFormat_;
  bool includeAdjustments_ = true;
  QVariantMap exportState_;
  QVariantList snapshotTransactions_;
  bool filterEditMode_ = true;
  int filterContentIndex_ = 0;
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
  QString adjustmentName_;
  QString adjustmentPercentText_;
  QString activeResultType_ = QStringLiteral("plot");
  QVariantMap currentAnalysisResult_;
  QString renderedPreviewSource_;
  int renderedPreviewRevision_ = 0;
  QVariantList tableContractTypes_;
  QVariantList tablePropertyRows_;
  double tableGrandTotal_ = 0.0;

};

} // namespace ui
