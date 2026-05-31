/**
 * @file ui/include/ui/state/session/PropertyState.h
 * @brief Declares the UI property state wrapper used by the Property view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace ui {

class WorkspaceFacade;

/**
 * @brief Owns the editable property UI state and its selection-aware workflow.
 */
class PropertyState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(PropertyState)
  QML_UNCREATABLE("PropertyState is exposed by WorkspaceFacade")
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
  explicit PropertyState(WorkspaceFacade *workspace, QObject *parent = nullptr);

  QString currentId() const;
  QString name() const;
  void setName(const QString &value);

  QVariantList aliases() const;
  void setAliases(const QVariantList &value);

  QString aliasInputText() const;
  void setAliasInputText(const QString &value);

  int aliasIndex() const;
  void setAliasIndex(int value);

  QVariantList selectedContractIds() const;
  void setSelectedContractIds(const QVariantList &value);
  QVariantList propertyRows() const;
  QVariantList contractRows() const;

  QString savedName() const { return savedName_; }
  QVariantList savedAliases() const { return savedAliases_; }
  QVariantList savedSelectedContractIds() const {
    return savedSelectedContractIds_;
  }
  bool isEdit() const;
  bool hasChanges() const;
  bool canSubmit() const;
  Q_INVOKABLE bool canAddAlias(const QString &value) const;
  Q_INVOKABLE bool canRemoveSelectedAlias() const;
  Q_INVOKABLE bool isAliasSelected(int index) const;
  Q_INVOKABLE bool isContractSelected(const QString &contractId) const;

  Q_INVOKABLE void clear();
  Q_INVOKABLE void enterCreateMode();
  Q_INVOKABLE void addAlias(const QString &value);
  Q_INVOKABLE void removeAlias(int index);
  Q_INVOKABLE void selectAlias(int index);
  Q_INVOKABLE void selectProperty(const QString &id);
  Q_INVOKABLE void requestRemoveSelectedAlias();
  Q_INVOKABLE void setContractSelected(const QString &contractId,
                                       bool selected);
  Q_INVOKABLE void previous();
  Q_INVOKABLE void next();
  Q_INVOKABLE QString submit();
  Q_INVOKABLE void deleteCurrent();

signals:
  void changed();

private:
  void bindSignals();
  void reloadFromSelection(bool forceReload);
  void captureSavedState();
  void applyFormState(const QVariantMap &state);
  void clearFormState();

  WorkspaceFacade *workspace_ = nullptr;
  QString currentOwnerId_;
  QString name_;
  QVariantList aliases_;
  QString aliasInputText_;
  int aliasIndex_ = -1;
  QVariantList selectedContractIds_;
  QString savedName_;
  QVariantList savedAliases_;
  QVariantList savedSelectedContractIds_;
  bool dirty_ = false;
};

} // namespace ui
