/**
 * @file ui/include/ui/viewmodels/ContractViewModel.h
 * @brief Declares the QML API for the Contract view.
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
 * @brief Owns editable contract UI state and delegates contract CRUD to
 * workspace.
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
  Q_PROPERTY(QVariantList actorDisplayRows READ actorDisplayRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractRows READ contractRows NOTIFY changed)
  Q_PROPERTY(QVariantList propertyRows READ propertyRows NOTIFY changed)
  Q_PROPERTY(int selectedActorIndex READ selectedActorIndex NOTIFY changed)
  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)

public:
  /**
   * @brief Creates the contract view model bound to workspace roles.
   * @param parent Optional QObject parent.
   */
  explicit ContractViewModel(WorkspaceStore* store, WorkspaceCommands* commands,
                             WorkspaceSelection* selection,
                             WorkspaceSelectors* selectors,
                             QObject* parent = nullptr);

  /**
   * @brief Returns the currently selected contract id.
   * @return Selected contract id, or an empty string in create mode.
   */
  QString currentId() const;
  /**
   * @brief Returns the editable contract name.
   * @return Current form name.
   */
  QString name() const {
    return name_;
  }
  /**
   * @brief Updates the editable contract name.
   * @param value New form name.
   */
  void setName(const QString& value);
  /**
   * @brief Returns editable contract aliases.
   * @return Current alias list.
   */
  QVariantList aliases() const {
    return aliases_;
  }
  /**
   * @brief Replaces editable contract aliases.
   * @param value New alias list.
   */
  void setAliases(const QVariantList& value);
  /**
   * @brief Returns the current alias input text.
   * @return Current alias input text.
   */
  QString aliasInputText() const {
    return aliasInputText_;
  }
  /**
   * @brief Updates the current alias input text.
   * @param value New alias input text.
   */
  void setAliasInputText(const QString& value);
  /**
   * @brief Returns the selected alias index.
   * @return Selected alias index, or `-1` when no alias is selected.
   */
  int aliasIndex() const {
    return aliasIndex_;
  }
  /**
   * @brief Updates the selected alias index.
   * @param value New selected alias index.
   */
  void setAliasIndex(int value);

  /**
   * @brief Returns the editable contract type.
   * @return Current type field.
   */
  QString type() const;

  /**
   * @brief Updates the editable contract type.
   * @param value New type text from QML.
   */
  void setType(const QString& value);

  /**
   * @brief Returns the editable allocatable mode.
   * @return Normalized allocatable mode.
   */
  QString allocatableMode() const;

  /**
   * @brief Updates the editable allocatable mode.
   * @param value Mode text from QML.
   */
  void setAllocatableMode(const QString& value);

  /**
   * @brief Returns selected actor ids for this contract.
   * @return Actor id list.
   */
  QVariantList selectedActorIds() const;

  /**
   * @brief Replaces selected actor ids for this contract.
   * @param value Actor id list from QML.
   */
  void setSelectedActorIds(const QVariantList& value);

  /**
   * @brief Returns selected property ids for this contract.
   * @return Property id list.
   */
  QVariantList selectedPropertyIds() const;

  /**
   * @brief Replaces selected property ids for this contract.
   * @param value Property id list from QML.
   */
  void setSelectedPropertyIds(const QVariantList& value);

  /**
   * @brief Returns actor rows with an empty entry for the primary actor combo.
   * @return QML-ready display rows.
   */
  QVariantList actorDisplayRows() const;

  /**
   * @brief Returns contract sidebar rows.
   * @return QML-ready contract rows from workspace selectors.
   */
  QVariantList contractRows() const;

  /**
   * @brief Returns property rows for the contract property selector.
   * @return QML-ready property rows from workspace selectors.
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
  /**
   * @brief Checks whether an alias value can be added to the form.
   * @param value Alias text to inspect.
   * @return `true` when the alias text is not blank.
   */
  Q_INVOKABLE bool canAddAlias(const QString& value) const;
  /**
   * @brief Adds an alias to the contract form.
   * @param value Alias text to add.
   */
  Q_INVOKABLE void addAlias(const QString& value);
  /**
   * @brief Removes the currently selected alias when possible.
   */
  Q_INVOKABLE void requestRemoveSelectedAlias();

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
  Q_INVOKABLE void selectContract(const QString& id);

  /**
   * @brief Selects the primary actor for the contract.
   * @param actorId Actor id from the QML combo box, or empty.
   */
  Q_INVOKABLE void selectPrimaryActor(const QString& actorId);

  /**
   * @brief Toggles a property link in the current form.
   * @param propertyId Property id toggled by QML.
   * @param selected Whether the property should be selected.
   */
  Q_INVOKABLE void setPropertySelected(const QString& propertyId,
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
   * @brief Creates or updates the current contract through workspace commands.
   * @return Saved contract id.
   */
  Q_INVOKABLE QString submit();

  /**
   * @brief Deletes the selected contract through workspace commands.
   */
  Q_INVOKABLE void deleteCurrent();

signals:
  void changed();

private:
  /** @brief Connects workspace and selection changes to form reloads. */
  void bindSignals();

  /**
   * @brief Returns actor rows used to build the primary actor display list.
   * @return QML-ready actor rows from workspace selectors.
   */
  QVariantList actorRows() const;

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
  void applyFormState(const QVariantMap& state);

  /** @brief Clears editable contract-specific form fields. */
  void clearFormState();
  /**
   * @brief Checks whether the alias index points to an existing alias.
   * @return `true` when the selected alias index points to an existing alias.
   */
  bool hasValidAliasSelection() const;
  WorkspaceStore* store_ = nullptr;
  WorkspaceCommands* commands_ = nullptr;
  WorkspaceSelection* selection_ = nullptr;
  WorkspaceSelectors* selectors_ = nullptr;
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
