/**
 * @file core/src/application/analysis/AnalysisService.cpp
 * @brief Implements application-level analysis execution services.
 */

#include "core/application/analysis/AnalysisService.h"

#include "adjustment/AdjustmentCalculation.h"
#include "core/application/analysis/AnalysisWorkflowSupport.h"
#include "core/application/workspace/WorkspaceSnapshotCatalogMapper.h"
#include "core/constants/analysis.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Transaction.h"
#include "core/ports/usecases/analysis/AnalysisResult.h"
#include "core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h"
#include "internal/AnalysisFilter.h"
#include "presentation/PlotAnalysis.h"
#include "presentation/TableAnalysis.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <utility>

#include <nlohmann/json.hpp>

namespace core::application::analysis {

namespace {

double parseNumber(const std::string &value) {
  char *end = nullptr;
  const double parsed = std::strtod(value.c_str(), &end);
  return end != value.c_str() ? parsed : 0.0;
}

void appendUnique(std::vector<std::string> &values, const std::string &value) {
  if (value.empty() ||
      std::find(values.begin(), values.end(), value) != values.end()) {
    return;
  }
  values.push_back(value);
}

std::vector<core::ports::analysis::AnalysisTransaction>
projectAnalysisTransactions(
    const core::domain::catalog::WorkspaceCatalog &state,
    const std::vector<std::shared_ptr<core::domain::Transaction>>
        &transactions) {
  std::vector<core::ports::analysis::AnalysisTransaction> out;
  out.reserve(transactions.size());

  std::unordered_map<std::string, std::shared_ptr<core::domain::Contract>>
      contractById;
  contractById.reserve(state.contracts().size());
  for (const auto &contract : state.contracts()) {
    if (!contract) {
      continue;
    }
    contractById.emplace(contract->id(), contract);
  }

  std::unordered_map<std::string, std::string> propertyNameById;
  propertyNameById.reserve(state.properties().size());
  for (const auto &property : state.properties()) {
    if (!property) {
      continue;
    }
    propertyNameById.emplace(property->id(), property->name());
  }

  for (const auto &transaction : transactions) {
    if (!transaction) {
      continue;
    }

    core::ports::analysis::AnalysisTransaction projected;
    projected.id = transaction->id();
    projected.name = transaction->name();
    projected.bookingDate = transaction->bookingDate();
    projected.amount = transaction->amount();
    projected.contractId = transaction->contractId();
    projected.propertyIds = transaction->propertyIds();
    if (!transaction->contractId().empty()) {
      const auto it = contractById.find(transaction->contractId());
      if (it != contractById.end() && it->second) {
        projected.contractType = it->second->type();
        for (const auto &propertyId : it->second->propertyIds()) {
          if (std::find(projected.propertyIds.begin(),
                        projected.propertyIds.end(),
                        propertyId) == projected.propertyIds.end()) {
            projected.propertyIds.push_back(propertyId);
          }
        }
      }
    }
    projected.propertyNames.reserve(projected.propertyIds.size());
    for (const auto &propertyId : projected.propertyIds) {
      const auto nameIt = propertyNameById.find(propertyId);
      projected.propertyNames.push_back(nameIt != propertyNameById.end() &&
                                                !nameIt->second.empty()
                                            ? nameIt->second
                                            : propertyId);
    }
    out.push_back(std::move(projected));
  }

  return out;
}

std::shared_ptr<core::domain::catalog::WorkspaceCatalog> catalogWithAdjustments(
    const core::domain::catalog::WorkspaceCatalog &source,
    const std::unordered_map<std::string, double> &adjustments) {
  if (adjustments.empty()) {
    return {};
  }

  auto catalog = std::make_shared<core::domain::catalog::WorkspaceCatalog>();
  catalog->setContracts(source.contracts());
  catalog->setProperties(source.properties());
  catalog->setAnalyses(source.analyses());
  catalog->setStatements(source.statements());

  core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
  transactions.reserve(source.transactions().size());
  for (const auto &transaction : source.transactions()) {
    if (!transaction) {
      continue;
    }
    auto clone = std::make_shared<core::domain::Transaction>(
        transaction->name(), transaction->bookingDate(), transaction->valuta(),
        transaction->amount(), transaction->isAllocatable());
    clone->setId(transaction->id());
    clone->setStatementId(transaction->statementId());
    clone->setContractId(transaction->contractId());
    clone->setActorId(transaction->actorId());
    clone->setPropertyIds(transaction->propertyIds());
    clone->setStatus(transaction->status());
    clone->setCreatedAt(transaction->createdAt());
    clone->setUpdatedAt(transaction->updatedAt());
    const auto adjustmentIt = adjustments.find(transaction->id());
    if (adjustmentIt != adjustments.end()) {
      clone->setAmount(adjustmentIt->second);
    }
    transactions.push_back(std::move(clone));
  }
  catalog->setTransactions(std::move(transactions));
  return catalog;
}

std::string jsonString(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_string()) {
    return {};
  }
  return it->get<std::string>();
}

double jsonDouble(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_number()) {
    return 0.0;
  }
  return it->get<double>();
}

