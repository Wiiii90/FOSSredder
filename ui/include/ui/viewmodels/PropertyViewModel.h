/**
 * @file ui/include/ui/viewmodels/PropertyViewModel.h
 * @brief Declares the UI property state wrapper used by the Property view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace ui {

class WorkspaceFacade;

/**
 * @brief Owns the editable property UI state and its selection-aware workflow.
 */
class PropertyViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentId READ currentId NOTIFY changed)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
  Q_PROPERTY(QVariantList aliases READ aliases WRITE setAliases NOTIFY changed)
  Q_PROPERTY(QString aliasInputText READ aliasInputText WRITE setAliasInputText
                 NOTIFY changed)
  Q_PROPERTY(int aliasIndex READ aliasIndex WRITE setAliasIndex NOTIFY changed)
  Q_PROPERTY(QVariantList selectedContractIds READ selectedContractIds WRITE
                 setSelectedContractIds NOTIFY changed)
  Q_PROPERTY(QVariantList propertyRows READ propertyRows NOTIFY changed)
  Q_PROPERTY(QVariantList contractRows READ contractRows NOTIFY changed)
  Q_PROPERTY(QString savedName READ savedName NOTIFY changed)
  Q_PROPERTY(QVariantList savedAliases READ savedAliases NOTIFY changed)
  Q_PROPERTY(QVariantList savedSelectedContractIds READ savedSelectedContractIds
                 NOTIFY changed)
  Q_PROPERTY(bool isEdit READ isEdit NOTIFY changed)
  Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY changed)
  Q_PROPERTY(bool canSubmit READ canSubmit NOTIFY changed)

public:
  /**
   * @brief Creates the property view model bound to the workspace facade.
   * @param workspace Workspace facade used for selection, rows, and CRUD.
   * @param parent Optional QObject parent.
   */
  explicit PropertyViewModel(WorkspaceFacade *workspace, QObject *parent = nullptr);

  /**
   * @brief Returns the currently selected property id.
   * @return Selected property id, or an empty string in create mode.
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
  QVariantList selectedContractIds() const { return selectedContractIds_; }
  void setSelectedContractIds(const QVariantList &value);
  QString savedName() const { return savedName_; }
  QVariantList savedAliases() const { return savedAliases_; }
  QVariantList savedSelectedContractIds() const {
    return savedSelectedContractIds_;
  }
  bool isEdit() const;
  bool hasChanges() const;
  bool canSubmit() const;

  /**
   * @brief Returns property sidebar rows.
   * @return QML-ready property rows from the workspace facade.
   */
  QVariantList propertyRows() const;

  /**
   * @brief Returns selectable contract rows for the property form.
   * @return QML-ready contract rows from the workspace facade.
   */
  QVariantList contractRows() const;

  /**
   * @brief Selects a property for editing.
   * @param id Property id selected by the QML sidebar.
   */
  Q_INVOKABLE void selectProperty(const QString &id);
  Q_INVOKABLE bool canAddAlias(const QString &value) const;
  Q_INVOKABLE bool canRemoveSelectedAlias() const;
  Q_INVOKABLE bool isAliasSelected(int index) const;
  Q_INVOKABLE void addAlias(const QString &value);
  Q_INVOKABLE void removeAlias(int index);
  Q_INVOKABLE void selectAlias(int index);
  Q_INVOKABLE void requestRemoveSelectedAlias();
  Q_INVOKABLE bool isContractSelected(const QString &contractId) const;
  Q_INVOKABLE void enterCreateMode();
  Q_INVOKABLE void previous();
  Q_INVOKABLE void next();
  Q_INVOKABLE QString submit();
  Q_INVOKABLE void deleteCurrent();
  Q_INVOKABLE void clear();
  Q_INVOKABLE void setContractSelected(const QString &contractId,
                                       bool selected);

signals:
  void changed();

private:
  /**
   * @brief Returns the primary row set used by inherited navigation.
   * @return Property rows.
   */
  QVariantList primaryRows() const;

  /**
   * @brief Writes the selected property id to the workspace selection state.
   * @param id Property id, or empty to enter create mode.
   */
  void setCurrentId(const QString &id);

  /**
   * @brief Creates or updates a property through the workspace facade.
   * @param id Existing property id, or empty for create.
   * @param name Property name.
   * @param aliases Property aliases.
   * @param contractIds Linked contract ids.
   * @return Saved property id.
   */
  QString saveLinkedCatalogObject(const QString &id, const QString &name,
                                  const QStringList &aliases,
                                  const QStringList &contractIds);

  /**
   * @brief Deletes a property through the workspace facade.
   * @param id Property id to delete.
   */
  void deleteLinkedCatalogObject(const QString &id);

  /**
   * @brief Connects workspace and selection changes to form reloads.
   */
  void bindSignals();

  /**
   * @brief Reloads editable form state from the selected property row.
   * @param forceReload Whether to reload even if the owner id did not change.
   */
  void reloadFromSelection(bool forceReload);
  void clearFormState();
  void captureSavedState();
  void applyFormState(const QVariantMap &state);
  void reloadLinkedFormState(bool forceReload, const QVariantMap &state);

  WorkspaceFacade *workspace_ = nullptr;
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
