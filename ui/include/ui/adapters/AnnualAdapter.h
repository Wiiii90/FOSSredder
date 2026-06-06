/**
 * @file ui/include/ui/adapters/AnnualAdapter.h
 * @brief Declares the annual adapter used by UI workflows.
 */

#pragma once

#include <memory>

#include <QVariantMap>

#include "core/ports/usecases/annual/AnnualResult.h"
#include "core/ports/usecases/annual/IAnnualRunner.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {

class AnnualAdapter final {
public:
  /**
   * @brief Creates an annual adapter backed by a core runner.
   * @param runner Core annual runner port.
   */
  explicit AnnualAdapter(
      std::shared_ptr<core::ports::annual::IAnnualRunner> runner);

  /**
   * @brief Runs annual computation through the core runner.
   * @param workspace Workspace snapshot used as input.
   * @param request Annual request.
   * @return Core annual result.
   */
  core::ports::annual::AnnualResult
  runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
            const core::ports::annual::AnnualRequest& request) const;

  /**
   * @brief Maps an annual result to a QML payload.
   * @param result Core annual result.
   * @return QML payload map.
   */
  [[nodiscard]] QVariantMap
  mapAnnualResult(const core::ports::annual::AnnualResult& result) const;

private:
  std::shared_ptr<core::ports::annual::IAnnualRunner> runner_;
};

} // namespace ui::adapters
