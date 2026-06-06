/**
 * @file ui/tests/support/WorkspacePortFakes.h
 * @brief Port-only fakes used by UI boundary tests.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "core/ports/usecases/analysis/IAnalysisRunner.h"
#include "core/ports/usecases/annual/IAnnualRunner.h"
#include "core/ports/usecases/export/IExportRunner.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

namespace ui::tests::support {

class InMemoryWorkspace final
    : public core::ports::workspace::IWorkspaceReader,
      public core::ports::workspace::IWorkspaceWriter {
public:
  explicit InMemoryWorkspace(
      core::ports::workspace::WorkspaceSnapshot snapshot = {})
      : snapshot_(std::move(snapshot)) {}

  void replaceSnapshot(core::ports::workspace::WorkspaceSnapshot snapshot) {
    snapshot_ = std::move(snapshot);
    publish();
  }

  [[nodiscard]] const core::ports::workspace::WorkspaceSnapshot&
  snapshot() const noexcept {
    return snapshot_;
  }

  [[nodiscard]] const core::ports::workspace::WorkspaceSnapshot&
  savedSnapshot() const noexcept {
    return savedSnapshot_;
  }

  [[nodiscard]] core::ports::workspace::WorkspaceSnapshot
  workspaceSnapshot() const override {
    return snapshot_;
  }

  [[nodiscard]] std::string currentPath() const override {
    return snapshot_.hasCurrentPath ? snapshot_.currentPath : std::string{};
  }

  [[nodiscard]] std::optional<core::ports::workspace::StatementDraftSnapshot>
  statementDraftSnapshot(const std::string& draftId = {}) const override {
    for (const auto& draft : snapshot_.statementDrafts) {
      if (draftId.empty() || draft.id == draftId) {
        return draft;
      }
    }
    return std::nullopt;
  }

  [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot
  actorIdentityByName(const std::string& name) const override {
    const auto target = normalized(name);
    for (const auto& actor : snapshot_.actors) {
      if (normalized(actor.name) == target) {
        return identity(actor.id, actor.name);
      }
    }
    return {};
  }

  [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot
  propertyIdentityByName(const std::string& name) const override {
    const auto target = normalized(name);
    for (const auto& property : snapshot_.properties) {
      if (normalized(property.name) == target) {
        return identity(property.id, property.name);
      }
    }
    return {};
  }

  [[nodiscard]] core::ports::workspace::WorkspaceIdentitySnapshot
  contractIdentityBySignature(
      const std::string& name, const std::string& type,
      const std::vector<std::string>& actorIds,
      const std::vector<std::string>& propertyIds) const override {
    const auto targetName = normalized(name);
    const auto targetType = normalized(type);
    for (const auto& contract : snapshot_.contracts) {
      if (normalized(contract.name) != targetName ||
          normalized(contract.type) != targetType ||
          sorted(contract.actorIds) != sorted(actorIds) ||
          sorted(contract.propertyIds) != sorted(propertyIds)) {
        continue;
      }
      auto out = identity(contract.id, contract.name);
      out.type = contract.type;
      out.allocatableMode = contract.allocatableMode;
      out.actorIds = sorted(contract.actorIds);
      out.propertyIds = sorted(contract.propertyIds);
      return out;
    }
    return {};
  }

  [[nodiscard]] core::ports::workspace::TransactionCatalogSelection
  transactionCatalogSelection(
      const core::ports::workspace::TransactionCatalogSelectionChange& change)
      const override {
    core::ports::workspace::TransactionCatalogSelection out = change.current;
    if (change.contractChanged) {
      out.contractId = trimmed(change.contractId);
      const auto* contract = contractById(out.contractId);
      out.actorId = contract && !contract->actorIds.empty()
                        ? trimmed(contract->actorIds.front())
                        : std::string{};
      out.propertyIds = contract ? normalizedValues(contract->propertyIds)
                                 : std::vector<std::string>{};
    }
    if (change.actorChanged) {
      out.actorId = trimmed(change.actorId);
      const auto* contract = contractById(out.contractId);
      if (contract && !out.actorId.empty() &&
          !contains(contract->actorIds, out.actorId)) {
        out.contractId.clear();
      }
    }
    if (change.propertiesChanged) {
      out.propertyIds = normalizedValues(change.propertyIds);
      const auto* contract = contractById(out.contractId);
      if (contract && !containsAll(contract->propertyIds, out.propertyIds)) {
        out.contractId.clear();
      }
    }
    return out;
  }

  [[nodiscard]] std::string nextContractName() const override {
    return "Contract " + std::to_string(snapshot_.contracts.size() + 1);
  }

  void setSnapshotChangedCallback(SnapshotChanged cb) override {
    snapshotChanged_ = std::move(cb);
  }

  void
  setErrorReporter(std::shared_ptr<core::errors::IErrorReporter>) override {}

  void setAtomicStoreSave(
      core::ports::storage::IStorageManager::AtomicStoreSave) override {}

  void setAtomicStoreLoad(
      core::ports::storage::IStorageManager::AtomicStoreLoad) override {}

  void setDeletionImpactCallback(DeletionImpactCallback cb) override {
    deletionImpactCallback_ = std::move(cb);
  }

  void openLatest() override {
    publish();
  }

  void newFile(const std::string& path) override {
    snapshot_ = {};
    setPath(path);
    publish();
  }

  void openFile(const std::string& path) override {
    setPath(path);
    publish();
  }

  void saveFile() override {
    savedSnapshot_ = snapshot_;
  }

  void saveFileAs(const std::string& path) override {
    setPath(path);
    savedSnapshot_ = snapshot_;
    publish();
  }

  void commit() override {
    savedSnapshot_ = snapshot_;
    publish();
  }

  void notifySnapshot() override {
    publish();
  }

  [[nodiscard]] core::ports::workspace::ValidationResult
  validateActor(const core::ports::workspace::ActorCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult validateProperty(
      const core::ports::workspace::PropertyCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult validateContract(
      const core::ports::workspace::ContractCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult validateStatement(
      const core::ports::workspace::StatementCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult validateTransaction(
      const core::ports::workspace::TransactionCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult
  validateStatementWithTransactions(
      const core::ports::workspace::StatementWithTransactionsCommand&)
      const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult validateAnalysis(
      const core::ports::workspace::AnalysisCommand&) const override {
    return {};
  }

  [[nodiscard]] core::ports::workspace::ValidationResult
  validateAnnual(const core::ports::workspace::AnnualCommand&) const override {
    return {};
  }

  [[nodiscard]] std::string
  addActor(const core::ports::workspace::ActorCommand& command) override {
    core::ports::workspace::ActorSnapshot row;
    row.id =
        command.id.empty() ? nextId(snapshot_.actors, "actor") : command.id;
    row.name = command.name;
    row.aliases = command.aliases;
    row.contractIds = command.contractIds;
    snapshot_.actors.push_back(row);
    publish();
    return row.id;
  }

  void
  updateActor(const core::ports::workspace::ActorCommand& command) override {
    auto* row = findById(snapshot_.actors, command.id);
    if (!row) {
      static_cast<void>(addActor(command));
      return;
    }
    row->name = command.name;
    row->aliases = command.aliases;
    row->contractIds = command.contractIds;
    publish();
  }

  void deleteActor(const std::string& id) override {
    eraseById(snapshot_.actors, id);
    publish();
  }

  [[nodiscard]] std::string
  addProperty(const core::ports::workspace::PropertyCommand& command) override {
    core::ports::workspace::PropertySnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.properties, "property")
                                : command.id;
    row.name = command.name;
    row.aliases = command.aliases;
    row.contractIds = command.contractIds;
    snapshot_.properties.push_back(row);
    publish();
    return row.id;
  }

  void updateProperty(
      const core::ports::workspace::PropertyCommand& command) override {
    auto* row = findById(snapshot_.properties, command.id);
    if (!row) {
      static_cast<void>(addProperty(command));
      return;
    }
    row->name = command.name;
    row->aliases = command.aliases;
    row->contractIds = command.contractIds;
    publish();
  }

  void deleteProperty(const std::string& id) override {
    eraseById(snapshot_.properties, id);
    publish();
  }

  [[nodiscard]] std::string
  addContract(const core::ports::workspace::ContractCommand& command) override {
    core::ports::workspace::ContractSnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.contracts, "contract")
                                : command.id;
    row.name = command.name;
    row.type = command.type;
    row.allocatableMode = command.allocatableMode;
    row.actorIds = command.actorIds;
    row.propertyIds = command.propertyIds;
    row.aliases = command.aliases;
    snapshot_.contracts.push_back(row);
    publish();
    return row.id;
  }

  void updateContract(
      const core::ports::workspace::ContractCommand& command) override {
    auto* row = findById(snapshot_.contracts, command.id);
    if (!row) {
      static_cast<void>(addContract(command));
      return;
    }
    row->name = command.name;
    row->type = command.type;
    row->allocatableMode = command.allocatableMode;
    row->actorIds = command.actorIds;
    row->propertyIds = command.propertyIds;
    row->aliases = command.aliases;
    publish();
  }

  void deleteContract(const std::string& id) override {
    eraseById(snapshot_.contracts, id);
    publish();
  }

  [[nodiscard]] std::string addStatement(
      const core::ports::workspace::StatementCommand& command) override {
    core::ports::workspace::StatementSnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.statements, "statement")
                                : command.id;
    row.name = command.name;
    snapshot_.statements.push_back(row);
    publish();
    return row.id;
  }

  [[nodiscard]] std::string addStatementWithTransactions(
      const core::ports::workspace::StatementWithTransactionsCommand& command)
      override {
    const std::string statementId = addStatement(command.statement);
    if (statementId.empty()) {
      return {};
    }
    for (auto transaction : command.transactions) {
      transaction.statementId = statementId;
      static_cast<void>(addTransaction(transaction));
    }
    return statementId;
  }

  void updateStatement(
      const core::ports::workspace::StatementCommand& command) override {
    auto* row = findById(snapshot_.statements, command.id);
    if (!row) {
      static_cast<void>(addStatement(command));
      return;
    }
    row->name = command.name;
    publish();
  }

  void deleteStatement(const std::string& id) override {
    eraseById(snapshot_.statements, id);
    publish();
  }

  [[nodiscard]] std::string addTransaction(
      const core::ports::workspace::TransactionCommand& command) override {
    core::ports::workspace::TransactionSnapshot row;
    row.id =
        command.id.empty() ? nextId(snapshot_.transactions, "tx") : command.id;
    apply(command, row);
    snapshot_.transactions.push_back(row);
    attachTransactionToStatement(row.statementId, row.id,
                                 command.insertAfterTransactionId);
    publish();
    return row.id;
  }

  void updateTransaction(
      const core::ports::workspace::TransactionCommand& command) override {
    auto* row = findById(snapshot_.transactions, command.id);
    if (!row) {
      static_cast<void>(addTransaction(command));
      return;
    }
    const auto previousStatementId = row->statementId;
    apply(command, *row);
    if (previousStatementId != row->statementId) {
      detachTransactionFromStatements(row->id);
      attachTransactionToStatement(row->statementId, row->id, {});
    }
    publish();
  }

  void deleteTransaction(const std::string& id) override {
    eraseById(snapshot_.transactions, id);
    detachTransactionFromStatements(id);
    publish();
  }

  [[nodiscard]] std::string
  addAnalysis(const core::ports::workspace::AnalysisCommand& command) override {
    core::ports::workspace::AnalysisSnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.analyses, "analysis")
                                : command.id;
    apply(command, row);
    snapshot_.analyses.push_back(row);
    publish();
    return row.id;
  }

  void updateAnalysis(
      const core::ports::workspace::AnalysisCommand& command) override {
    auto* row = findById(snapshot_.analyses, command.id);
    if (!row) {
      static_cast<void>(addAnalysis(command));
      return;
    }
    apply(command, *row);
    publish();
  }

  void deleteAnalysis(const std::string& id) override {
    eraseById(snapshot_.analyses, id);
    publish();
  }

  [[nodiscard]] std::string
  addAnnual(const core::ports::workspace::AnnualCommand& command) override {
    core::ports::workspace::AnnualSnapshot row;
    row.id =
        command.id.empty() ? nextId(snapshot_.annuals, "annual") : command.id;
    apply(command, row);
    snapshot_.annuals.push_back(row);
    publish();
    return row.id;
  }

  void
  updateAnnual(const core::ports::workspace::AnnualCommand& command) override {
    auto* row = findById(snapshot_.annuals, command.id);
    if (!row) {
      static_cast<void>(addAnnual(command));
      return;
    }
    apply(command, *row);
    publish();
  }

  void deleteAnnual(const std::string& id) override {
    eraseById(snapshot_.annuals, id);
    publish();
  }

  [[nodiscard]] std::string finalizeStatementDraft(
      const core::ports::workspace::FinalizeStatementDraftCommand& command)
      override {
    core::ports::workspace::StatementCommand statement;
    statement.name = command.draft.name;
    const auto statementId = addStatement(statement);
    clearStatementDraft(command.draft.id);
    return statementId;
  }

  void saveStatementDraft(
      const core::ports::workspace::StatementDraftCommand& command) override {
    auto* row = findById(snapshot_.statementDrafts, command.draft.id);
    if (row) {
      *row = command.draft;
    } else {
      snapshot_.statementDrafts.push_back(command.draft);
    }
    publish();
  }

  void clearStatementDraft(const std::string& draftId = {}) override {
    if (draftId.empty()) {
      snapshot_.statementDrafts.clear();
    } else {
      eraseById(snapshot_.statementDrafts, draftId);
    }
    publish();
  }

  void saveImportLog(
      const core::ports::workspace::ImportLogCommand& command) override {
    auto* row = findById(snapshot_.importLogs, command.log.id);
    if (row) {
      *row = command.log;
    } else {
      snapshot_.importLogs.push_back(command.log);
    }
    publish();
  }

  void deleteImportLog(const std::string& id) override {
    eraseById(snapshot_.importLogs, id);
    publish();
  }

  void saveExportLog(
      const core::ports::workspace::ExportLogCommand& command) override {
    auto* row = findById(snapshot_.exportLogs, command.log.id);
    if (row) {
      *row = command.log;
    } else {
      snapshot_.exportLogs.push_back(command.log);
    }
    publish();
  }

  void deleteExportLog(const std::string& id) override {
    eraseById(snapshot_.exportLogs, id);
    publish();
  }

private:
  static std::string normalized(std::string value) {
    auto notSpace = [](unsigned char ch) {
      return !std::isspace(ch);
    };
    value.erase(value.begin(),
                std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(),
                value.end());
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) {
                     return static_cast<char>(std::tolower(ch));
                   });
    return value;
  }

  static std::vector<std::string> sorted(std::vector<std::string> values) {
    std::sort(values.begin(), values.end());
    return values;
  }

  static std::string trimmed(std::string value) {
    auto notSpace = [](unsigned char ch) {
      return !std::isspace(ch);
    };
    value.erase(value.begin(),
                std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(),
                value.end());
    return value;
  }

  static std::vector<std::string>
  normalizedValues(const std::vector<std::string>& values) {
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
      const auto next = trimmed(value);
      if (!next.empty()) {
        out.push_back(next);
      }
    }
    return out;
  }

  static bool contains(const std::vector<std::string>& values,
                       const std::string& value) {
    const auto target = trimmed(value);
    const auto normalized = normalizedValues(values);
    return !target.empty() && std::find(normalized.begin(), normalized.end(),
                                        target) != normalized.end();
  }

  static bool containsAll(const std::vector<std::string>& values,
                          const std::vector<std::string>& required) {
    const auto normalizedRequired = normalizedValues(required);
    return std::all_of(normalizedRequired.begin(), normalizedRequired.end(),
                       [&](const std::string& value) {
                         return contains(values, value);
                       });
  }

  static core::ports::workspace::WorkspaceIdentitySnapshot
  identity(const std::string& id, const std::string& name) {
    core::ports::workspace::WorkspaceIdentitySnapshot out;
    out.id = id;
    out.name = name;
    return out;
  }

  template<typename Row>
  static Row* findById(std::vector<Row>& rows, const std::string& id) {
    const auto it =
        std::find_if(rows.begin(), rows.end(), [&id](const Row& row) {
          return row.id == id;
        });
    return it == rows.end() ? nullptr : &*it;
  }

  template<typename Row>
  static const Row* findById(const std::vector<Row>& rows,
                             const std::string& id) {
    const auto it =
        std::find_if(rows.begin(), rows.end(), [&id](const Row& row) {
          return row.id == id;
        });
    return it == rows.end() ? nullptr : &*it;
  }

  const core::ports::workspace::ContractSnapshot*
  contractById(const std::string& id) const {
    return findById(snapshot_.contracts, trimmed(id));
  }

  template<typename Row>
  static void eraseById(std::vector<Row>& rows, const std::string& id) {
    rows.erase(std::remove_if(rows.begin(), rows.end(),
                              [&id](const Row& row) {
                                return row.id == id;
                              }),
               rows.end());
  }

  template<typename Row>
  static std::string nextId(const std::vector<Row>& rows,
                            const std::string& prefix) {
    int index = static_cast<int>(rows.size()) + 1;
    while (findById(rows, prefix + "-" + std::to_string(index))) {
      ++index;
    }
    return prefix + "-" + std::to_string(index);
  }

  void publish() {
    if (snapshotChanged_) {
      snapshotChanged_(snapshot_);
    }
    if (deletionImpactCallback_) {
      core::ports::workspace::DeletionImpact impact;
      deletionImpactCallback_(impact);
    }
  }

  void setPath(const std::string& path) {
    snapshot_.currentPath = path;
    snapshot_.hasCurrentPath = !path.empty();
  }

  void attachTransactionToStatement(const std::string& statementId,
                                    const std::string& transactionId,
                                    const std::string& afterTransactionId) {
    auto* statement = findById(snapshot_.statements, statementId);
    if (!statement) {
      return;
    }
    auto& ids = statement->transactionIds;
    ids.erase(std::remove(ids.begin(), ids.end(), transactionId), ids.end());
    const auto afterIt = std::find(ids.begin(), ids.end(), afterTransactionId);
    if (afterIt == ids.end()) {
      ids.push_back(transactionId);
    } else {
      ids.insert(afterIt + 1, transactionId);
    }
  }

  void detachTransactionFromStatements(const std::string& transactionId) {
    for (auto& statement : snapshot_.statements) {
      auto& ids = statement.transactionIds;
      ids.erase(std::remove(ids.begin(), ids.end(), transactionId), ids.end());
    }
  }

  static void apply(const core::ports::workspace::TransactionCommand& command,
                    core::ports::workspace::TransactionSnapshot& row) {
    row.id = command.id.empty() ? row.id : command.id;
    row.name = command.name;
    row.bookingDate = command.bookingDate;
    row.valuta = command.valuta;
    row.amount = amountValue(command.amountText);
    row.statementId = command.statementId;
    row.status = command.status;
    row.actorId = command.actorId;
    row.contractId = command.contractId;
    row.allocatable = command.allocatable;
    row.propertyIds = command.propertyIds;
  }

  static double amountValue(const std::string& text) {
    try {
      return std::stod(text);
    } catch (...) {
      return 0.0;
    }
  }

  static void apply(const core::ports::workspace::AnalysisCommand& command,
                    core::ports::workspace::AnalysisSnapshot& row) {
    row.id = command.id.empty() ? row.id : command.id;
    row.name = command.name;
    row.type = command.type;
    row.config = command.config;
    row.filter = command.filter;
    row.exportFormat = command.exportFormat;
    row.includeCalculationAdjustments = command.includeCalculationAdjustments;
    row.snapshotTransactions = command.snapshotTransactions;
    row.adjustments = command.adjustments;
  }

  static void apply(const core::ports::workspace::AnnualCommand& command,
                    core::ports::workspace::AnnualSnapshot& row) {
    row.id = command.id.empty() ? row.id : command.id;
    row.name = command.name;
    row.year = command.year;
    row.analysisIds = command.assignedAnalysisIds;
  }

  core::ports::workspace::WorkspaceSnapshot snapshot_;
  core::ports::workspace::WorkspaceSnapshot savedSnapshot_;
  SnapshotChanged snapshotChanged_;
  DeletionImpactCallback deletionImpactCallback_;
};

class FakeAnalysisRunner final : public core::ports::analysis::IAnalysisRunner {
public:
  [[nodiscard]] core::ports::analysis::AnalysisResult runAnalysis(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const core::ports::analysis::AnalysisRequest& request) const override {
    core::ports::analysis::AnalysisResult result;
    const auto* analysis = findAnalysis(workspace, request.analysisId);
    if (!analysis) {
      return result;
    }

    result.found = true;
    result.type = analysis->type;
    result.config = analysis->config;
    result.generatedAt = "2026-06-01T00:00:00Z";

    double total = 0.0;
    const auto rows = analysis->snapshotTransactions.empty()
                          ? previewTransactions(workspace, request.filter)
                                .transactions
                          : snapshotTransactionRows(workspace,
                                                    analysis->snapshotTransactions);
    for (const auto& row : rows) {
      core::ports::analysis::AnalysisTransaction transaction;
      transaction.id = row.id;
      transaction.name = row.name;
      transaction.bookingDate = row.bookingDate;
      transaction.amount = adjustedAmount(*analysis, row.id, row.amount);
      transaction.contractId = row.contractId;
      transaction.contractType = row.contractType;
      transaction.propertyIds = row.propertyIds;
      transaction.propertyNames = row.propertyNames;
      total += transaction.amount;

      result.table.push_back(
          {transaction.name, doubleText(std::abs(transaction.amount))});
      result.transactions.push_back(std::move(transaction));
    }

    result.metrics["transactionCount"] =
        static_cast<double>(result.transactions.size());
    result.metrics["amountSum"] = total;
    result.artifacts.push_back("data:image/png;base64," + doubleText(total));
    return result;
  }

  [[nodiscard]] core::ports::analysis::AnalysisPreviewResult
  previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const core::ports::analysis::AnalysisFilterSelection& filter)
      const override {
    core::ports::analysis::AnalysisPreviewResult result;
    std::unordered_set<std::string> statementIds;
    double amountSum = 0.0;

    for (const auto& transaction : workspace.transactions) {
      if (!filter.propertyIds.empty() &&
          !containsAllStrings(transaction.propertyIds, filter.propertyIds)) {
        continue;
      }
      if (!filter.contractTypes.empty()) {
        const auto* contract = findContract(workspace, transaction.contractId);
        const std::string contractType = contract ? contract->type : "";
        if (std::find(filter.contractTypes.begin(), filter.contractTypes.end(),
                      contractType) == filter.contractTypes.end()) {
          continue;
        }
      }
      if (filter.allocatableMode == "allocatable" &&
          !transaction.allocatable) {
        continue;
      }
      if ((filter.allocatableMode == "nonAllocatable" ||
           filter.allocatableMode == "non-allocatable") &&
          transaction.allocatable) {
        continue;
      }

      core::ports::analysis::AnalysisPreviewTransaction row;
      row.id = transaction.id;
      row.name = transaction.name;
      row.bookingDate = transaction.bookingDate;
      row.valuta = transaction.valuta;
      row.amount = transaction.amount;
      row.statementId = transaction.statementId;
      row.actorId = transaction.actorId;
      row.contractId = transaction.contractId;
      row.allocatable = transaction.allocatable;
      row.propertyIds = transaction.propertyIds;
      if (const auto* actor = findActor(workspace, transaction.actorId)) {
        row.actorName = actor->name;
      }
      if (const auto* statement =
              findStatement(workspace, transaction.statementId)) {
        row.statementName = statement->name;
      }
      if (const auto* contract =
              findContract(workspace, transaction.contractId)) {
        row.contractType = contract->type;
        row.contractName = contract->name;
      }
      for (const auto& propertyId : transaction.propertyIds) {
        if (const auto* property = findProperty(workspace, propertyId)) {
          row.propertyNames.push_back(property->name);
        }
      }
      if (!row.statementId.empty()) {
        statementIds.insert(row.statementId);
      }
      amountSum += row.amount;
      result.transactions.push_back(std::move(row));
    }

    result.metrics.transactionCount =
        static_cast<int>(result.transactions.size());
    result.metrics.statementCount = static_cast<int>(statementIds.size());
    result.metrics.amountSum = amountSum;
    return result;
  }

  [[nodiscard]] core::ports::analysis::AnalysisFilterSelection
  filterSelectionFromFields(const std::string& dateField,
                            const std::string& dateMode,
                            const std::string& year,
                            const std::string& dateFrom,
                            const std::string& dateTo,
                            const std::vector<std::string>& propertyIds,
                            const std::vector<std::string>& contractTypes,
                            const std::string& allocatableMode) const override {
    core::ports::analysis::AnalysisFilterSelection selection;
    selection.dateField = dateField;
    selection.dateMode = dateMode;
    selection.year = year;
    selection.dateFrom = dateFrom;
    selection.dateTo = dateTo;
    selection.propertyIds = propertyIds;
    selection.contractTypes = contractTypes;
    selection.allocatableMode = allocatableMode;
    return selection;
  }

  [[nodiscard]] core::ports::analysis::AnalysisFilterSelection
  parseFilterSpec(const std::string& filterSpec) const override {
    return core::ports::analysis::parseAnalysisFilterSelection(filterSpec);
  }

  [[nodiscard]] std::string buildFilterSpec(
      const core::ports::analysis::AnalysisFilterSelection& selection)
      const override {
    return core::ports::analysis::buildAnalysisFilterSpec(selection);
  }

  [[nodiscard]] std::string buildAnalysisConfigJson(
      const core::ports::analysis::AnalysisConfigInput&) const override {
    return "{}";
  }

  [[nodiscard]] core::ports::analysis::AnalysisAdjustmentAmounts
  buildAnalysisAdjustments(
      const std::vector<
          core::ports::analysis::AnalysisAdjustmentTransactionInput>&
          transactions,
      const std::vector<std::string>& selectedTransactionIds,
      double taxPercent) const override {
    core::ports::analysis::AnalysisAdjustmentAmounts out;
    const double factor = 1.0 + taxPercent / 100.0;
    for (const auto& transaction : transactions) {
      if (std::find(selectedTransactionIds.begin(),
                    selectedTransactionIds.end(),
                    transaction.id) != selectedTransactionIds.end()) {
        out.emplace_back(transaction.id, transaction.amount * factor);
      }
    }
    return out;
  }

  [[nodiscard]] std::optional<double>
  parseAnalysisPercentText(const std::string& text) const override {
    std::string normalized;
    normalized.reserve(text.size());
    for (const char ch : text) {
      if (ch == ',') {
        normalized.push_back('.');
      } else if (ch != '%' && !std::isspace(static_cast<unsigned char>(ch))) {
        normalized.push_back(ch);
      }
    }
    if (normalized.empty()) {
      return std::nullopt;
    }

    try {
      return std::stod(normalized);
    } catch (...) {
      return std::nullopt;
    }
  }

  void applyAnalysisPreviewOverrides(
      core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::string& analysisId, bool includeCalculationAdjustments,
      const core::ports::analysis::AnalysisAdjustmentAmounts& adjustments)
      const override {
    for (auto& analysis : workspace.analyses) {
      if (analysis.id == analysisId) {
        analysis.includeCalculationAdjustments = includeCalculationAdjustments;
        if (!adjustments.empty()) {
          analysis.adjustments = adjustments;
        }
        return;
      }
    }
  }

  [[nodiscard]] core::ports::analysis::AnalysisTableState
  projectTableState(const core::ports::analysis::AnalysisResult& result,
                    const core::ports::analysis::AnalysisAdjustmentAmounts&,
                    bool, const std::string& unassignedLabel) const override {
    core::ports::analysis::AnalysisTableState state;
    state.contractTypes.push_back(unassignedLabel);
    for (const auto& transaction : result.transactions) {
      core::ports::analysis::AnalysisTablePropertyRow row;
      row.propertyName = transaction.propertyNames.empty()
                             ? unassignedLabel
                             : transaction.propertyNames.front();
      row.amounts.push_back(transaction.amount);
      row.total = transaction.amount;
      state.grandTotal += transaction.amount;
      state.propertyRows.push_back(std::move(row));
    }
    return state;
  }

  [[nodiscard]] std::vector<std::string>
  contractTypes(const core::ports::workspace::WorkspaceSnapshot& workspace)
      const override {
    std::vector<std::string> out;
    std::unordered_set<std::string> seen;
    for (const auto& contract : workspace.contracts) {
      if (!contract.type.empty() && seen.insert(contract.type).second) {
        out.push_back(contract.type);
      }
    }
    return out;
  }

private:
  static const core::ports::workspace::AnalysisSnapshot*
  findAnalysis(const core::ports::workspace::WorkspaceSnapshot& workspace,
               const std::string& id) {
    const auto it =
        std::find_if(workspace.analyses.begin(), workspace.analyses.end(),
                     [&id](const auto& analysis) {
                       return analysis.id == id;
                     });
    return it == workspace.analyses.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::ContractSnapshot*
  findContract(const core::ports::workspace::WorkspaceSnapshot& workspace,
               const std::string& id) {
    const auto it =
        std::find_if(workspace.contracts.begin(), workspace.contracts.end(),
                     [&id](const auto& contract) {
                       return contract.id == id;
                     });
    return it == workspace.contracts.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::ActorSnapshot*
  findActor(const core::ports::workspace::WorkspaceSnapshot& workspace,
            const std::string& id) {
    const auto it =
        std::find_if(workspace.actors.begin(), workspace.actors.end(),
                     [&id](const auto& actor) {
                       return actor.id == id;
                     });
    return it == workspace.actors.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::StatementSnapshot*
  findStatement(const core::ports::workspace::WorkspaceSnapshot& workspace,
                const std::string& id) {
    const auto it =
        std::find_if(workspace.statements.begin(), workspace.statements.end(),
                     [&id](const auto& statement) {
                       return statement.id == id;
                     });
    return it == workspace.statements.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::PropertySnapshot*
  findProperty(const core::ports::workspace::WorkspaceSnapshot& workspace,
               const std::string& id) {
    const auto it =
        std::find_if(workspace.properties.begin(), workspace.properties.end(),
                     [&id](const auto& property) {
                       return property.id == id;
                     });
    return it == workspace.properties.end() ? nullptr : &*it;
  }

  static std::vector<core::ports::analysis::AnalysisPreviewTransaction>
  snapshotTransactionRows(
      const core::ports::workspace::WorkspaceSnapshot& workspace,
      const std::vector<core::ports::workspace::TransactionSnapshot>&
          transactions) {
    std::vector<core::ports::analysis::AnalysisPreviewTransaction> rows;
    rows.reserve(transactions.size());
    for (const auto& transaction : transactions) {
      core::ports::analysis::AnalysisPreviewTransaction row;
      row.id = transaction.id;
      row.name = transaction.name;
      row.bookingDate = transaction.bookingDate;
      row.valuta = transaction.valuta;
      row.amount = transaction.amount;
      row.statementId = transaction.statementId;
      row.actorId = transaction.actorId;
      row.contractId = transaction.contractId;
      row.contractType = transaction.contractType;
      row.allocatable = transaction.allocatable;
      row.propertyIds = transaction.propertyIds;
      row.propertyNames = transaction.propertyNames;
      if (row.contractType.empty()) {
        if (const auto* contract =
                findContract(workspace, transaction.contractId)) {
          row.contractType = contract->type;
          row.contractName = contract->name;
        }
      }
      if (const auto* actor = findActor(workspace, transaction.actorId)) {
        row.actorName = actor->name;
      }
      if (const auto* statement =
              findStatement(workspace, transaction.statementId)) {
        row.statementName = statement->name;
      }
      if (row.propertyNames.empty()) {
        for (const auto& propertyId : transaction.propertyIds) {
          if (const auto* property = findProperty(workspace, propertyId)) {
            row.propertyNames.push_back(property->name);
          }
        }
      }
      rows.push_back(std::move(row));
    }
    return rows;
  }

  static double
  adjustedAmount(const core::ports::workspace::AnalysisSnapshot& analysis,
                 const std::string& transactionId, double baseAmount) {
    if (!analysis.includeCalculationAdjustments) {
      return baseAmount;
    }
    for (const auto& [id, amount] : analysis.adjustments) {
      if (id == transactionId) {
        return amount;
      }
    }
    return baseAmount;
  }

  static bool containsAllStrings(const std::vector<std::string>& values,
                                 const std::vector<std::string>& required) {
    return std::all_of(required.begin(), required.end(),
                       [&](const std::string& value) {
                         return std::find(values.begin(), values.end(),
                                          value) != values.end();
                       });
  }

  static std::string doubleText(double value) {
    std::ostringstream out;
    out << value;
    return out.str();
  }
};

class FakeAnnualRunner final : public core::ports::annual::IAnnualRunner {
public:
  [[nodiscard]] core::ports::annual::AnnualResult
  runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
            const core::ports::annual::AnnualRequest& request) const override {
    core::ports::annual::AnnualResult result;
    const auto* annual = findAnnual(workspace, request.annualId);
    if (!annual) {
      return result;
    }

    result.annualId = annual->id;
    result.annualName = annual->name;
    const std::vector<std::string> analysisIds =
        request.previewAnalysisIds.empty() ? annual->analysisIds
                                          : request.previewAnalysisIds;
    result.year = request.previewYear == 0 ? annual->year
                                           : request.previewYear;
    result.stats.assignedAnalysisCount =
        static_cast<int>(analysisIds.size());
    result.stats.snapshotTransactionCount =
        static_cast<int>(analysisIds.size());

    if (analysisIds.size() <= 1) {
      result.deduplicated.push_back(
          row("tx-1", "Rent", 1250.0, analysisIds));
      result.workspaceOnly.push_back(row("tx-2", "Fees", -35.5, {}));
    } else {
      result.divergent.push_back(
          row("tx-1", "Rent", 1250.0, analysisIds));
      auto missing = row("tx-2", "Fees", -35.5, analysisIds);
      missing.missingLive = true;
      result.divergent.push_back(missing);
      result.missingLive.push_back(missing);
      result.stats.missingLive = 1;
    }
    return result;
  }

private:
  static const core::ports::workspace::AnnualSnapshot*
  findAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
             const std::string& id) {
    const auto it =
        std::find_if(workspace.annuals.begin(), workspace.annuals.end(),
                     [&id](const auto& annual) {
                       return annual.id == id;
                     });
    return it == workspace.annuals.end() ? nullptr : &*it;
  }

  static core::ports::annual::AnnualRowResult
  row(const std::string& id, const std::string& name, double amount,
      std::vector<std::string> sourceAnalysisIds) {
    core::ports::annual::AnnualRowResult row;
    row.key = id;
    row.transactionId = id;
    row.transactionName = name;
    row.bookingDate = id == "tx-1" ? "2026-01-05" : "2026-01-06";
    row.amount = amount;
    row.status = id == "tx-1" ? 2 : 1;
    row.allocatable = id == "tx-1";
    row.contractId = id == "tx-1" ? "contract-1" : "";
    row.contractType = id == "tx-1" ? "lease" : "";
    row.statementId = "statement-1";
    row.sourceAnalysisIds = std::move(sourceAnalysisIds);
    return row;
  }
};

class ExportRunnerStub final : public core::ports::exporting::IExportRunner {
public:
  [[nodiscard]] core::ports::exporting::ExportResult
  runExport(const core::ports::workspace::WorkspaceSnapshot&,
            core::ports::exporting::ExportRequest request) const override {
    return {true,
            core::ports::exporting::ExportStatus::Ok,
            request.format,
            request.outputPath,
            {},
            {}};
  }
};

} // namespace ui::tests::support
