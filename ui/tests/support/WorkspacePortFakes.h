/**
 * @file ui/tests/support/WorkspacePortFakes.h
 * @brief Port-only fakes used by UI boundary tests.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/ports/analysis/IAnalysisRunner.h"
#include "core/ports/annual/IAnnualRunner.h"
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

  [[nodiscard]] const core::ports::workspace::WorkspaceSnapshot &
  snapshot() const noexcept {
    return snapshot_;
  }

  [[nodiscard]] const core::ports::workspace::WorkspaceSnapshot &
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
  statementDraftSnapshot(const std::string &draftId = {}) const override {
    for (const auto &draft : snapshot_.statementDrafts) {
      if (draftId.empty() || draft.id == draftId) {
        return draft;
      }
    }
    return std::nullopt;
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

  void openLatest() override { publish(); }

  void newFile(const std::string &path) override {
    snapshot_ = {};
    setPath(path);
    publish();
  }

  void openFile(const std::string &path) override {
    setPath(path);
    publish();
  }

  void saveFile() override { savedSnapshot_ = snapshot_; }

  void saveFileAs(const std::string &path) override {
    setPath(path);
    savedSnapshot_ = snapshot_;
    publish();
  }

  void commit() override {
    savedSnapshot_ = snapshot_;
    publish();
  }

  void notifySnapshot() override { publish(); }

  [[nodiscard]] std::string
  addActor(const core::ports::workspace::ActorCommand &command) override {
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
  updateActor(const core::ports::workspace::ActorCommand &command) override {
    auto *row = findById(snapshot_.actors, command.id);
    if (!row) {
      static_cast<void>(addActor(command));
      return;
    }
    row->name = command.name;
    row->aliases = command.aliases;
    row->contractIds = command.contractIds;
    publish();
  }

  void deleteActor(const std::string &id) override {
    eraseById(snapshot_.actors, id);
    publish();
  }

  [[nodiscard]] std::string
  addProperty(const core::ports::workspace::PropertyCommand &command) override {
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
      const core::ports::workspace::PropertyCommand &command) override {
    auto *row = findById(snapshot_.properties, command.id);
    if (!row) {
      static_cast<void>(addProperty(command));
      return;
    }
    row->name = command.name;
    row->aliases = command.aliases;
    row->contractIds = command.contractIds;
    publish();
  }

  void deleteProperty(const std::string &id) override {
    eraseById(snapshot_.properties, id);
    publish();
  }

  [[nodiscard]] std::string
  addContract(const core::ports::workspace::ContractCommand &command) override {
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
      const core::ports::workspace::ContractCommand &command) override {
    auto *row = findById(snapshot_.contracts, command.id);
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

  void deleteContract(const std::string &id) override {
    eraseById(snapshot_.contracts, id);
    publish();
  }

  [[nodiscard]] std::string addStatement(
      const core::ports::workspace::StatementCommand &command) override {
    core::ports::workspace::StatementSnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.statements, "statement")
                                : command.id;
    row.name = command.name;
    snapshot_.statements.push_back(row);
    publish();
    return row.id;
  }

  void updateStatement(
      const core::ports::workspace::StatementCommand &command) override {
    auto *row = findById(snapshot_.statements, command.id);
    if (!row) {
      static_cast<void>(addStatement(command));
      return;
    }
    row->name = command.name;
    publish();
  }

  void deleteStatement(const std::string &id) override {
    eraseById(snapshot_.statements, id);
    publish();
  }

  [[nodiscard]] std::string addTransaction(
      const core::ports::workspace::TransactionCommand &command) override {
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
      const core::ports::workspace::TransactionCommand &command) override {
    auto *row = findById(snapshot_.transactions, command.id);
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

  void deleteTransaction(const std::string &id) override {
    eraseById(snapshot_.transactions, id);
    detachTransactionFromStatements(id);
    publish();
  }

  [[nodiscard]] std::string
  addAnalysis(const core::ports::workspace::AnalysisCommand &command) override {
    core::ports::workspace::AnalysisSnapshot row;
    row.id = command.id.empty() ? nextId(snapshot_.analyses, "analysis")
                                : command.id;
    apply(command, row);
    snapshot_.analyses.push_back(row);
    publish();
    return row.id;
  }

  void updateAnalysis(
      const core::ports::workspace::AnalysisCommand &command) override {
    auto *row = findById(snapshot_.analyses, command.id);
    if (!row) {
      static_cast<void>(addAnalysis(command));
      return;
    }
    apply(command, *row);
    publish();
  }

  void deleteAnalysis(const std::string &id) override {
    eraseById(snapshot_.analyses, id);
    publish();
  }

  [[nodiscard]] std::string
  addAnnual(const core::ports::workspace::AnnualCommand &command) override {
    core::ports::workspace::AnnualSnapshot row;
    row.id =
        command.id.empty() ? nextId(snapshot_.annuals, "annual") : command.id;
    apply(command, row);
    snapshot_.annuals.push_back(row);
    publish();
    return row.id;
  }

  void
  updateAnnual(const core::ports::workspace::AnnualCommand &command) override {
    auto *row = findById(snapshot_.annuals, command.id);
    if (!row) {
      static_cast<void>(addAnnual(command));
      return;
    }
    apply(command, *row);
    publish();
  }

  void deleteAnnual(const std::string &id) override {
    eraseById(snapshot_.annuals, id);
    publish();
  }

  [[nodiscard]] std::string finalizeStatementDraft(
      const core::ports::workspace::FinalizeStatementDraftCommand &command)
      override {
    core::ports::workspace::StatementCommand statement;
    statement.name = command.draft.name;
    const auto statementId = addStatement(statement);
    clearStatementDraft(command.draft.id);
    return statementId;
  }

  void saveStatementDraft(
      const core::ports::workspace::StatementDraftCommand &command) override {
    auto *row = findById(snapshot_.statementDrafts, command.draft.id);
    if (row) {
      *row = command.draft;
    } else {
      snapshot_.statementDrafts.push_back(command.draft);
    }
    publish();
  }

  void clearStatementDraft(const std::string &draftId = {}) override {
    if (draftId.empty()) {
      snapshot_.statementDrafts.clear();
    } else {
      eraseById(snapshot_.statementDrafts, draftId);
    }
    publish();
  }

  void setImportLogs(
      const core::ports::workspace::ImportLogsCommand &command) override {
    snapshot_.importLogs = command.logs;
    publish();
  }

  void saveImportLog(
      const core::ports::workspace::ImportLogCommand &command) override {
    auto *row = findById(snapshot_.importLogs, command.log.id);
    if (row) {
      *row = command.log;
    } else {
      snapshot_.importLogs.push_back(command.log);
    }
    publish();
  }

  void deleteImportLog(const std::string &id) override {
    eraseById(snapshot_.importLogs, id);
    publish();
  }

  void clearImportLogs() override {
    snapshot_.importLogs.clear();
    publish();
  }

  void setExportLogs(
      const core::ports::workspace::ExportLogsCommand &command) override {
    snapshot_.exportLogs = command.logs;
    publish();
  }

  void saveExportLog(
      const core::ports::workspace::ExportLogCommand &command) override {
    auto *row = findById(snapshot_.exportLogs, command.log.id);
    if (row) {
      *row = command.log;
    } else {
      snapshot_.exportLogs.push_back(command.log);
    }
    publish();
  }

  void deleteExportLog(const std::string &id) override {
    eraseById(snapshot_.exportLogs, id);
    publish();
  }

  void clearExportLogs() override {
    snapshot_.exportLogs.clear();
    publish();
  }

private:
  template <typename Row>
  static Row *findById(std::vector<Row> &rows, const std::string &id) {
    const auto it =
        std::find_if(rows.begin(), rows.end(),
                     [&id](const Row &row) { return row.id == id; });
    return it == rows.end() ? nullptr : &*it;
  }

  template <typename Row>
  static const Row *findById(const std::vector<Row> &rows,
                             const std::string &id) {
    const auto it =
        std::find_if(rows.begin(), rows.end(),
                     [&id](const Row &row) { return row.id == id; });
    return it == rows.end() ? nullptr : &*it;
  }

  template <typename Row>
  static void eraseById(std::vector<Row> &rows, const std::string &id) {
    rows.erase(std::remove_if(rows.begin(), rows.end(),
                              [&id](const Row &row) { return row.id == id; }),
               rows.end());
  }

  template <typename Row>
  static std::string nextId(const std::vector<Row> &rows,
                            const std::string &prefix) {
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

  void setPath(const std::string &path) {
    snapshot_.currentPath = path;
    snapshot_.hasCurrentPath = !path.empty();
  }

  void attachTransactionToStatement(const std::string &statementId,
                                    const std::string &transactionId,
                                    const std::string &afterTransactionId) {
    auto *statement = findById(snapshot_.statements, statementId);
    if (!statement) {
      return;
    }
    auto &ids = statement->transactionIds;
    ids.erase(std::remove(ids.begin(), ids.end(), transactionId), ids.end());
    const auto afterIt = std::find(ids.begin(), ids.end(), afterTransactionId);
    if (afterIt == ids.end()) {
      ids.push_back(transactionId);
    } else {
      ids.insert(afterIt + 1, transactionId);
    }
  }

  void detachTransactionFromStatements(const std::string &transactionId) {
    for (auto &statement : snapshot_.statements) {
      auto &ids = statement.transactionIds;
      ids.erase(std::remove(ids.begin(), ids.end(), transactionId), ids.end());
    }
  }

  static void apply(const core::ports::workspace::TransactionCommand &command,
                    core::ports::workspace::TransactionSnapshot &row) {
    row.id = command.id.empty() ? row.id : command.id;
    row.name = command.name;
    row.bookingDate = command.bookingDate;
    row.valuta = command.valuta;
    row.amount = command.amount;
    row.statementId = command.statementId;
    row.status = command.status;
    row.actorId = command.actorId;
    row.contractId = command.contractId;
    row.allocatable = command.allocatable;
    row.propertyIds = command.propertyIds;
  }

  static void apply(const core::ports::workspace::AnalysisCommand &command,
                    core::ports::workspace::AnalysisSnapshot &row) {
    row.id = command.id.empty() ? row.id : command.id;
    row.name = command.name;
    row.type = command.type;
    row.configJson = command.configJson;
    row.filterSpec = command.filterSpec;
    row.exportFormat = command.exportFormat;
    row.includeCalculationAdjustments = command.includeCalculationAdjustments;
    row.exportStateJson = command.exportStateJson;
    row.snapshotTransactionsJson = command.snapshotTransactionsJson;
    row.adjustments = command.adjustments;
  }

  static void apply(const core::ports::workspace::AnnualCommand &command,
                    core::ports::workspace::AnnualSnapshot &row) {
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
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const core::ports::analysis::AnalysisRequest &request) const override {
    core::ports::analysis::AnalysisResult result;
    const auto *analysis = findAnalysis(workspace, request.analysisId);
    if (!analysis) {
      return result;
    }

    result.found = true;
    result.type = analysis->type;
    result.configJson = analysis->configJson;
    result.generatedAt = "2026-06-01T00:00:00Z";

    double total = 0.0;
    auto rows = previewTransactions(workspace, request.filterSpecification);
    for (const auto &row : rows) {
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

  [[nodiscard]] std::vector<core::ports::analysis::AnalysisPreviewTransaction>
  previewTransactions(
      const core::ports::workspace::WorkspaceSnapshot &workspace,
      const std::string &filterSpec) const override {
    std::vector<core::ports::analysis::AnalysisPreviewTransaction> out;
    const bool requireLargeAmount =
        filterSpec.find("amount>=1000") != std::string::npos;

    for (const auto &transaction : workspace.transactions) {
      if (requireLargeAmount && transaction.amount < 1000.0) {
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
      if (const auto *contract =
              findContract(workspace, transaction.contractId)) {
        row.contractType = contract->type;
        row.contractName = contract->name;
      }
      for (const auto &propertyId : transaction.propertyIds) {
        if (const auto *property = findProperty(workspace, propertyId)) {
          row.propertyNames.push_back(property->name);
        }
      }
      out.push_back(std::move(row));
    }

    return out;
  }

private:
  static const core::ports::workspace::AnalysisSnapshot *
  findAnalysis(const core::ports::workspace::WorkspaceSnapshot &workspace,
               const std::string &id) {
    const auto it =
        std::find_if(workspace.analyses.begin(), workspace.analyses.end(),
                     [&id](const auto &analysis) { return analysis.id == id; });
    return it == workspace.analyses.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::ContractSnapshot *
  findContract(const core::ports::workspace::WorkspaceSnapshot &workspace,
               const std::string &id) {
    const auto it =
        std::find_if(workspace.contracts.begin(), workspace.contracts.end(),
                     [&id](const auto &contract) { return contract.id == id; });
    return it == workspace.contracts.end() ? nullptr : &*it;
  }

  static const core::ports::workspace::PropertySnapshot *
  findProperty(const core::ports::workspace::WorkspaceSnapshot &workspace,
               const std::string &id) {
    const auto it =
        std::find_if(workspace.properties.begin(), workspace.properties.end(),
                     [&id](const auto &property) { return property.id == id; });
    return it == workspace.properties.end() ? nullptr : &*it;
  }

  static double
  adjustedAmount(const core::ports::workspace::AnalysisSnapshot &analysis,
                 const std::string &transactionId, double baseAmount) {
    if (!analysis.includeCalculationAdjustments) {
      return baseAmount;
    }
    for (const auto &[id, amount] : analysis.adjustments) {
      if (id == transactionId) {
        return amount;
      }
    }
    return baseAmount;
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
  runAnnual(const core::ports::workspace::WorkspaceSnapshot &workspace,
            const core::ports::annual::AnnualRequest &request) const override {
    core::ports::annual::AnnualResult result;
    const auto *annual = findAnnual(workspace, request.annualId);
    if (!annual) {
      return result;
    }

    result.annualId = annual->id;
    result.annualName = annual->name;
    result.year = annual->year;
    result.stats.assignedAnalysisCount =
        static_cast<int>(annual->analysisIds.size());
    result.stats.snapshotTransactionCount =
        static_cast<int>(annual->analysisIds.size());

    if (annual->analysisIds.size() <= 1) {
      result.deduplicated.push_back(
          row("tx-1", "Rent", 1250.0, annual->analysisIds));
      result.workspaceOnly.push_back(row("tx-2", "Fees", -35.5, {}));
    } else {
      result.divergent.push_back(
          row("tx-1", "Rent", 1250.0, annual->analysisIds));
      result.divergent.push_back(
          row("tx-2", "Fees", -35.5, annual->analysisIds));
    }
    return result;
  }

private:
  static const core::ports::workspace::AnnualSnapshot *
  findAnnual(const core::ports::workspace::WorkspaceSnapshot &workspace,
             const std::string &id) {
    const auto it =
        std::find_if(workspace.annuals.begin(), workspace.annuals.end(),
                     [&id](const auto &annual) { return annual.id == id; });
    return it == workspace.annuals.end() ? nullptr : &*it;
  }

  static core::ports::annual::AnnualRowResult
  row(const std::string &id, const std::string &name, double amount,
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

} // namespace ui::tests::support