bool jsonBool(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_boolean()) {
    return false;
  }
  return it->get<bool>();
}

std::vector<std::string> jsonStringList(const nlohmann::json &object,
                                        const char *key);

core::ports::analysis::AnalysisConfigInput
analysisConfigFromJson(const std::string &raw, const std::string &type) {
  core::ports::analysis::AnalysisConfigInput out;
  out.type = type;
  if (raw.empty()) {
    return out;
  }
  nlohmann::json config;
  try {
    config = nlohmann::json::parse(raw);
  } catch (...) {
    return out;
  }
  if (!config.is_object()) {
    return out;
  }
  out.plotType = jsonString(config, core::constants::analysis::kPlotTypeKey.data());
  out.plotMeasure =
      jsonString(config, core::constants::analysis::kPlotMeasureKey.data());
  out.propertyIds =
      jsonStringList(config, core::constants::analysis::kPropertiesKey.data());
  out.contractTypes = jsonStringList(
      config, core::constants::analysis::kContractTypesKey.data());
  out.taxPercent =
      jsonDouble(config, core::constants::analysis::calculation::kPercentKey.data());
  return out;
}

std::vector<std::string> jsonStringList(const nlohmann::json &object,
                                        const char *key) {
  std::vector<std::string> out;
  const auto it = object.find(key);
  if (it == object.end() || !it->is_array()) {
    return out;
  }
  for (const auto &item : *it) {
    if (!item.is_string()) {
      continue;
    }
    const auto value = item.get<std::string>();
    if (!value.empty() &&
        std::find(out.begin(), out.end(), value) == out.end()) {
      out.push_back(value);
    }
  }
  return out;
}

