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
  explicit AnnualAdapter(
      std::shared_ptr<core::ports::annual::IAnnualRunner> runner);

  core::ports::annual::AnnualResult
  runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
            const core::ports::annual::AnnualRequest& request) const;

  [[nodiscard]] QVariantMap
  mapAnnualResult(const core::ports::annual::AnnualResult& result) const;

private:
  std::shared_ptr<core::ports::annual::IAnnualRunner> runner_;
};

} // namespace ui::adapters
