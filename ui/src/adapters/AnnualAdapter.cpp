/**
 * @file ui/src/adapters/AnnualAdapter.cpp
 * @brief Implements the annual adapter used by UI workflows.
 */

#include "ui/adapters/AnnualAdapter.h"

#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadMapper.h"

#include <utility>

namespace ui::adapters {
namespace {

QVariantMap toAnnualRow(const core::ports::annual::AnnualRowResult &row) {
  QVariantMap out;
  out.insert(QStringLiteral("key"), QString::fromStdString(row.key));
  out.insert(QStringLiteral("id"), QString::fromStdString(row.transactionId));
  out.insert(QStringLiteral("name"),
             QString::fromStdString(row.transactionName));
  out.insert(QStringLiteral("bookingDate"),
             QString::fromStdString(row.bookingDate));
  out.insert(QStringLiteral("amount"), row.amount);
  out.insert(QStringLiteral("status"), row.status);
  out.insert(QStringLiteral("statusText"),
             row.status == 1   ? QStringLiteral("Unverified")
             : row.status == 2 ? QStringLiteral("Verified")
             : row.status == 3 ? QStringLiteral("Completed")
                               : QStringLiteral("Neutral"));
  out.insert(QStringLiteral("allocatable"), row.allocatable);
  out.insert(QStringLiteral("contractId"),
             QString::fromStdString(row.contractId));
  out.insert(QStringLiteral("contractType"),
             QString::fromStdString(row.contractType));
  out.insert(QStringLiteral("statementId"),
             QString::fromStdString(row.statementId));
  out.insert(QStringLiteral("isMissingLive"), row.missingLive);
  out.insert(QStringLiteral("isMixedYear"), row.mixedYear);
  out.insert(QStringLiteral("isCalcVariant"), row.calcVariant);
  out.insert(QStringLiteral("isDuplicate"), row.duplicateCount > 1);
  out.insert(QStringLiteral("duplicateCount"), row.duplicateCount);
  out.insert(QStringLiteral("sourceAnalysisIds"),
             ui::payload::mapper::toVariantStringList(row.sourceAnalysisIds));
  out.insert(QStringLiteral("sourceAnalysisNames"),
             ui::payload::mapper::toVariantStringList(row.sourceAnalysisNames));
  return out;
}

QVariantList
toAnnualRows(const std::vector<core::ports::annual::AnnualRowResult> &rows) {
  QVariantList out;
  out.reserve(static_cast<int>(rows.size()));
  for (const auto &row : rows) {
    out.push_back(toAnnualRow(row));
  }
  return out;
}

} // namespace

AnnualAdapter::AnnualAdapter(
    std::shared_ptr<core::ports::annual::IAnnualRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::annual::AnnualResult AnnualAdapter::runAnnual(
    const core::ports::workspace::WorkspaceSnapshot &workspace,
    const core::ports::annual::AnnualRequest &request) const {
  observability::traceAdapter(
      "AnnualAdapter::runAnnual", "Annual runner invoked",
      {{"annualId", request.annualId}});
  return runner_ ? runner_->runAnnual(workspace, request)
                 : core::ports::annual::AnnualResult{};
}

QVariantMap AnnualAdapter::mapAnnualResult(
    const core::ports::annual::AnnualResult &result) const {
  QVariantMap out;
  out.insert(QStringLiteral("annualId"),
             QString::fromStdString(result.annualId));
  out.insert(QStringLiteral("annualName"),
             QString::fromStdString(result.annualName));
  out.insert(QStringLiteral("year"), result.year);

  QVariantMap stats;
  stats.insert(QStringLiteral("assignedAnalysisCount"),
               result.stats.assignedAnalysisCount);
  stats.insert(QStringLiteral("snapshotTransactionCount"),
               result.stats.snapshotTransactionCount);
  stats.insert(QStringLiteral("missingFromYear"),
               result.stats.missingFromYear);
  stats.insert(QStringLiteral("mixedYear"), result.stats.mixedYear);
  stats.insert(QStringLiteral("duplicateCount"),
               result.stats.duplicateCount);
  stats.insert(QStringLiteral("missingLive"), result.stats.missingLive);
  stats.insert(QStringLiteral("neutral"), result.stats.neutral);
  stats.insert(QStringLiteral("unverified"), result.stats.unverified);
  stats.insert(QStringLiteral("verified"), result.stats.verified);
  stats.insert(QStringLiteral("completed"), result.stats.completed);
  out.insert(QStringLiteral("stats"), stats);

  const QVariantList deduplicated = toAnnualRows(result.deduplicated);
  const QVariantList similar = toAnnualRows(result.similar);
  const QVariantList divergent = toAnnualRows(result.divergent);
  const QVariantList workspaceOnly = toAnnualRows(result.workspaceOnly);

  out.insert(QStringLiteral("deduplicated"), deduplicated);
  out.insert(QStringLiteral("similar"), similar);
  out.insert(QStringLiteral("divergent"), divergent);
  out.insert(QStringLiteral("workspaceOnly"), workspaceOnly);

  QVariantList all;
  all.reserve(deduplicated.size() + similar.size() + divergent.size() +
              workspaceOnly.size());
  for (const auto &value : deduplicated) {
    all.push_back(value);
  }
  for (const auto &value : similar) {
    all.push_back(value);
  }
  for (const auto &value : divergent) {
    all.push_back(value);
  }
  for (const auto &value : workspaceOnly) {
    all.push_back(value);
  }
  out.insert(QStringLiteral("transactions"), all);
  return out;
}

} // namespace ui::adapters
