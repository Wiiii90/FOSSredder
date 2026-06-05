/**
 * @file ui/include/ui/viewmodels/ActorViewModel.h
 * @brief Declares the QML API for the Actor view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace ui {

class WorkspaceCommands;
class WorkspaceSelection;
class WorkspaceSelectors;
class WorkspaceStore;

/**
 * @brief Owns editable actor UI state and delegates actor CRUD to workspace.
 */
class ActorViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentId READ currentId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(QVariantList aliases READ aliases WRITE setAliases NOTIFY changed)
  Q_PROPERTY(QString aliasInputText READ aliasInputText WRITE setAliasInputText
                 NOTIFY changed)
  Q_PROPERTY(int aliasIndex READ aliasIndex WRITE setAliasIndex NOTIFY changed)
  Q_PROPERTY(QVariantList selectedContractIds READ selectedContractIds WRITE
                 setSelectedContractIds NOTIFY changed)
  Q_PROPERTY(QVariantList actorRows READ actorRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractRows READ contractRows NOTIFY changed)
  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)

public:
  /**
   * @brief Creates the actor view model bound to workspace roles.
   * @param parent Optional QObject parent.
   */
  explicit ActorViewModel(WorkspaceStore *store, WorkspaceCommands *commands,
                          WorkspaceSelection *selection,
                          WorkspaceSelectors *selectors,
                          QObject *parent = nullptr);

  /**
   * @brief Returns the currently selected actor id.
   * @return Selected actor id, or an empty string in create mode.
   */
  QString currentId() const;
  /**
   * @brief Returns the editable actor name.
   * @return Current form name.
   */
  QString name() const { return name_; }
  /**
   * @brief Updates the editable actor name.
   * @param value New form name.
   */
  void setName(const QString &value);
  /**
   * @brief Returns editable actor aliases.
   * @return Current alias list.
   */
  QVariantList aliases() const { return aliases_; }
  /**
   * @brief Replaces editable actor aliases.
   * @param value New alias list.
   */
  void setAliases(const QVariantList &value);
  /**
   * @brief Returns the current alias input text.
   * @return Current alias input text.
   */
  QString aliasInputText() const { return aliasInputText_; }
  /**
   * @brief Updates the current alias input text.
   * @param value New alias input text.
   */
  void setAliasInputText(const QString &value);
  /**
   * @brief Returns the selected alias index.
   * @return Selected alias index, or `-1` when no alias is selected.
   */
  int aliasIndex() const { return aliasIndex_; }
  /**
   * @brief Updates the selected alias index.
   * @param value New selected alias index.
   */
  void setAliasIndex(int value);
  /**
   * @brief Returns selected contract ids in the actor form.
   * @return Selected contract ids.
   */
  QVariantList selectedContractIds() const { return selectedContractIds_; }
  /**
   * @brief Replaces selected contract ids in the actor form.
   * @param value New selected contract ids.
   */
  void setSelectedContractIds(const QVariantList &value);
  /**
   * @brief Checks whether the form edits an existing actor.
   * @return `true` when an actor id is selected.
   */
  bool isEdit() const;
  /**
   * @brief Checks whether the editable form differs from the loaded actor.
   * @return `true` when the create form is submittable or an edit changed.
   */
  bool hasChanges() const;
  /**
   * @brief Checks whether the form has enough UI data to submit.
   * @return `true` when the actor name field is not blank.
   */
  bool canSubmit() const;

  /**
   * @brief Returns actor sidebar rows.
   * @return QML-ready actor rows from workspace selectors.
   */
  QVariantList actorRows() const;

  /**
   * @brief Returns selectable contract rows for the actor form.
   * @return QML-ready contract rows from workspace selectors.
   */
  QVariantList contractRows() const;

  /**
   * @brief Selects an actor for editing.
   * @param id Actor id selected by the QML sidebar.
   */
  Q_INVOKABLE void selectActor(const QString &id);
  /**
   * @brief Checks whether an alias value can be added to the form.
   * @param value Alias text to inspect.
   * @return `true` when the alias text is not blank.
   */
  Q_INVOKABLE bool canAddAlias(const QString &value) const;
  /**
   * @brief Adds an alias to the actor form.
   * @param value Alias text to add.
   */
  Q_INVOKABLE void addAlias(const QString &value);
  /**
   * @brief Removes the currently selected alias when possible.
   */
  Q_INVOKABLE void requestRemoveSelectedAlias();
  /**
   * @brief Clears selection and opens an empty create form.
   */
  Q_INVOKABLE void enterCreateMode();
  /**
   * @brief Selects the previous actor row.
   */
  Q_INVOKABLE void previous();
  /**
   * @brief Selects the next actor row.
   */
  Q_INVOKABLE void next();
  /**
   * @brief Creates or updates the actor through workspace commands.
   * @return Saved actor id, or an empty string when submission fails.
   */
  Q_INVOKABLE QString submit();
  /**
   * @brief Deletes the selected actor through workspace commands.
   */
  Q_INVOKABLE void deleteCurrent();
  /**
   * @brief Clears the editable actor form.
   */
  Q_INVOKABLE void clear();
  /**
   * @brief Adds or removes a contract id from the actor form selection.
   * @param contractId Contract id to toggle.
   * @param selected Whether the contract should be selected.
   */
  Q_INVOKABLE void setContractSelected(const QString &contractId,
                                       bool selected);

signals:
  /** @brief Emitted when actor view model state changed. */
  void changed();

private:
  /**
   * @brief Writes the selected actor id to the workspace selection state.
   * @param id Actor id, or empty to enter create mode.
   */
  void setCurrentId(const QString &id);

  /**
   * @brief Connects workspace and selection changes to form reloads.
   */
  void bindSignals();

  /**
   * @brief Reloads editable form state from the selected actor row.
   * @param forceReload Whether to reload even if the owner id did not change.
   */
  void reloadFromSelection(bool forceReload);
  /**
   * @brief Clears all editable form fields.
   */
  void clearFormState();
  /**
   * @brief Checks whether the alias index points to an existing alias.
   * @return `true` when the selected alias index points to an existing alias.
   */
  bool hasValidAliasSelection() const;
  /**
   * @brief Captures the current form state as the dirty-check baseline.
   */
  void captureSavedState();
  /**
   * @brief Applies a prepared form state to the editable fields.
   * @param state Form state projected from an actor row.
   */
  void applyFormState(const QVariantMap &state);
  /**
   * @brief Applies a reloaded form state when selection or workspace data changes.
   * @param forceReload Whether to reload even if the owner id did not change.
   * @param state Form state projected from the selected actor row.
   */
  void reloadFormState(bool forceReload, const QVariantMap &state);

  WorkspaceStore *store_ = nullptr;
  WorkspaceCommands *commands_ = nullptr;
  WorkspaceSelection *selection_ = nullptr;
  WorkspaceSelectors *selectors_ = nullptr;
  QString currentOwnerId_;
  QString name_;
  QVariantList aliases_;
  QString aliasInputText_;
  int aliasIndex_ = -1;
  QString savedName_;
  QVariantList savedAliases_;
  QVariantList selectedContractIds_;
  QVariantList savedSelectedContractIds_;
};

} // namespace ui
