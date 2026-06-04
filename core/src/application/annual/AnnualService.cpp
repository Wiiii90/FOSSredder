/**
 * @file core/src/application/annual/AnnualService.cpp
 * @brief Implements annual result read-model projection.
 */

#include "core/pch.h"

#include "core/application/annual/AnnualService.h"
#include "core/application/annual/AnnualWorkflowSupport.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace core::application::annual {

namespace annual_ports = core::ports::annual;

namespace {

struct SnapshotTx {
  std::string id;
  std::string name;
  std::string bookingDate;
  double amount = 0.0;
  bool allocatable = false;
  std::string contractId;
  std::string statementId;
  std::vector<std::string> propertyIds;
  std::string sourceAnalysisId;
  std::string sourceAnalysisName;
};

std::string jsonString(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_string())
    return {};
  return it->get<std::string>();
}

double jsonDouble(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_number())
    return 0.0;
  return it->get<double>();
}

bool jsonBool(const nlohmann::json &object, const char *key) {
  const auto it = object.find(key);
  if (it == object.end() || !it->is_boolean())
    return false;
  return it->get<bool>();
}

std::vector<std::string> jsonStringList(const nlohmann::json &object,
                                        const char *key) {
  std::vector<std::string> out;
  const auto it = object.find(key);
  if (it == object.end() || !it->is_array())
    return out;
  out.reserve(it->size());
  for (const auto &item : *it) {
    if (!item.is_string())
      continue;
    out.push_back(item.get<std::string>());
  }
  return out;
}

std::vector<nlohmann::json> parseSnapshotRows(const std::string &raw) {
  std::vector<nlohmann::json> out;
  if (raw.empty())
    return out;
  nlohmann::json parsed;
  try {
    parsed = nlohmann::json::parse(raw);
  } catch (...) {
    return out;
  }
  if (parsed.is_array()) {
    out.reserve(parsed.size());
    for (const auto &item : parsed)
      if (item.is_object())
        out.push_back(item);
    return out;
  }
  return out;
}

int bookingYear(const std::string &value) {
  if (value.size() < 4)
    return -1;

  auto parseYearAt = [&](size_t index) -> int {
    if (index + 4 > value.size())
      return -1;
    for (size_t i = index; i < index + 4; ++i) {
      if (value[i] < '0' || value[i] > '9')
        return -1;
    }
    try {
      const int year = std::stoi(value.substr(index, 4));
      return (year >= 1900 && year <= 2500) ? year : -1;
    } catch (...) {
      return -1;
    }
  };

  const int leading = parseYearAt(0);
  if (leading > 0)
    return leading;

  for (size_t i = 0; i + 4 <= value.size(); ++i) {
    const int year = parseYearAt(i);
    if (year > 0)
      return year;
  }
  return -1;
}

std::string exactKey(const SnapshotTx &tx) {
  return tx.bookingDate + "|" + std::to_string(tx.amount) + "|" +
         tx.contractId + "|" + tx.statementId;
}

} // namespace

annual_ports::AnnualResult AnnualService::runAnnual(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const annual_ports::AnnualRequest &request) const {
  if (!request.previewAnalysisIds.empty() || request.previewYear != 0) {
    const auto previewWorkspace =
        withPreviewAnnual(workspace, request.annualId,
                          request.previewAnalysisIds, request.previewYear);
    return sortAnnualResult(buildAnnualResult(previewWorkspace,
                                              request.annualId.empty()
                                                  ? std::string(kPreviewAnnualId)
                                                  : request.annualId));
  }
  return sortAnnualResult(buildAnnualResult(workspace, request.annualId));
}

