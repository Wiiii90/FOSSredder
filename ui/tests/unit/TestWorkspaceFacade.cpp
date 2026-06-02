/**
 * @file ui/tests/unit/TestWorkspaceFacade.cpp
 * @brief Tests for the UI WorkspaceFacade boundary.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "support/WorkspacePortFakes.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceRowProjector.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/workspace/RowSelectionSupport.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace {

QStringList
aliasValues(const std::vector<core::ports::workspace::AliasSnapshot> &aliases) {
  QStringList values;
  for (const auto &alias : aliases) {
    values.push_back(QString::fromStdString(alias.value));
  }
  return values;
}

template <typename Row>
const Row *findById(const std::vector<Row> &items, const std::string &id) {
  const auto it = std::find_if(items.begin(), items.end(),
                               [&](const auto &item) { return item.id == id; });
  return it == items.end() ? nullptr : &*it;
}

} // namespace

namespace ui {

TEST(WorkspaceFacadeTest, LoadsTheWorkspaceProjectionAndSelectionIds) {
  tests::support::InMemoryWorkspace workspace;
  WorkspaceFacade facade(&workspace, &workspace);

  const int initialRevision = facade.dataRevision();
  facade.loadFromState(tests::support::makeWorkspaceSnapshot());
  auto &models = facade.cache()->models();
  EXPECT_GT(facade.dataRevision(), initialRevision);

  EXPECT_EQ(facade.actorRows().size(), 1);
  EXPECT_EQ(facade.propertyRows().size(), 1);
  EXPECT_EQ(facade.contractRows().size(), 1);
  EXPECT_EQ(models.statements().rowCount(), 1);
  EXPECT_EQ(models.transactions().rowCount(), 2);
  EXPECT_EQ(models.analyses().rowCount(), 1);
  EXPECT_EQ(models.annuals().rowCount(), 1);

  auto *selection = facade.selection();
  selection->setSelectedActorId(QStringLiteral("actor-1"));
  selection->setSelectedPropertyId(QStringLiteral("property-1"));
  selection->setSelectedContractId(QStringLiteral("contract-1"));
  selection->setSelectedStatementId(QStringLiteral("statement-1"));
  selection->setSelectedTransactionId(QStringLiteral("tx-1"));
  selection->setSelectedAnalysisId(QStringLiteral("analysis-1"));
  selection->setSelectedAnnualId(QStringLiteral("annual-1"));

  EXPECT_EQ(selection->selectedActorId(), QStringLiteral("actor-1"));
  EXPECT_EQ(selection->selectedPropertyId(), QStringLiteral("property-1"));
  EXPECT_EQ(selection->selectedContractId(), QStringLiteral("contract-1"));
  EXPECT_EQ(selection->selectedStatementId(), QStringLiteral("statement-1"));
  EXPECT_EQ(selection->selectedTransactionId(), QStringLiteral("tx-1"));
  EXPECT_EQ(selection->selectedAnalysisId(), QStringLiteral("analysis-1"));
  EXPECT_EQ(selection->selectedAnnualId(), QStringLiteral("annual-1"));

  const QVariantList transactionRows = buildStatementTransactionRows(
      *facade.cache(), QStringLiteral("statement-1"));
  ASSERT_EQ(transactionRows.size(), 2);
  EXPECT_EQ(
      transactionRows.at(0).toMap().value(QStringLiteral("id")).toString(),
      QStringLiteral("tx-1"));
  EXPECT_EQ(
      transactionRows.at(1).toMap().value(QStringLiteral("id")).toString(),
      QStringLiteral("tx-2"));
  const int txRow = models.transactions().findRowById(QStringLiteral("tx-1"));
  ASSERT_GE(txRow, 0);
  const QVariantMap txPayload = models.transactions().get(txRow);
  EXPECT_EQ(txPayload.value(QStringLiteral("id")).toString(),
            QStringLiteral("tx-1"));
  EXPECT_EQ(txPayload.value(QStringLiteral("contractId")).toString(),
            QStringLiteral("contract-1"));
  const QVariantList annualRows = buildAnnualRows(*facade.cache());
  ASSERT_EQ(annualRows.size(), 1);
  EXPECT_EQ(annualRows.first().toMap().value(QStringLiteral("id")).toString(),
            QStringLiteral("annual-1"));

  const QVariantList rows{
      QVariantMap{{QStringLiteral("id"), QStringLiteral("a")},
                  {QStringLiteral("display"), QStringLiteral("Alpha")}},
      QVariantMap{{QStringLiteral("id"), QStringLiteral("b")},
                  {QStringLiteral("display"), QStringLiteral("Beta")}},
      QVariantMap{{QStringLiteral("id"), QStringLiteral("c")},
                  {QStringLiteral("display"), QStringLiteral("Gamma")}}};
  const QVariantMap orderedSelection = ui::orderedSelectionState(
      rows, QVariantList{QStringLiteral("c"), QStringLiteral("a")}, 1,
      QStringLiteral("b"));
  EXPECT_EQ(orderedSelection.value(QStringLiteral("orderIds")).toList(),
            QVariantList({QStringLiteral("c"), QStringLiteral("a"),
                          QStringLiteral("b")}));
  EXPECT_EQ(orderedSelection.value(QStringLiteral("index")).toInt(), 2);
  EXPECT_EQ(orderedSelection.value(QStringLiteral("id")).toString(),
            QStringLiteral("b"));
  EXPECT_EQ(orderedSelection.value(QStringLiteral("currentId")).toString(),
            QStringLiteral("b"));

  EXPECT_EQ(workspace.currentPath(), std::string());
  facade.newFile(QStringLiteral("workspace-a.fr"));
  EXPECT_EQ(facade.currentPath(), QStringLiteral("workspace-a.fr"));
  EXPECT_EQ(workspace.currentPath(), std::string("workspace-a.fr"));
}

TEST(WorkspaceFacadeTest,
     RoutesMutationsThroughTheCoreBoundaryAndRefreshesUIRows) {
  tests::support::InMemoryWorkspace workspace;
  WorkspaceFacade facade(&workspace, &workspace);
  auto &models = facade.cache()->models();

  const int initialRevision = facade.dataRevision();
  EXPECT_EQ(facade.actorRows().size(), 0);

  const QString createdId =
      facade.saveActor(QString(), QStringLiteral("Second Actor"),
                       QStringList{QStringLiteral("Actor Two")});
  ASSERT_FALSE(createdId.isEmpty());
  EXPECT_GT(facade.dataRevision(), initialRevision);
  EXPECT_EQ(facade.actorRows().size(), 1);
  EXPECT_EQ(facade.actorRows()
                .at(0)
                .toMap()
                .value(payload::keys::common::kName)
                .toString(),
            QStringLiteral("Second Actor"));

  const QString thirdId =
      facade.saveActor(QString(), QStringLiteral("Third Actor"));
  ASSERT_FALSE(thirdId.isEmpty());
  EXPECT_EQ(facade.actorRows().size(), 2);
  const int revisionAfterSecondAdd = facade.dataRevision();

  facade.saveActor(thirdId, QStringLiteral("Third Actor Updated"));
  EXPECT_GT(facade.dataRevision(), revisionAfterSecondAdd);
  EXPECT_EQ(facade.actorRows()
                .at(1)
                .toMap()
                .value(payload::keys::common::kName)
                .toString(),
            QStringLiteral("Third Actor Updated"));

  const int revisionAfterUpdate = facade.dataRevision();
  facade.deleteActor(createdId);
  EXPECT_GT(facade.dataRevision(), revisionAfterUpdate);
  EXPECT_EQ(facade.actorRows().size(), 1);
  EXPECT_EQ(facade.actorRows()
                .at(0)
                .toMap()
                .value(payload::keys::common::kName)
                .toString(),
            QStringLiteral("Third Actor Updated"));

  const int revisionBeforeContract = facade.dataRevision();
  const QString contractId = facade.saveContract(
      QString(), QStringLiteral("Lease Agreement"), QStringLiteral("lease"));
  ASSERT_FALSE(contractId.isEmpty());
  EXPECT_GT(facade.dataRevision(), revisionBeforeContract);
  EXPECT_EQ(facade.contractRows().size(), 1);

  const int revisionBeforeContractDelete = facade.dataRevision();
  facade.deleteContract(contractId);
  EXPECT_GT(facade.dataRevision(), revisionBeforeContractDelete);
  EXPECT_EQ(facade.contractRows().size(), 0);
}

TEST(WorkspaceFacadeTest, PersistsActorAliasesAcrossSelectionChanges) {
  tests::support::InMemoryWorkspace workspace;
  WorkspaceFacade facade(&workspace, &workspace);

  facade.newFile(QStringLiteral("workspace-actor.fr"));

  const QString actorId =
      facade.saveActor(QString(), QStringLiteral("Main Actor"),
                       QStringList{QStringLiteral("Primary Actor"),
                                   QStringLiteral("Main Actor")});
  const QString otherActorId =
      facade.saveActor(QString(), QStringLiteral("Backup Actor"),
                       QStringList{QStringLiteral("Backup Actor")});
  ASSERT_FALSE(actorId.isEmpty());
  ASSERT_FALSE(otherActorId.isEmpty());

  facade.selection()->setSelectedActorId(actorId);

  const QStringList updatedAliases{QStringLiteral("Primary Actor"),
                                   QStringLiteral("Main Actor"),
                                   QStringLiteral("Actor Fresh Alias")};

  const QString savedId = facade.saveActor(
      actorId, QStringLiteral("Main Actor"), updatedAliases, QStringList{});
  EXPECT_EQ(savedId, actorId);

  const auto snapshot = workspace.workspaceSnapshot();
  const auto savedActor = findById(snapshot.actors, actorId.toStdString());
  ASSERT_NE(savedActor, nullptr);
  EXPECT_EQ(aliasValues(savedActor->aliases), updatedAliases);

  const QVariantMap otherActor =
      rowById(buildActorRows(*facade.cache()), otherActorId);
  EXPECT_EQ(otherActor.value(payload::keys::actor::kAliases).toList(),
            QVariantList({QStringLiteral("Backup Actor")}));

  const QVariantMap updatedActor =
      rowById(buildActorRows(*facade.cache()), actorId);
  EXPECT_EQ(updatedActor.value(payload::keys::actor::kAliases).toList(),
            QVariantList({QStringLiteral("Primary Actor"),
                          QStringLiteral("Main Actor"),
                          QStringLiteral("Actor Fresh Alias")}));
}

TEST(WorkspaceFacadeTest, PersistsPropertyAliasesAcrossSelectionChanges) {
  tests::support::InMemoryWorkspace workspace;
  WorkspaceFacade facade(&workspace, &workspace);

  facade.newFile(QStringLiteral("workspace-property.fr"));

  const QString propertyId =
      facade.saveProperty(QString(), QStringLiteral("Primary Property"),
                          QStringList{QStringLiteral("Property Alias")});
  const QString otherPropertyId =
      facade.saveProperty(QString(), QStringLiteral("Backup Property"),
                          QStringList{QStringLiteral("Backup Property")});
  ASSERT_FALSE(propertyId.isEmpty());
  ASSERT_FALSE(otherPropertyId.isEmpty());

  facade.selection()->setSelectedPropertyId(propertyId);

  const QStringList updatedAliases{QStringLiteral("Property Alias"),
                                   QStringLiteral("Property Fresh Alias")};

  const QString savedId =
      facade.saveProperty(propertyId, QStringLiteral("Primary Property"),
                          updatedAliases, QStringList{});
  EXPECT_EQ(savedId, propertyId);

  const auto snapshot = workspace.workspaceSnapshot();
  const auto savedProperty =
      findById(snapshot.properties, propertyId.toStdString());
  ASSERT_NE(savedProperty, nullptr);
  EXPECT_EQ(aliasValues(savedProperty->aliases), updatedAliases);

  const QVariantMap otherProperty =
      rowById(buildPropertyRows(*facade.cache()), otherPropertyId);
  EXPECT_EQ(otherProperty.value(payload::keys::property::kAliases).toList(),
            QVariantList({QStringLiteral("Backup Property")}));

  const QVariantMap updatedProperty =
      rowById(buildPropertyRows(*facade.cache()), propertyId);
  EXPECT_EQ(updatedProperty.value(payload::keys::property::kAliases).toList(),
            QVariantList({QStringLiteral("Property Alias"),
                          QStringLiteral("Property Fresh Alias")}));
}

TEST(WorkspaceFacadeTest, PersistsContractAliasesAcrossSelectionChanges) {
  tests::support::InMemoryWorkspace workspace;
  WorkspaceFacade facade(&workspace, &workspace);

  facade.newFile(QStringLiteral("workspace-contract.fr"));

  const QString contractId = facade.saveContract(
      QString(), QStringLiteral("Lease Contract"), QStringLiteral("lease"),
      QStringList{}, QStringList{}, QStringList{QStringLiteral("Lease")});
  const QString otherContractId =
      facade.saveContract(QString(), QStringLiteral("Backup Contract"),
                          QStringLiteral("lease"), QStringList{}, QStringList{},
                          QStringList{QStringLiteral("Backup Contract")});
  ASSERT_FALSE(contractId.isEmpty());
  ASSERT_FALSE(otherContractId.isEmpty());

  facade.selection()->setSelectedContractId(contractId);

  const QStringList updatedAliases{QStringLiteral("Lease"),
                                   QStringLiteral("Lease Fresh Alias")};

  const QString savedId = facade.saveContract(
      contractId, QStringLiteral("Lease Contract"), QStringLiteral("lease"),
      QStringList{}, QStringList{}, updatedAliases);
  EXPECT_EQ(savedId, contractId);

  const auto snapshot = workspace.workspaceSnapshot();
  const auto savedContract =
      findById(snapshot.contracts, contractId.toStdString());
  ASSERT_NE(savedContract, nullptr);
  EXPECT_EQ(aliasValues(savedContract->aliases), updatedAliases);

  const QVariantMap otherContract =
      rowById(buildContractRows(*facade.cache()), otherContractId);
  EXPECT_EQ(otherContract.value(payload::keys::contract::kAliases).toList(),
            QVariantList({QStringLiteral("Backup Contract")}));

  const QVariantMap updatedContract =
      rowById(buildContractRows(*facade.cache()), contractId);
  EXPECT_EQ(updatedContract.value(payload::keys::contract::kAliases).toList(),
            QVariantList({QStringLiteral("Lease"),
                          QStringLiteral("Lease Fresh Alias")}));
}

} // namespace ui
