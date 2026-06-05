/**
 * @file ui/include/ui/workflows/AnnualWorkflow.h
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

  /**
   * @brief Creates an annual workflow.
   * @param stateSnapshotProvider Provider for the current workspace snapshot.
   * @param annualAdapter Adapter used to invoke the core annual runner.
   * @param parent Optional Qt parent.
   */
  explicit AnnualWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<ui::adapters::AnnualAdapter> annualAdapter,
      QObject* parent = nullptr);

  /**
   * @brief Computes an annual preview through the core runner.
   * @param annualId Annual id.
   * @param analysisIds Analysis ids assigned to the annual.
   * @param year Annual year.
   * @return QML annual result payload.
   */
  QVariantMap computeAnnualPreview(const QString& annualId,
                                   const QStringList& analysisIds,
                                   int year) const;

private:
  /**
   * @brief Returns the current workspace snapshot.
   * @return Workspace snapshot or an empty snapshot.
   */
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<ui::adapters::AnnualAdapter> annualAdapter_;
};

} // namespace ui
