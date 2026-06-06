/**
 * @file ui/include/ui/viewmodels/AnnualViewModel.h
 * @brief Declares the QML API for the Annual view.
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
class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;

/**
 * @brief Owns Annual view form state and delegates annual CRUD and computation
 * to workspace and workflow.
 */
class AnnualViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(AnnualViewModel)

  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(QVariantList annualRows READ annualRows NOTIFY changed)
  Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(int year READ year WRITE setYear NOTIFY changed)
  Q_PROPERTY(QVariantList availableAnalysisRows READ availableAnalysisRows
                 NOTIFY changed)
  Q_PROPERTY(QVariantList assignedAnalysisRows READ assignedAnalysisRows NOTIFY
                 changed)
  Q_PROPERTY(int contentIndex READ contentIndex NOTIFY changed)
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
  /**
   * @brief Creates the annual view model.
   * @param parent Optional Qt parent object.
   */
  explicit AnnualViewModel(QObject* parent = nullptr);

  /**
   * @brief Sets workspace roles used for annual rows, selection and mutations.
   */
  void setWorkspaceRoles(WorkspaceStore* store, WorkspaceCommands* commands,
                         WorkspaceSelection* selection,
                         WorkspaceSelectors* selectors);

  /**
   * @brief Sets the workflow used for annual computation.
   * @param value Annual workflow or nullptr.
   */
  void setAnnualWorkflow(AnnualWorkflow* value);

  /**
   * @brief Returns whether an existing annual is selected.
   * @return True in edit mode, false in create mode.
   */
  bool isEdit() const;

  /**
   * @brief Returns annual rows for the sidebar.
   * @return Annual rows.
   */
  QVariantList annualRows() const;

  /**
   * @brief Returns the selected annual id.
   * @return Selected annual id or an empty string.
   */
  QString selectedAnnualId() const;

  /**
   * @brief Returns the annual name field.
   * @return Annual name.
   */
  QString name() const {
    return name_;
  }

  /**
   * @brief Updates the annual name field.
   * @param value Annual name.
   */
  void setName(const QString& value);

  /**
   * @brief Returns the selected annual year.
   * @return Year value.
   */
  int year() const noexcept {
    return year_;
  }

  /**
   * @brief Updates the selected annual year.
   * @param value Year value.
   */
  void setYear(int value);

  /**
   * @brief Returns ids of analyses assigned to the annual.
   * @return Analysis id list.
   */
  QVariantList analysisIds() const;

  /**
   * @brief Returns analyses available for assignment.
   * @return Available analysis rows.
   */
  QVariantList availableAnalysisRows() const {
    return availableAnalysisRows_;
  }

  /**
   * @brief Returns analyses assigned to the annual.
   * @return Assigned analysis rows.
   */
  QVariantList assignedAnalysisRows() const {
    return assignedAnalysisRows_;
  }

  /**
   * @brief Returns the active annual content stack index.
   * @return Content stack index.
   */
  int contentIndex() const noexcept {
    return contentIndex_;
  }

  /**
   * @brief Returns annual transaction preview rows.
   * @return Annual transaction rows.
   */
  QVariantList annualTransactions() const {
    return annualTransactions_;
  }

  /**
   * @brief Returns grouped annual transaction sections.
   * @return Section rows.
   */
  QVariantList transactionSections() const;

  /**
   * @brief Returns annual verification rows.
   * @return Verification rows.
   */
  QVariantList verificationRows() const;

  /**
   * @brief Returns the annual status summary text.
   * @return Status summary text.
   */
  QString statusSummaryText() const;

  /**
   * @brief Returns whether the current annual can be submitted.
   * @return True when quick UI checks pass.
   */
  bool canSubmit() const noexcept {
    return year_ > 0;
  }

  /**
   * @brief Returns whether annual rows exist.
   * @return True when the sidebar has rows.
   */
  bool hasRows() const;

  /**
   * @brief Returns whether form state differs from the saved annual state.
   * @return True when there are unsaved UI changes.
   */
  bool hasChanges() const;

  /**
   * @brief Reloads create or edit state from the workspace selection.
   */
  Q_INVOKABLE void refreshFromSelection();

  /**
   * @brief Resets the create form to defaults.
   */
  Q_INVOKABLE void resetCreateState();

  /**
   * @brief Creates an annual through workspace.
   */
  Q_INVOKABLE void submitCreate();

  /**
   * @brief Updates the selected annual through workspace.
   */
  Q_INVOKABLE void submitUpdate();

  /**
   * @brief Deletes the selected annual through workspace.
   */
  Q_INVOKABLE void deleteCurrent();

  /**
   * @brief Navigates between annual rows.
   * @param delta Relative movement, usually -1 or 1.
   */
  Q_INVOKABLE void navigate(int delta);

  /**
   * @brief Selects an annual row.
   * @param id Annual id or empty string for create mode.
   */
  Q_INVOKABLE void selectAnnual(const QString& id);

  /**
   * @brief Toggles between annual analysis assignment and transaction preview.
   */
  Q_INVOKABLE void toggleContent();

  /**
   * @brief Steps the selected year by a relative delta.
   * @param delta Year delta.
   */
  Q_INVOKABLE void stepYear(int delta);

  /**
   * @brief Adds an available analysis by row index.
   * @param index Index into availableAnalysisRows().
   */
  Q_INVOKABLE void addAvailableAnalysisAtIndex(int index);

  /**
   * @brief Removes an analysis from the annual.
   * @param id Analysis id.
   */
  Q_INVOKABLE void removeAnalysis(const QString& id);

  /**
   * @brief Updates export format for an assigned analysis.
   * @param id Analysis id.
   * @param exportFormat Export format key.
   */
  Q_INVOKABLE void setAnalysisExportFormat(const QString& id,
                                           const QString& exportFormat);

  /**
   * @brief Toggles expansion state for a transaction section.
   * @param key Section key.
   */
  Q_INVOKABLE void toggleTransactionSection(const QString& key);

