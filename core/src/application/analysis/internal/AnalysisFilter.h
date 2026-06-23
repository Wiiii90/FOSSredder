/**
 * @file core/src/application/analysis/internal/AnalysisFilter.h
 * @brief Declares internal analysis filter predicates and collection helpers.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Transaction.h"

namespace core::application::analysis {

/**
 * @brief Aggregates transaction predicates parsed from one analysis filter
 * specification.
 */
class AnalysisFilter {
public:
  using Pred =
      std::function<bool(const std::shared_ptr<core::domain::Transaction> &,
                         const core::domain::catalog::WorkspaceCatalog &)>;

  /** @brief Adds one predicate to the composed filter. */
  void addPredicate(Pred p) { preds_.push_back(std::move(p)); }

  /**
   * @brief Evaluates all predicates against one transaction.
   * @param t Transaction candidate.
   * @param state Current workspace catalog snapshot.
   * @return `true` when all predicates accept the transaction.
   */
  bool matches(const std::shared_ptr<core::domain::Transaction> &t,
               const core::domain::catalog::WorkspaceCatalog &state) const {
    for (const auto &p : preds_)
      if (!p(t, state))
        return false;
    return true;
  }

  /** @brief Checks whether no predicates have been added. */
  bool empty() const { return preds_.empty(); }

private:
  std::vector<Pred> preds_;
};

/**
 * @brief Parses a raw filter specification into predicate form.
 * @param spec Raw filter specification.
 * @return Parsed predicate aggregate.
 */
AnalysisFilter parseAnalysisFilterSpec(const std::string &spec);

/**
 * @brief Collects transactions that satisfy the supplied filter.
 * @param state Current workspace catalog snapshot.
 * @param filter Parsed filter predicates.
 * @return Transactions accepted by the filter.
 */
std::vector<std::shared_ptr<core::domain::Transaction>>
collectAnalysisTransactions(
    const core::domain::catalog::WorkspaceCatalog &state,
    const AnalysisFilter &filter);

} // namespace core::application::analysis
