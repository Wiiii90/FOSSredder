/**
 * @file ui/src/workspace/WorkspaceCacheModels.cpp
 * @brief Implementation of the UI WorkspaceCacheModels component.
 */

#include "ui/workspace/WorkspaceCacheModels.h"

#include <QHash>
#include <QString>
#include <QVariant>
#include <utility>

#include "ui/presentation/PayloadKeys.h"
#include "ui/presentation/PayloadMapper.h"
#include "ui/workspace/IndexedListModel.h"

namespace ui {

class ActorCatalogModel final
    : public models::IndexedListModel<core::ports::workspace::ActorSnapshot> {
  Q_OBJECT
  using Base = models::IndexedListModel<core::ports::workspace::ActorSnapshot>;

public:
  enum Roles { IdRole = Qt::UserRole + 1, NameRole };

  explicit ActorCatalogModel(QObject *parent = nullptr) : Base(parent) {}

  QVariant data(const QModelIndex &index, int role) const override {
    if (!index.isValid()) {
      return {};
    }
    const auto &actor = itemAtRow(index.row());
    if (!actor) {
      return {};
    }
    switch (role) {
    case IdRole:
      return QString::fromStdString(actor->id);
    case NameRole:
      return QString::fromStdString(actor->name);
    default:
      return {};
    }
  }

  QHash<int, QByteArray> roleNames() const override {
    return {{IdRole, payload::keys::common::kId.toUtf8()},
            {NameRole, payload::keys::common::kName.toUtf8()}};
  }

  void setActors(std::vector<core::ports::workspace::ActorSnapshot> actors) {
    setValueItems(std::move(actors));
  }

  const std::vector<std::shared_ptr<core::ports::workspace::ActorSnapshot>> &
  actors() const {
    return items();
  }

  int findRowById(const QString &id) const { return findIndexedRow(id); }
  void removeAt(int row) { removeItemAt(row); }
};

class PropertyCatalogModel final
    : public models::IndexedListModel<
          core::ports::workspace::PropertySnapshot> {
  Q_OBJECT
  using Base =
      models::IndexedListModel<core::ports::workspace::PropertySnapshot>;

public:
  enum Roles { IdRole = Qt::UserRole + 1, NameRole };

  explicit PropertyCatalogModel(QObject *parent = nullptr) : Base(parent) {}

  QVariant data(const QModelIndex &index, int role) const override {
    if (!index.isValid()) {
      return {};
    }
    const auto &property = itemAtRow(index.row());
    if (!property) {
      return {};
    }
    switch (role) {
    case IdRole:
      return QString::fromStdString(property->id);
    case NameRole:
      return QString::fromStdString(property->name);
    default:
      return {};
    }
  }

  QHash<int, QByteArray> roleNames() const override {
    return {{IdRole, payload::keys::common::kId.toUtf8()},
            {NameRole, payload::keys::common::kName.toUtf8()}};
  }

  void setProperties(
      std::vector<core::ports::workspace::PropertySnapshot> properties) {
    setValueItems(std::move(properties));
  }

  const std::vector<std::shared_ptr<core::ports::workspace::PropertySnapshot>> &
  properties() const {
    return items();
  }

  int findRowById(const QString &id) const { return findIndexedRow(id); }
  void removeAt(int row) { removeItemAt(row); }
};

class ContractCatalogModel final
    : public models::IndexedListModel<
          core::ports::workspace::ContractSnapshot> {
  Q_OBJECT
  using Base =
      models::IndexedListModel<core::ports::workspace::ContractSnapshot>;

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    TypeRole,
    AllocatableModeRole,
    ActorIdsRole,
    PropertyIdsRole
  };

  explicit ContractCatalogModel(QObject *parent = nullptr) : Base(parent) {}

  QVariant data(const QModelIndex &index, int role) const override {
    if (!index.isValid()) {
      return {};
    }
    const auto &contract = itemAtRow(index.row());
    if (!contract) {
      return {};
    }
    switch (role) {
    case IdRole:
      return QString::fromStdString(contract->id);
    case NameRole:
      return QString::fromStdString(contract->name);
    case TypeRole:
      return QString::fromStdString(contract->type);
    case AllocatableModeRole:
      return QString::fromStdString(contract->allocatableMode);
    case ActorIdsRole:
      return payload::mapper::toQStringList(contract->actorIds);
    case PropertyIdsRole:
      return payload::mapper::toQStringList(contract->propertyIds);
    default:
      return {};
    }
  }

