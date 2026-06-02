/**
 * @file core/src/application/annual/AnnualWorkflowSupport.cpp
 * @brief Shared annual preview helpers for UI and application layers.
 */

#include "core/application/annual/AnnualWorkflowSupport.h"

#include <algorithm>

namespace core::application::annual {

namespace {

auto rowLess = [](const core::ports::annual::AnnualRowResult& lhs,
                  const core::ports::annual::AnnualRowResult& rhs) {
    if (lhs.bookingDate != rhs.bookingDate) {
        return lhs.bookingDate < rhs.bookingDate;
    }
    if (lhs.amount != rhs.amount) {
        return lhs.amount < rhs.amount;
    }
    return lhs.transactionId < rhs.transactionId;
};

} // namespace

core::ports::annual::AnnualResult
sortAnnualResult(const core::ports::annual::AnnualResult& result) {
    auto projected = result;
    std::sort(projected.deduplicated.begin(), projected.deduplicated.end(),
              rowLess);
    std::sort(projected.similar.begin(), projected.similar.end(), rowLess);
    std::sort(projected.divergent.begin(), projected.divergent.end(), rowLess);
    std::sort(projected.workspaceOnly.begin(), projected.workspaceOnly.end(),
              rowLess);
    return projected;
}

core::ports::workspace::WorkspaceSnapshot withPreviewAnnual(
    core::ports::workspace::WorkspaceSnapshot workspace,
    const std::string& annualId, const std::vector<std::string>& analysisIds,
    int year) {
    core::ports::workspace::AnnualSnapshot previewAnnual;
    bool found = false;
    const std::string previewId =
        annualId.empty() ? std::string(kPreviewAnnualId) : annualId;

    for (const auto& row : workspace.annuals) {
        if (row.id != previewId) {
            continue;
        }
        previewAnnual = row;
        found = true;
        break;
    }

    previewAnnual.id = previewId;
    if (year > 0) {
        previewAnnual.year = year;
    }
    previewAnnual.analysisIds = analysisIds;

    if (found) {
        for (auto& row : workspace.annuals) {
            if (row.id != previewId) {
                continue;
            }
            row = previewAnnual;
            return workspace;
        }
    }

    workspace.annuals.push_back(previewAnnual);
    return workspace;
}

} // namespace core::application::annual