annual_ports::AnnualResult AnnualService::buildAnnualResult(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const std::string &annualId) const {
  using namespace core::ports::workspace;

  annual_ports::AnnualResult out;
  out.annualId = annualId;

  const AnnualSnapshot *annual = nullptr;
  for (const auto &row : workspace.annuals) {
    if (row.id == annualId) {
      annual = &row;
      break;
    }
  }
  if (!annual)
    return out;

  out.annualName = annual->name;
  out.year = annual->year;
  out.stats.assignedAnalysisCount =
      static_cast<int>(annual->analysisIds.size());

  std::unordered_map<std::string, AnalysisSnapshot> analysisById;
  analysisById.reserve(workspace.analyses.size());
  for (const auto &analysis : workspace.analyses) {
    analysisById.emplace(analysis.id, analysis);
  }

  std::unordered_map<std::string, TransactionSnapshot> liveById;
  liveById.reserve(workspace.transactions.size());
  for (const auto &tx : workspace.transactions) {
    liveById.emplace(tx.id, tx);
  }

  std::unordered_map<std::string, std::string> contractTypeById;
  contractTypeById.reserve(workspace.contracts.size());
  for (const auto &contract : workspace.contracts) {
    contractTypeById.emplace(contract.id, contract.type);
  }

  std::vector<SnapshotTx> allSnapshotRows;
  for (const auto &analysisId : annual->analysisIds) {
    const auto it = analysisById.find(analysisId);
    if (it == analysisById.end())
      continue;
    const auto rows = parseSnapshotRows(it->second.snapshotTransactionsJson);
    for (const auto &row : rows) {
      SnapshotTx tx;
      tx.id = jsonString(row, "id");
      tx.name = jsonString(row, "transactionName");
      if (tx.name.empty())
        tx.name = jsonString(row, "name");
      tx.bookingDate = jsonString(row, "bookingDate");
      if (tx.bookingDate.empty())
        tx.bookingDate = jsonString(row, "date");
      tx.amount = jsonDouble(row, "amount");
      tx.allocatable = jsonBool(row, "allocatable");
      tx.contractId = jsonString(row, "contractId");
      tx.statementId = jsonString(row, "statementId");
      tx.propertyIds = jsonStringList(row, "propertyIds");
      tx.sourceAnalysisId = it->second.id;
      tx.sourceAnalysisName = it->second.name;
      allSnapshotRows.push_back(std::move(tx));
    }
  }
  out.stats.snapshotTransactionCount = static_cast<int>(allSnapshotRows.size());

  std::unordered_map<std::string, std::vector<SnapshotTx>> byExact;
  std::unordered_map<std::string, std::vector<SnapshotTx>> byDateContract;
  byExact.reserve(allSnapshotRows.size());
  byDateContract.reserve(allSnapshotRows.size());

  for (const auto &tx : allSnapshotRows) {
    byExact[exactKey(tx)].push_back(tx);
    byDateContract[tx.bookingDate + "|" + tx.contractId + "|" + tx.statementId]
        .push_back(tx);
  }

  auto buildRow = [&](const std::vector<SnapshotTx> &group, bool similar,
                      bool divergent) {
    annual_ports::AnnualRowResult row;
    if (group.empty())
      return row;
    const auto &base = group.front();
    row.key = similar
                  ? ("sim|" + exactKey(base))
                  : (divergent ? ("div|" + exactKey(base)) : exactKey(base));
    row.transactionId = base.id;
    row.transactionName = base.name;
    row.bookingDate = base.bookingDate;
    row.amount = base.amount;
    row.allocatable = base.allocatable;
    row.contractId = base.contractId;
    row.statementId = base.statementId;
    row.duplicateCount = static_cast<int>(group.size());
    row.calcVariant = similar;
    std::unordered_set<std::string> srcIds;
    for (const auto &item : group) {
      if (!srcIds.insert(item.sourceAnalysisId).second)
        continue;
      row.sourceAnalysisIds.push_back(item.sourceAnalysisId);
      row.sourceAnalysisNames.push_back(item.sourceAnalysisName);
    }
    const auto contractIt = contractTypeById.find(row.contractId);
    if (contractIt != contractTypeById.end())
      row.contractType = contractIt->second;

    auto liveIt = row.transactionId.empty() ? liveById.end()
                                            : liveById.find(row.transactionId);
    const bool hasLive = liveIt != liveById.end();
    row.missingLive = !hasLive;
    const int year = bookingYear(row.bookingDate);
    row.mixedYear = out.year > 0 && year > 0 && year != out.year;
    if (hasLive) {
      row.status = static_cast<int>(liveIt->second.status);
      row.allocatable = liveIt->second.allocatable;
      row.contractId = liveIt->second.contractId;
      row.statementId = liveIt->second.statementId;
      const auto cIt = contractTypeById.find(row.contractId);
      row.contractType = cIt != contractTypeById.end() ? cIt->second : "";
    }
    return row;
  };

  if (annual->analysisIds.size() <= 1) {
    for (const auto &tx : allSnapshotRows) {
      auto row = buildRow({tx}, false, false);
      out.deduplicated.push_back(row);
      if (row.missingLive)
        out.stats.missingLive += 1;
      if (row.mixedYear)
        out.stats.mixedYear += 1;
    }

    std::unordered_set<std::string> snapshotIds;
    for (const auto &tx : allSnapshotRows)
      if (!tx.id.empty())
        snapshotIds.insert(tx.id);
    for (const auto &live : workspace.transactions) {
      if (snapshotIds.count(live.id) > 0)
        continue;
      if (out.year > 0 && bookingYear(live.bookingDate) != out.year)
        continue;
      annual_ports::AnnualRowResult row;
      row.key = "live|" + live.id;
      row.transactionId = live.id;
      row.transactionName = live.name;
      row.bookingDate = live.bookingDate;
      row.amount = live.amount;
      row.status = static_cast<int>(live.status);
      row.allocatable = live.allocatable;
      row.contractId = live.contractId;
      row.statementId = live.statementId;
      const auto cIt = contractTypeById.find(live.contractId);
      row.contractType = cIt != contractTypeById.end() ? cIt->second : "";
      out.workspaceOnly.push_back(row);
      out.stats.missingFromYear += 1;
    }

    auto updateStatus = [&](const annual_ports::AnnualRowResult &row) {
      if (row.status == 1)
        out.stats.unverified += 1;
      else if (row.status == 2)
        out.stats.verified += 1;
      else if (row.status == 3)
        out.stats.completed += 1;
      else
        out.stats.neutral += 1;
    };
    for (const auto &row : out.deduplicated)
      updateStatus(row);
    return out;
  }

  std::unordered_set<std::string> consumedExact;
  consumedExact.reserve(byExact.size());
  for (const auto &[key, group] : byExact) {
    if (group.empty())
      continue;
    if (group.size() > 1) {
      auto row = buildRow(group, false, false);
      out.deduplicated.push_back(row);
      out.stats.duplicateCount += 1;
      if (row.missingLive)
        out.stats.missingLive += 1;
      if (row.mixedYear)
        out.stats.mixedYear += 1;
      consumedExact.insert(key);
    }
  }

  for (const auto &[clusterKey, cluster] : byDateContract) {
    if (cluster.size() < 2)
      continue;
    std::unordered_map<double, std::vector<SnapshotTx>> byAmount;
    for (const auto &tx : cluster)
      byAmount[tx.amount].push_back(tx);
    if (byAmount.size() < 2)
      continue;

    std::vector<SnapshotTx> similarGroup;
    for (const auto &[amount, items] : byAmount) {
      (void)amount;
      for (const auto &tx : items) {
        if (consumedExact.count(exactKey(tx)) > 0)
          continue;
        similarGroup.push_back(tx);
      }
    }
    if (similarGroup.size() > 1) {
      auto row = buildRow(similarGroup, true, false);
      out.similar.push_back(row);
      if (row.missingLive)
        out.stats.missingLive += 1;
      if (row.mixedYear)
        out.stats.mixedYear += 1;
      for (const auto &tx : similarGroup)
        consumedExact.insert(exactKey(tx));
    }
  }

  for (const auto &tx : allSnapshotRows) {
    const auto key = exactKey(tx);
    if (consumedExact.count(key) > 0)
      continue;
    auto row = buildRow({tx}, false, true);
    out.divergent.push_back(row);
    if (row.missingLive)
      out.stats.missingLive += 1;
    if (row.mixedYear)
      out.stats.mixedYear += 1;
  }

  std::unordered_set<std::string> snapshotIds;
  for (const auto &tx : allSnapshotRows)
    if (!tx.id.empty())
      snapshotIds.insert(tx.id);
  for (const auto &live : workspace.transactions) {
    if (snapshotIds.count(live.id) > 0)
      continue;
    if (out.year > 0 && bookingYear(live.bookingDate) != out.year)
      continue;
    annual_ports::AnnualRowResult row;
    row.key = "live|" + live.id;
    row.transactionId = live.id;
    row.transactionName = live.name;
    row.bookingDate = live.bookingDate;
    row.amount = live.amount;
    row.status = static_cast<int>(live.status);
    row.allocatable = live.allocatable;
    row.contractId = live.contractId;
    row.statementId = live.statementId;
    const auto cIt = contractTypeById.find(live.contractId);
    row.contractType = cIt != contractTypeById.end() ? cIt->second : "";
    out.workspaceOnly.push_back(row);
    out.stats.missingFromYear += 1;
  }

  auto updateStatus = [&](const annual_ports::AnnualRowResult &row) {
    if (row.status == 1)
      out.stats.unverified += 1;
    else if (row.status == 2)
      out.stats.verified += 1;
    else if (row.status == 3)
      out.stats.completed += 1;
    else
      out.stats.neutral += 1;
  };
  for (const auto &row : out.deduplicated)
    updateStatus(row);
  for (const auto &row : out.similar)
    updateStatus(row);
  for (const auto &row : out.divergent)
    updateStatus(row);

  return out;
}

} // namespace core::application::annual
