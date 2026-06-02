/**
 * @file ui/include/ui/viewmodels/ContractViewModel.h
 * @brief Declares the UI contract state wrapper used by the Contract view.
 */

#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QObject>

namespace ui {

class WorkspaceFacade;

/**
 * @brief Owns the editable contract UI state and its selection-aware workflow.
 */
class ContractViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentId READ currentId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(QString type READ type WRITE setType NOTIFY changed)
  Q_PROPERTY(QString allocatableMode READ allocatableMode WRITE
                 setAllocatableMode NOTIFY changed)
  Q_PROPERTY(QVariantList aliases READ aliases WRITE setAliases NOTIFY changed)
  Q_PROPERTY(QString aliasInputText READ aliasInputText WRITE setAliasInputText
                 NOTIFY changed)
  Q_PROPERTY(int aliasIndex READ aliasIndex WRITE setAliasIndex NOTIFY changed)
  Q_PROPERTY(QVariantList selectedActorIds READ selectedActorIds WRITE
                 setSelectedActorIds NOTIFY changed)
  Q_PROPERTY(QVariantList selectedPropertyIds READ selectedPropertyIds WRITE
                 setSelectedPropertyIds NOTIFY changed)
  Q_PROPERTY(QVariantList actorRows READ actorRows NOTIFY changed)
  Q_PROPERTY(QVariantList actorDisplayRows READ actorDisplayRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractRows READ contractRows NOTIFY changed)
  Q_PROPERTY(QVariantList propertyRows READ propertyRows NOTIFY changed)
  Q_PROPERTY(int selectedActorIndex READ selectedActorIndex NOTIFY changed)
  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)