  QHash<int, QByteArray> roleNames() const override {
    return {{IdRole, payload::keys::common::kId.toUtf8()},
            {NameRole, payload::keys::common::kName.toUtf8()},
            {TypeRole, payload::keys::common::kType.toUtf8()},
            {AllocatableModeRole, "allocatableMode"},
            {ActorIdsRole, payload::keys::contract::kActorIds.toUtf8()},
            {PropertyIdsRole,
             payload::keys::transaction::kPropertyIds.toUtf8()}};
  }

  void setContracts(
      std::vector<core::ports::workspace::ContractSnapshot> contracts) {
    setValueItems(std::move(contracts));
  }

  const std::vector<std::shared_ptr<core::ports::workspace::ContractSnapshot>> &
  contracts() const {
    return items();
  }

  int findRowById(const QString &id) const { return findIndexedRow(id); }
  void removeAt(int row) { removeItemAt(row); }
};

WorkspaceCacheModels::WorkspaceCacheModels(QObject *objectParent)
    : actors_(std::make_unique<ActorCatalogModel>(objectParent)),
      analyses_(std::make_unique<AnalysisList>(objectParent)),
      properties_(std::make_unique<PropertyCatalogModel>(objectParent)),
      contracts_(std::make_unique<ContractCatalogModel>(objectParent)),
      statements_(std::make_unique<StatementListModel>(objectParent)),
      transactions_(std::make_unique<TransactionListModel>(objectParent)),
      annuals_(std::make_unique<AnnualList>(objectParent)) {}

WorkspaceCacheModels::~WorkspaceCacheModels() = default;

void WorkspaceCacheModels::loadFromState(
    const core::ports::workspace::WorkspaceSnapshot &state) {
  QHash<QString, QString> contractTypes;
  contractTypes.reserve(static_cast<int>(state.contracts.size()));
  for (const auto &contract : state.contracts) {
    contractTypes.insert(QString::fromStdString(contract.id),
                         QString::fromStdString(contract.type));
  }

  setActors(state.actors);
  setProperties(state.properties);
  setContracts(state.contracts);
  transactions().setContractTypes(std::move(contractTypes), false);
  statements().setStatements(state.statements);
  transactions().setTransactions(state.transactions);
  analyses().setAnalyses(state.analyses);
  annuals().setAnnuals(state.annuals);
}

void WorkspaceCacheModels::refreshContractTypes() {
  QHash<QString, QString> contractTypes;
  const auto &rows = contractSnapshots();
  contractTypes.reserve(static_cast<int>(rows.size()));
  for (const auto &contract : rows) {
    if (!contract)
      continue;
    contractTypes.insert(QString::fromStdString(contract->id),
                         QString::fromStdString(contract->type));
  }
  transactions().setContractTypes(std::move(contractTypes));
}

QAbstractItemModel &WorkspaceCacheModels::actorModel() noexcept { return *actors_; }

QAbstractItemModel &WorkspaceCacheModels::propertyModel() noexcept {
  return *properties_;
}

QAbstractItemModel &WorkspaceCacheModels::contractModel() noexcept {
  return *contracts_;
}

void WorkspaceCacheModels::setActors(
    std::vector<core::ports::workspace::ActorSnapshot> actors) {
  actors_->setActors(std::move(actors));
}

void WorkspaceCacheModels::setProperties(
    std::vector<core::ports::workspace::PropertySnapshot> props) {
  properties_->setProperties(std::move(props));
}

void WorkspaceCacheModels::setContracts(
    std::vector<core::ports::workspace::ContractSnapshot> contracts) {
  contracts_->setContracts(std::move(contracts));
}

const std::vector<std::shared_ptr<core::ports::workspace::ActorSnapshot>> &
WorkspaceCacheModels::actorSnapshots() const noexcept {
  return actors_->actors();
}

const std::vector<std::shared_ptr<core::ports::workspace::PropertySnapshot>> &
WorkspaceCacheModels::propertySnapshots() const noexcept {
  return properties_->properties();
}

const std::vector<std::shared_ptr<core::ports::workspace::ContractSnapshot>> &
WorkspaceCacheModels::contractSnapshots() const noexcept {
  return contracts_->contracts();
}

int WorkspaceCacheModels::findActorRowById(const QString &id) const {
  return actors_->findRowById(id);
}

int WorkspaceCacheModels::findPropertyRowById(const QString &id) const {
  return properties_->findRowById(id);
}

int WorkspaceCacheModels::findContractRowById(const QString &id) const {
  return contracts_->findRowById(id);
}

void WorkspaceCacheModels::removeActorAt(int row) { actors_->removeAt(row); }

void WorkspaceCacheModels::removePropertyAt(int row) { properties_->removeAt(row); }

void WorkspaceCacheModels::removeContractAt(int row) { contracts_->removeAt(row); }

} // namespace ui

#include "WorkspaceCacheModels.moc"
