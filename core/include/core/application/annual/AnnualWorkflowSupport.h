/**
 * @file core/include/core/application/annual/AnnualWorkflowSupport.h
 * @brief Shared annual preview helpers for UI and application layers.
 */

#pragma once

#include "core/ports/usecases/annual/AnnualResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <string>
#include <vector>

namespace core::application::annual {

inline constexpr const char* kPreviewAnnualId = "__annual_preview__";

[[nodiscard]] core::ports::annual::AnnualResult
sortAnnualResult(const core::ports::annual::AnnualResult& result);

[[nodiscard]] core::ports::workspace::WorkspaceSnapshot withPreviewAnnual(
    core::ports::workspace::WorkspaceSnapshot workspace,
    const std::string& annualId,
    const std::vector<std::string>& analysisIds,
    int year);

} // namespace core::application::annual
