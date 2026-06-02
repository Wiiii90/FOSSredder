/**
 * @file ui/include/ui/workflows/annual/AnnualWorkflow.h
 * @brief Declares the UI workflow for annual result computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::adapters {
class AnnualAdapter;
}

namespace ui {

/**
 * @brief Coordinates annual computation for UI states.
 */
class AnnualWorkflow : public QObject {
  Q_OBJECT

public:
  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;

  explicit AnnualWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::AnnualAdapter> annualAdapter,
      QObject *parent = nullptr);

  QVariantMap computeAnnual(const QString &annualId) const;
  QVariantMap computeAnnualPreview(const QString &annualId,
                                   const QStringList &analysisIds,
                                   int year) const;

private:
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<ui::adapters::AnnualAdapter> annualAdapter_;
};

} // namespace ui