public:
  /**
   * @brief Creates the contract view model bound to the workspace facade.
   * @param workspace Workspace facade used for selection, rows, and CRUD.
   * @param parent Optional QObject parent.
   */
  explicit ContractViewModel(WorkspaceFacade *workspace, QObject *parent = nullptr);

  /**
   * @brief Returns the currently selected contract id.
   * @return Selected contract id, or an empty string in create mode.
   */
  QString currentId() const;
  QString name() const { return name_; }
  void setName(const QString &value);
  QVariantList aliases() const { return aliases_; }
  void setAliases(const QVariantList &value);
  QString aliasInputText() const { return aliasInputText_; }
  void setAliasInputText(const QString &value);
  int aliasIndex() const { return aliasIndex_; }
  void setAliasIndex(int value);
  QString savedName() const { return savedName_; }
  QVariantList savedAliases() const { return savedAliases_; }

  /**
   * @brief Returns the editable contract type.
   * @return Current type field.
   */
  QString type() const;

  /**
   * @brief Updates the editable contract type.
   * @param value New type text from QML.
   */
  void setType(const QString &value);

  /**
   * @brief Returns the editable allocatable mode.
   * @return Normalized allocatable mode.
   */
  QString allocatableMode() const;

  /**
   * @brief Updates the editable allocatable mode.
   * @param value Mode text from QML.
   */
  void setAllocatableMode(const QString &value);

  /**
   * @brief Returns selected actor ids for this contract.
   * @return Actor id list.
   */
  QVariantList selectedActorIds() const;

  /**
   * @brief Replaces selected actor ids for this contract.
   * @param value Actor id list from QML.
   */
  void setSelectedActorIds(const QVariantList &value);

  /**
   * @brief Returns selected property ids for this contract.
   * @return Property id list.
   */
  QVariantList selectedPropertyIds() const;

  /**
   * @brief Replaces selected property ids for this contract.
   * @param value Property id list from QML.
   */
  void setSelectedPropertyIds(const QVariantList &value);

  /**
   * @brief Returns actor rows for the contract actor selector.
   * @return QML-ready actor rows from the workspace facade.
   */
  QVariantList actorRows() const;

  /**
   * @brief Returns actor rows with an empty entry for the primary actor combo.
   * @return QML-ready display rows.
   */
  QVariantList actorDisplayRows() const;

  /**
   * @brief Returns contract sidebar rows.
   * @return QML-ready contract rows from the workspace facade.
   */
  QVariantList contractRows() const;

  /**
   * @brief Returns property rows for the contract property selector.
   * @return QML-ready property rows from the workspace facade.
   */
  QVariantList propertyRows() const;

  /**
   * @brief Returns the selected primary actor combo index.
   * @return Combo index, or 0 when no actor is selected.
   */
  int selectedActorIndex() const;

  /**
   * @brief Reports whether the form edits an existing contract.
   * @return True when a contract id is selected.
   */
  bool isEdit() const;

  /**
   * @brief Reports whether the editable fields differ from saved state.
   * @return True when the form has unsaved changes.
   */
  bool hasChanges() const;

  /**
   * @brief Reports whether the current form can be submitted.
   * @return True when required contract fields are valid.
   */
  bool canSubmit() const;
  Q_INVOKABLE bool canAddAlias(const QString &value) const;
  Q_INVOKABLE bool canRemoveSelectedAlias() const;
  Q_INVOKABLE bool isAliasSelected(int index) const;
  Q_INVOKABLE void addAlias(const QString &value);
  Q_INVOKABLE void removeAlias(int index);
  Q_INVOKABLE void selectAlias(int index);
  Q_INVOKABLE void requestRemoveSelectedAlias();

  /**
   * @brief Checks whether a property id is selected.
   * @param propertyId Property id to check.
   * @return True when the property is selected.
   */
  Q_INVOKABLE bool isPropertySelected(const QString &propertyId) const;

  /**
   * @brief Clears the current contract form without changing workspace data.
   */
  Q_INVOKABLE void clear();

  /**
   * @brief Enters create mode and clears the contract form.
   */
  Q_INVOKABLE void enterCreateMode();

  /**
   * @brief Selects a contract for editing.
   * @param id Contract id selected by the QML sidebar.
   */
  Q_INVOKABLE void selectContract(const QString &id);

  /**
   * @brief Selects the primary actor for the contract.
   * @param actorId Actor id from the QML combo box, or empty.
   */
  Q_INVOKABLE void selectPrimaryActor(const QString &actorId);

  /**
   * @brief Toggles a property link in the current form.
   * @param propertyId Property id toggled by QML.
   * @param selected Whether the property should be selected.
   */
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
                                       bool selected);

  /**
   * @brief Selects the previous contract row.
   */
  Q_INVOKABLE void previous();

  /**
   * @brief Selects the next contract row.
   */
  Q_INVOKABLE void next();

  /**
   * @brief Creates or updates the current contract through the workspace facade.
   * @return Saved contract id.
   */
  Q_INVOKABLE QString submit();

  /**
   * @brief Deletes the selected contract through the workspace facade.
   */
  Q_INVOKABLE void deleteCurrent();

signals:
  void changed();

private:
  /** @brief Connects workspace and selection changes to form reloads. */
  void bindSignals();

  /**
   * @brief Reloads editable form state from the selected contract row.
   * @param forceReload Whether to reload even if the owner id did not change.
   */
  void reloadFromSelection(bool forceReload);

  /** @brief Captures the current form values as saved comparison state. */
  void captureSavedState();

  /**
   * @brief Applies a projected contract row to the editable form fields.
   * @param state Form state projected from the workspace row.
   */
  void applyFormState(const QVariantMap &state);

  /** @brief Clears editable contract-specific form fields. */
  void clearFormState();

  /**
   * @brief Reads the selected contract allocatable mode from workspace rows.
   * @return Normalized allocatable mode.
   */
  QString currentAllocatableMode() const;
  void clearBaseFormState();
  void captureBaseSavedState();
  void applyBaseFormState(const QVariantMap &state);

  WorkspaceFacade *workspace_ = nullptr;
  QString currentOwnerId_;
  QString name_;
  QVariantList aliases_;
  QString aliasInputText_;
  int aliasIndex_ = -1;
  QString savedName_;
  QVariantList savedAliases_;
  QString type_;
  QString allocatableMode_ = QStringLiteral("mixed");
  QVariantList selectedActorIds_;
  QVariantList selectedPropertyIds_;
  QString savedType_;
  QString savedAllocatableMode_ = QStringLiteral("mixed");
  QVariantList savedSelectedActorIds_;
  QVariantList savedSelectedPropertyIds_;
};

} // namespace ui
