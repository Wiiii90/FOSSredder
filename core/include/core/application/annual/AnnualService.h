/**
 * @file core/include/core/application/annual/AnnualService.h
 * @brief Builds annual derived snapshots from workspace snapshots.
 */

#pragma once

#include <string>

#include "core/ports/annual/AnnualRequest.h"
#include "core/ports/annual/AnnualResult.h"
#include "core/ports/annual/IAnnualRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::application::annual {

class AnnualService : public core::ports::annual::IAnnualRunner {
public:
  [[nodiscard]] core::ports::annual::AnnualResult
  runAnnual(const core::ports::workspace::WorkspaceSnapshot &workspace,
            const core::ports::annual::AnnualRequest &request) const override;
  [[nodiscard]] core::ports::annual::AnnualResult
  buildAnnualResult(const core::ports::workspace::WorkspaceSnapshot &workspace,
                    const std::string &annualId) const;
};

} // namespace core::application::annual
