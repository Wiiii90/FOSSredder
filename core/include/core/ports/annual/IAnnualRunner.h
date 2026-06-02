/**
 * @file core/include/core/ports/annual/IAnnualRunner.h
 * @brief Annual use-case port consumed by application boundaries.
 */

#pragma once

#include "core/ports/annual/AnnualRequest.h"
#include "core/ports/annual/AnnualResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::annual {

class IAnnualRunner {
public:
    virtual ~IAnnualRunner() = default;

    [[nodiscard]] virtual AnnualResult
    runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
              const AnnualRequest& request) const = 0;
};

} // namespace core::ports::annual