std::shared_ptr<core::domain::catalog::WorkspaceCatalog>
catalogFromAnalysisSnapshot(const core::domain::Analysis &analysis) {
  if (!analysis.hasSnapshotTransactions()) {
    return {};
  }

  nlohmann::json rows;
  try {
    rows = nlohmann::json::parse(analysis.snapshotTransactionsJson());
  } catch (...) {
    return {};
  }
  if (rows.is_object()) {
    const auto transactions = rows.find("transactions");
    rows = transactions != rows.end() ? *transactions : nlohmann::json{};
  }
  if (!rows.is_array()) {
    return {};
  }

  auto catalog = std::make_shared<core::domain::catalog::WorkspaceCatalog>();
  core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
  core::domain::catalog::WorkspaceCatalog::ContractList contracts;
  core::domain::catalog::WorkspaceCatalog::PropertyList properties;
  std::unordered_set<std::string> seenContracts;
  std::unordered_set<std::string> seenProperties;

  for (std::size_t index = 0; index < rows.size(); ++index) {
    const auto &row = rows[index];
    if (!row.is_object()) {
      continue;
    }

    const std::string explicitId = jsonString(row, "id");
    const std::string transactionId = jsonString(row, "transactionId");
    const std::string id =
        !explicitId.empty()
            ? explicitId
            : (!transactionId.empty()
                   ? transactionId
                   : "snapshot-transaction-" + std::to_string(index));
    const std::string name = jsonString(row, "name").empty()
                                 ? jsonString(row, "transactionName")
                                 : jsonString(row, "name");
    const std::string bookingDate = jsonString(row, "date");
    const std::string valuta = jsonString(row, "valuta");
    const double amount = jsonDouble(row, "amount");
    const std::string contractId = jsonString(row, "contractId");
    const std::string contractType = jsonString(row, "contractType");
    const auto propertyIds = jsonStringList(row, "propertyIds");
    const auto propertyNames = jsonStringList(row, "propertyNames");

    auto transaction = std::make_shared<core::domain::Transaction>(
        name, bookingDate, valuta, amount, jsonBool(row, "allocatable"));
    transaction->setId(id);
    transaction->setStatementId(jsonString(row, "statementId"));
    transaction->setContractId(contractId);
    transaction->setPropertyIds(propertyIds);
    transactions.push_back(std::move(transaction));

    if (!contractId.empty() && seenContracts.insert(contractId).second) {
      auto contract = std::make_shared<core::domain::Contract>();
      contract->setId(contractId);
      contract->rename(jsonString(row, "contractName"));
      contract->setType(contractType);
      contract->setPropertyIds(propertyIds);
      contracts.push_back(std::move(contract));
    }

    for (std::size_t p = 0; p < propertyIds.size(); ++p) {
      const auto &propertyId = propertyIds[p];
      if (propertyId.empty() || !seenProperties.insert(propertyId).second) {
        continue;
      }
      auto property = std::make_shared<core::domain::Property>();
      property->setId(propertyId);
      property->rename(p < propertyNames.size() && !propertyNames[p].empty()
                           ? propertyNames[p]
                           : propertyId);
      properties.push_back(std::move(property));
    }
  }

  catalog->setTransactions(std::move(transactions));
  catalog->setContracts(std::move(contracts));
  catalog->setProperties(std::move(properties));
  return catalog;
}

std::filesystem::path analysisPreviewPath(
    const core::ports::analysis::AnalysisRequest &request,
    const core::ports::analysis::AnalysisResult &result) {
  std::ostringstream fingerprint;
  fingerprint << request.analysisId << '\0'
              << core::ports::analysis::buildAnalysisFilterSpec(request.filter)
              << '\0' << result.type << '\0' << result.config.plotType << '\0'
              << result.config.plotMeasure << '\0';
  for (const auto &row : result.table) {
    for (const auto &column : row) {
      fingerprint << column << '\0';
    }
    fingerprint << '\n';
  }

  const auto hash = std::hash<std::string>{}(fingerprint.str());
  std::ostringstream fileName;
  fileName << std::hex << hash << ".png";

  auto directory =
      std::filesystem::temp_directory_path() / "fossredder-analysis-preview";
  std::error_code error;
  std::filesystem::create_directories(directory, error);
  return directory / fileName.str();
}

} // namespace

AnalysisService::AnalysisService(
    std::shared_ptr<
        core::ports::analysis_image_renderer::IAnalysisImageRenderer>
        imageRenderer)
    : imageRenderer_(std::move(imageRenderer)) {}

core::ports::analysis::AnalysisResult AnalysisService::runAnalysis(
    const core::domain::catalog::WorkspaceCatalog &state,
    const core::ports::analysis::AnalysisRequest &request) const {
  return withRenderedArtifacts(
      request,
      runAnalysisById(
          state, request.analysisId,
          core::ports::analysis::buildAnalysisFilterSpec(request.filter)));
}

core::ports::analysis::AnalysisResult AnalysisService::runAnalysis(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const core::ports::analysis::AnalysisRequest &request) const {
  return runAnalysis(
      core::application::workspace::toWorkspaceCatalog(workspace), request);
}

