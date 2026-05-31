/**
 * @file ui/include/ui/state/session/ContractState.h
 * @brief Declares the UI contract state wrapper used by the Contract view.
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
 * @brief Owns the editable contract UI state and its selection-aware workflow.
 */
class ContractState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ContractState)
  QML_UNCREATABLE("ContractState is exposed by WorkspaceFacade")
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
  explicit ContractState(WorkspaceFacade *workspace, QObject *parent = nullptr);

  QString currentId() const;
  QString name() const;
  void setName(const QString &value);
  QString type() const;
  void setType(const QString &value);
  QString allocatableMode() const;
  void setAllocatableMode(const QString &value);

  QVariantList aliases() const;
  void setAliases(const QVariantList &value);
  QString aliasInputText() const;
  void setAliasInputText(const QString &value);
  int aliasIndex() const;
  void setAliasIndex(int value);

  QVariantList selectedActorIds() const;
  void setSelectedActorIds(const QVariantList &value);
  QVariantList selectedPropertyIds() const;
  void setSelectedPropertyIds(const QVariantList &value);
  QVariantList actorRows() const;
  QVariantList actorDisplayRows() const;
  QVariantList contractRows() const;
  QVariantList propertyRows() const;
  int selectedActorIndex() const;

  bool isEdit() const;
  bool hasChanges() const;
  bool canSubmit() const;
  Q_INVOKABLE bool canAddAlias(const QString &value) const;
  Q_INVOKABLE bool canRemoveSelectedAlias() const;
  Q_INVOKABLE bool isAliasSelected(int index) const;
  Q_INVOKABLE bool isPropertySelected(const QString &propertyId) const;

  Q_INVOKABLE void clear();
  Q_INVOKABLE void enterCreateMode();
  Q_INVOKABLE void addAlias(const QString &value);
  Q_INVOKABLE void removeAlias(int index);
  Q_INVOKABLE void selectAlias(int index);
  Q_INVOKABLE void selectContract(const QString &id);
  Q_INVOKABLE void requestRemoveSelectedAlias();
  Q_INVOKABLE void selectPrimaryActor(const QString &actorId);
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
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
  QString currentAllocatableMode() const;

  WorkspaceFacade *workspace_ = nullptr;
  QString currentOwnerId_;
  QString name_;
  QString type_;
  QString allocatableMode_ = QStringLiteral("mixed");
  QVariantList aliases_;
  QString aliasInputText_;
  int aliasIndex_ = -1;
  QVariantList selectedActorIds_;
  QVariantList selectedPropertyIds_;
  QString savedName_;
  QString savedType_;
  QString savedAllocatableMode_ = QStringLiteral("mixed");
  QVariantList savedAliases_;
  QVariantList savedSelectedActorIds_;
  QVariantList savedSelectedPropertyIds_;
  bool dirty_ = false;
};

} // namespace ui