signals:
  void changed();

private:
  /**
   * @brief Normalizes mixed analysis id payloads into stable ids.
   * @param values String, row, or list payload from workspace/QML.
   * @return Unique analysis ids.
   */
  QStringList normalizedAnalysisIds(const QVariant& values) const;
  /**
   * @brief Extracts an analysis id from a scalar or row payload.
   * @param value Input value.
   * @return Analysis id or empty string.
   */
  QString analysisIdFromVariant(const QVariant& value) const;
  /**
   * @brief Returns an annual row by id.
   * @param id Annual id.
   * @return Annual row or empty map.
   */
  QVariantMap annualRowById(const QString& id) const;
  /**
   * @brief Returns a normalized analysis row by id.
   * @param id Analysis id.
   * @return Analysis row or empty map.
   */
  QVariantMap analysisRowById(const QString& id) const;
  /**
   * @brief Normalizes a workspace analysis row for annual selection.
   * @param value Raw workspace row.
   * @return Normalized analysis row.
   */
  QVariantMap normalizeAnalysisRow(const QVariant& value) const;
  /**
   * @brief Returns normalized analysis rows from workspace.
   * @return Analysis rows.
   */
  QVariantList normalizedAnalysisRows() const;
  /**
   * @brief Builds rows for analyses assigned to the current annual.
   * @return Assigned analysis rows.
   */
  QVariantList buildAssignedAnalysisRows() const;
  /**
   * @brief Builds rows for analyses not assigned to the current annual.
   * @return Available analysis rows.
   */
  QVariantList buildAvailableAnalysisRows() const;
  /**
   * @brief Adds presentation metadata to an analysis row.
   * @param row Normalized analysis row.
   * @return Augmented row.
   */
  QVariantMap augmentAnalysisRow(const QVariantMap& row) const;
  /**
   * @brief Returns export format options for an analysis type.
   * @param type Analysis type.
   * @return Export option rows.
   */
  QVariantList exportOptionsForType(const QString& type) const;
  /**
   * @brief Resolves the selected export format index.
   * @param options Available export options.
   * @param exportFormat Selected export format.
   * @return Option index.
   */
  int exportFormatIndex(const QVariantList& options,
                        const QString& exportFormat) const;
  /**
   * @brief Normalizes an export format for an analysis type.
   * @param value Raw export format.
   * @param type Analysis type.
   * @return Normalized export format.
   */
  QString normalizedExportFormat(const QString& value,
                                 const QString& type) const;
  /**
   * @brief Returns the default annual year.
   * @return Default year.
   */
  int defaultYear() const;
  /**
   * @brief Loads default create-mode state.
   */
  void loadCreateState();
  /**
   * @brief Loads edit state for the selected annual.
   */
  void loadSelectedAnnual();
  /**
   * @brief Refreshes available and assigned analysis rows.
   */
  void refreshAnalysisSelectionRows();
  /**
   * @brief Recomputes annual preview/result state through the workflow.
   */
  void rebuildAnnualResultState();
  /**
   * @brief Applies an annual workflow result to presentation fields.
   * @param result Workflow result payload.
   */
  void applyAnnualResult(const QVariantMap& result);
  /**
   * @brief Captures current form state for dirty checks.
   */
  void captureSavedState();
  /**
   * @brief Replaces assigned analysis ids.
   * @param ids New analysis ids.
   */
  void setAnalysisIds(const QStringList& ids);
  /**
   * @brief Creates an empty verification issue payload.
   * @return Empty issue map.
   */
  QVariantMap emptyIssues() const;
  /**
   * @brief Creates an empty status metrics payload.
   * @return Empty status metrics map.
   */
  QVariantMap emptyStatusMetrics() const;
  /**
   * @brief Converts result bucket rows into QML rows.
   * @param rows Raw result rows.
   * @return QML transaction rows.
   */
  QVariantList rowsFromResultBucket(const QVariantList& rows) const;
  /**
   * @brief Adapts one annual transaction result row for QML.
   * @param source Source row.
   * @return Transaction row.
   */
  QVariantMap transactionRow(const QVariantMap& source) const;
  /**
   * @brief Builds one expandable transaction section.
   * @param key Section key.
   * @param title Section title.
   * @param rows Section rows.
   * @return Section payload.
   */
  QVariantMap transactionSection(const QString& key, const QString& title,
                                 const QVariantList& rows) const;
  /**
   * @brief Returns whether a transaction section is expanded.
   * @param key Section key.
   * @return True when expanded.
   */
  bool isTransactionSectionExpanded(const QString& key) const;
  /**
   * @brief Returns the current workspace revision.
   * @return Workspace revision or -1.
   */
  int workspaceRevision() const;
  /**
   * @brief Connects workspace role signals.
   */
  void bindWorkspaceRoles(WorkspaceStore* store, WorkspaceCommands* commands,
                          WorkspaceSelection* selection,
                          WorkspaceSelectors* selectors);
  /**
   * @brief Emits the shared changed signal.
   */
  void emitChanged();

  WorkspaceStore* store_ = nullptr;
  WorkspaceCommands* commands_ = nullptr;
  WorkspaceSelection* selection_ = nullptr;
  WorkspaceSelectors* selectors_ = nullptr;
  AnnualWorkflow* annualWorkflow_ = nullptr;
  QString name_;
  int year_ = 0;
  QStringList analysisIds_;
  QVariantList availableAnalysisRows_;
  QVariantList assignedAnalysisRows_;
  int contentIndex_ = 0;
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