core::ports::analysis::AnalysisResult AnalysisService::runAnalysisById(
    const core::domain::catalog::WorkspaceCatalog &state,
    const std::string &analysisId, const std::string &filterSpec) const {
  for (const auto &analysis : state.analyses()) {
    if (!analysis) {
      continue;
    }
    if (analysis->id() != analysisId) {
      continue;
    }

    const std::string effectiveFilter =
        filterSpec.empty() ? analysis->filterSpec() : filterSpec;
    return computeAnalysis(*analysis, state, effectiveFilter);
  }

  return core::ports::analysis::AnalysisResult{};
}

core::ports::analysis::AnalysisResult AnalysisService::computeAnalysis(
    const core::domain::Analysis &analysis,
    const core::domain::catalog::WorkspaceCatalog &state,
    const std::string &filterSpec) const {
  core::ports::analysis::AnalysisResult out;
  const auto frozenCatalog = catalogFromAnalysisSnapshot(analysis);
  const auto adjustedCatalog =
      analysis.includeCalculationAdjustments()
          ? catalogWithAdjustments(frozenCatalog ? *frozenCatalog : state,
                                   analysis.adjustments())
          : std::shared_ptr<core::domain::catalog::WorkspaceCatalog>{};
  const auto &sourceState = adjustedCatalog
                                ? *adjustedCatalog
                                : (frozenCatalog ? *frozenCatalog : state);
  const core::application::analysis::AnalysisFilter filter =
      core::application::analysis::parseAnalysisFilterSpec(filterSpec);

  const std::string key = analysis.executionType();

  if (key == core::constants::analysis::kTypePlot) {
    out = core::application::analysis::computePlotAnalysis(analysis,
                                                           sourceState, filter);
  } else if (key == core::constants::analysis::kTypeTab) {
    out = core::application::analysis::computeTableAnalysis(
        analysis, sourceState, filter);
  } else if (key == core::constants::analysis::kTypeCalculation) {
    out = core::application::analysis::computeAdjustmentAnalysis(
        analysis, sourceState, filter);
  } else {
    out = core::application::analysis::computeTableAnalysis(
        analysis, sourceState, filter);
  }

  out.type = analysis.outputType();
  out.config = analysisConfigFromJson(analysis.configJson(), analysis.type());
  out.transactions = core::application::analysis::projectAnalysisTransactions(
      sourceState, core::application::analysis::collectAnalysisTransactions(
                       sourceState, filter));
  out.generatedAt = analysis.updatedAt().empty() ? analysis.createdAt()
                                                 : analysis.updatedAt();
  out.found = true;
  return out;
}

core::ports::analysis::AnalysisResult AnalysisService::withRenderedArtifacts(
    const core::ports::analysis::AnalysisRequest &request,
    core::ports::analysis::AnalysisResult result) const {
  if (!imageRenderer_ || !result.found) {
    return result;
  }

  const auto outputPath = analysisPreviewPath(request, result);
  std::error_code error;
  if (std::filesystem::exists(outputPath, error) ||
      imageRenderer_->writeAnalysisImage(outputPath, request.analysisId,
                                         result)) {
    result.artifacts.push_back(outputPath.string());
  }
  return result;
}

std::vector<std::shared_ptr<core::domain::Transaction>>
AnalysisService::filterTransactions(
    const core::domain::catalog::WorkspaceCatalog &state,
    const std::string &filterSpec) const {
  return collectAnalysisTransactions(state,
                                     parseAnalysisFilterSpec(filterSpec));
}

