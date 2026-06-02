/**
 * @file ui/include/ui/workspace/WorkspaceCacheModels.h
 * @brief Declarations for the UI WorkspaceCacheModels component.
 */

#pragma once

#include <memory>

#include "core/ports/workspace/WorkspaceSnapshot.h"
#include "ui/workspace/StatementViewModel.h"
#include "ui/workspace/TransactionViewModel.h"
#include "ui/workspace/AnalysisListModel.h"
#include "ui/workspace/AnnualListModel.h"

class QAbstractItemModel;
class QObject;
class QString;

namespace ui {

class ActorCatalogModel;
class ContractCatalogModel;
class PropertyCatalogModel;

class WorkspaceCacheModels {
public:
  explicit WorkspaceCacheModels(QObject *objectParent = nullptr);
  ~WorkspaceCacheModels();

  StatementViewModel &statements() noexcept { return *statements_; }
  TransactionViewModel &transactions() noexcept { return *transactions_; }
  AnalysisList &analyses() noexcept { return *analyses_; }
  AnnualList &annuals() noexcept { return *annuals_; }

  const StatementViewModel &statements() const noexcept { return *statements_; }
  const TransactionViewModel &transactions() const noexcept {
    return *transactions_;
  }
  const AnalysisList &analyses() const noexcept { return *analyses_; }
  const AnnualList &annuals() const noexcept { return *annuals_; }

  void loadFromState(const core::ports::workspace::WorkspaceSnapshot &state);
  void refreshContractTypes();

  QAbstractItemModel &actorModel() noexcept;
  QAbstractItemModel &propertyModel() noexcept;
  QAbstractItemModel &contractModel() noexcept;

  void setActors(std::vector<core::ports::workspace::ActorSnapshot> actors);
  void
  setProperties(std::vector<core::ports::workspace::PropertySnapshot> props);
  void setContracts(
      std::vector<core::ports::workspace::ContractSnapshot> contracts);

  const std::vector<std::shared_ptr<core::ports::workspace::ActorSnapshot>> &
  actorSnapshots() const noexcept;
  const std::vector<std::shared_ptr<core::ports::workspace::PropertySnapshot>> &
  propertySnapshots() const noexcept;
  const std::vector<std::shared_ptr<core::ports::workspace::ContractSnapshot>> &
  contractSnapshots() const noexcept;

  int findActorRowById(const QString &id) const;
  int findPropertyRowById(const QString &id) const;
  int findContractRowById(const QString &id) const;

  void removeActorAt(int row);
  void removePropertyAt(int row);
  void removeContractAt(int row);

private:
  std::unique_ptr<ActorCatalogModel> actors_;
  std::unique_ptr<AnalysisList> analyses_;
  std::unique_ptr<PropertyCatalogModel> properties_;
  std::unique_ptr<ContractCatalogModel> contracts_;
  std::unique_ptr<StatementViewModel> statements_;
  std::unique_ptr<TransactionViewModel> transactions_;
  std::unique_ptr<AnnualList> annuals_;
};

} // namespace ui
