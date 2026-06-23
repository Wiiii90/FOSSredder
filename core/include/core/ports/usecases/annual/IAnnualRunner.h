/**
 * @file core/include/core/ports/usecases/annual/IAnnualRunner.h
 * @brief Annual use-case port consumed by application boundaries.
 */

#pragma once

#include "core/ports/usecases/annual/AnnualRequest.h"
#include "core/ports/usecases/annual/AnnualResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::ports::annual {

class IAnnualRunner {
public:
    /**
     * @brief Destroys the annual runner port.
     */
    virtual ~IAnnualRunner() = default;

    /**
     * @brief Computes annual reporting data from a workspace snapshot and request.
     * @param workspace Workspace data used as the annual report input.
     * @param request Annual report configuration.
     * @return Computed annual report result.
     */
    [[nodiscard]] virtual AnnualResult
    runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
              const AnnualRequest& request) const = 0;
};

} // namespace core::ports::annual