core::ports::analysis::AnalysisPreviewResult
AnalysisService::previewTransactions(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const core::ports::analysis::AnalysisFilterSelection &filter) const {
  const auto state =
      core::application::workspace::toWorkspaceCatalog(workspace);
  const std::string filterSpec =
      core::ports::analysis::buildAnalysisFilterSpec(filter);

  std::unordered_map<std::string, std::shared_ptr<core::domain::Contract>>
      contractById;
  contractById.reserve(state.contracts().size());
  for (const auto &contract : state.contracts()) {
    if (!contract || contract->id().empty()) {
      continue;
    }
    contractById.emplace(contract->id(), contract);
  }

  std::unordered_map<std::string, std::string> propertyNameById;
  propertyNameById.reserve(state.properties().size());
  for (const auto &property : state.properties()) {
    if (!property || property->id().empty()) {
      continue;
    }
    propertyNameById.emplace(property->id(), property->name());
  }

  std::unordered_map<std::string, std::string> actorNameById;
  actorNameById.reserve(workspace.actors.size());
  for (const auto &actor : workspace.actors) {
    if (!actor.id.empty()) {
      actorNameById.emplace(actor.id, actor.name);
    }
  }

  std::unordered_map<std::string, std::string> statementNameById;
  statementNameById.reserve(workspace.statements.size());
  for (const auto &statement : workspace.statements) {
    if (!statement.id.empty()) {
      statementNameById.emplace(statement.id, statement.name);
    }
  }

  const auto filtered = filterTransactions(state, filterSpec);
  core::ports::analysis::AnalysisPreviewResult result;
  result.transactions.reserve(filtered.size());
  std::unordered_set<std::string> statementIds;
  double amountSum = 0.0;

  for (const auto &transaction : filtered) {
    if (!transaction) {
      continue;
    }

    core::ports::analysis::AnalysisPreviewTransaction row;
    row.id = transaction->id();
    row.name = transaction->name();
    row.bookingDate = transaction->bookingDate();
    row.valuta = transaction->valuta();
    row.amount = transaction->amount();
    row.statementId = transaction->statementId();
    row.actorId = transaction->actorId();
    row.contractId = transaction->contractId();
    row.allocatable = transaction->isAllocatable();
    row.propertyIds = transaction->propertyIds();

    const auto actorIt = actorNameById.find(row.actorId);
    if (actorIt != actorNameById.end()) {
      row.actorName = actorIt->second;
    }
    const auto statementIt = statementNameById.find(row.statementId);
    if (statementIt != statementNameById.end()) {
      row.statementName = statementIt->second;
    }

    const auto contractIt = contractById.find(row.contractId);
    if (contractIt != contractById.end() && contractIt->second) {
      row.contractName = contractIt->second->name();
      row.contractType = contractIt->second->type();
      for (const auto &propertyId : contractIt->second->propertyIds()) {
        if (std::find(row.propertyIds.begin(), row.propertyIds.end(),
                      propertyId) == row.propertyIds.end()) {
          row.propertyIds.push_back(propertyId);
        }
      }
    }

    row.propertyNames.reserve(row.propertyIds.size());
    for (const auto &propertyId : row.propertyIds) {
      const auto nameIt = propertyNameById.find(propertyId);
      row.propertyNames.push_back(nameIt != propertyNameById.end() &&
                                          !nameIt->second.empty()
                                      ? nameIt->second
                                      : propertyId);
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

core::ports::analysis::AnalysisFilterSelection
AnalysisService::filterSelectionFromFields(
    const std::string &dateField, const std::string &dateMode,
    const std::string &year, const std::string &dateFrom,
    const std::string &dateTo, const std::vector<std::string> &propertyIds,
    const std::vector<std::string> &contractTypes,
    const std::string &allocatableMode) const {
  return analysis::filterSelectionFromFields(
      dateField, dateMode, year, dateFrom, dateTo, propertyIds, contractTypes,
      allocatableMode);
}

core::ports::analysis::AnalysisFilterSelection
AnalysisService::parseFilterSpec(const std::string &filterSpec) const {
  return core::ports::analysis::parseAnalysisFilterSelection(filterSpec);
}

std::string AnalysisService::buildFilterSpec(
    const core::ports::analysis::AnalysisFilterSelection &selection) const {
  return core::ports::analysis::buildAnalysisFilterSpec(selection);
}

std::string AnalysisService::buildAnalysisConfigJson(
    const core::ports::analysis::AnalysisConfigInput &input) const {
  return analysis::buildAnalysisConfigJson(input);
}

core::ports::analysis::AnalysisAdjustmentAmounts
AnalysisService::buildAnalysisAdjustments(
    const std::vector<core::ports::analysis::AnalysisAdjustmentTransactionInput>
        &transactions,
    const std::vector<std::string> &selectedTransactionIds,
    double taxPercent) const {
  std::vector<AnalysisAdjustmentTransactionInput> coreTransactions;
  coreTransactions.reserve(transactions.size());
  for (const auto &transaction : transactions) {
    coreTransactions.push_back({transaction.id, transaction.amount});
  }
  return analysis::parseAnalysisAdjustmentsJson(
      analysis::buildAnalysisAdjustmentsJson(coreTransactions,
                                             selectedTransactionIds,
                                             taxPercent));
}

std::optional<double>
AnalysisService::parseAnalysisPercentText(const std::string &text) const {
  return analysis::parseAnalysisPercentText(text);
}

void AnalysisService::applyAnalysisPreviewOverrides(
    core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::string &analysisId, bool includeCalculationAdjustments,
    const core::ports::analysis::AnalysisAdjustmentAmounts &adjustments) const {
  analysis::applyAnalysisPreviewOverrides(
      workspace, analysisId, includeCalculationAdjustments, adjustments);
}

core::ports::analysis::AnalysisTableState AnalysisService::projectTableState(
    const core::ports::analysis::AnalysisResult &result,
    const core::ports::analysis::AnalysisAdjustmentAmounts &adjustments,
    bool includeCalculationAdjustments,
    const std::string &unassignedLabel) const {
  core::ports::analysis::AnalysisTableState state;

  std::map<std::string, std::map<std::string, double>> amountsByProperty;
  std::map<std::string, double> totalsByProperty;

  auto appendAmount = [&](const std::string &transactionId,
                          const std::string &contractType,
                          const std::vector<std::string> &propertyNames,
                          double baseAmount) {
    const auto adjustmentIt =
        std::find_if(adjustments.begin(), adjustments.end(),
                     [&](const auto &adjustment) {
                       return adjustment.first == transactionId;
                     });
    const double amount = includeCalculationAdjustments &&
                                  adjustmentIt != adjustments.end()
                              ? adjustmentIt->second
                              : baseAmount;
    const std::string effectiveContract =
        contractType.empty() ? unassignedLabel : contractType;

    state.grandTotal += amount;
    appendUnique(state.contractTypes, effectiveContract);

    if (propertyNames.empty()) {
      amountsByProperty[unassignedLabel][effectiveContract] += amount;
      totalsByProperty[unassignedLabel] += amount;
      return;
    }
    for (const auto &propertyName : propertyNames) {
      const std::string effectiveProperty =
          propertyName.empty() ? unassignedLabel : propertyName;
      amountsByProperty[effectiveProperty][effectiveContract] += amount;
      totalsByProperty[effectiveProperty] += amount;
    }
  };

  if (!result.transactions.empty()) {
    for (const auto &transaction : result.transactions) {
      std::vector<std::string> propertyNames = transaction.propertyNames;
      if (propertyNames.empty()) {
        propertyNames = transaction.propertyIds;
      }
      appendAmount(transaction.id, transaction.contractType, propertyNames,
                   transaction.amount);
    }
  } else {
    for (std::size_t i = 0; i < result.table.size(); ++i) {
      const auto &row = result.table[i];
      if (row.size() < 3) {
        continue;
      }
      appendAmount("table-row-" + std::to_string(i), unassignedLabel,
                   {unassignedLabel}, parseNumber(row[2]));
    }
  }

  std::sort(state.contractTypes.begin(), state.contractTypes.end());

  for (const auto &[propertyName, amountsByContract] : amountsByProperty) {
    core::ports::analysis::AnalysisTablePropertyRow row;
    row.propertyName = propertyName;
    row.total = totalsByProperty[propertyName];
    row.amounts.reserve(state.contractTypes.size());
    for (const auto &contractType : state.contractTypes) {
      const auto it = amountsByContract.find(contractType);
      row.amounts.push_back(it != amountsByContract.end() ? it->second : 0.0);
    }
    state.propertyRows.push_back(std::move(row));
  }
  return state;
}

std::vector<std::string> AnalysisService::contractTypes(
    const core::ports::workspace::WorkspaceSnapshot &workspace) const {
  return analysis::contractTypesFromSnapshot(workspace);
}

} // namespace core::application::analysis
