/**
 * @file ui/include/ui/workspace/WorkspaceStore.h
 * @brief Declares the UI workspace snapshot store.
 */

#pragma once

#include <QObject>
#include <QString>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui {

class WorkspaceCommands;
class WorkspaceSelectors;

/**
 * @brief Owns the current UI-side workspace snapshot and revision lifecycle.
 */
class WorkspaceStore : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Creates an empty workspace store.
   * @param parent Optional Qt parent.
   */
  explicit WorkspaceStore(QObject* parent = nullptr);

  /**
   * @brief Binds core workspace ports used by workspace roles.
   * @param writer Workspace writer port or nullptr.
   * @param reader Workspace reader port or nullptr.
   */
  void setWorkspacePorts(core::ports::workspace::IWorkspaceWriter* writer,
                         core::ports::workspace::IWorkspaceReader* reader);

  /**
   * @brief Loads a fresh workspace snapshot.
   * @param snapshot Snapshot returned by the core workspace reader.
   */
  void loadFromState(const core::ports::workspace::WorkspaceSnapshot& snapshot);

  /**
   * @brief Applies deletion side effects to the stored snapshot.
   * @param impact Deletion impact reported by the core workspace writer.
   */
  void
  applyDeletionImpact(const core::ports::workspace::DeletionImpact& impact);

  /**
   * @brief Returns the current snapshot.
   * @return Stored workspace snapshot.
   */
  const core::ports::workspace::WorkspaceSnapshot& snapshot() const noexcept {
    return snapshot_;
  }

  /**
   * @brief Returns the current workspace path.
   * @return Current path as a Qt string, or empty when no file is open.
   */
  QString currentPath() const;

  /**
   * @brief Returns the current data revision.
   * @return Monotonic revision incremented on snapshot changes.
   */
  int dataRevision() const noexcept {
    return dataRevision_;
  }

signals:
  void dataRevisionChanged();

private:
  friend class WorkspaceCommands;
  friend class WorkspaceSelectors;

  /**
   * @brief Returns the bound workspace writer.
   * @return Workspace writer or nullptr.
   */
  core::ports::workspace::IWorkspaceWriter* writer() const noexcept {
    return writer_;
  }

  /**
   * @brief Returns the bound workspace reader.
   * @return Workspace reader or nullptr.
   */
  core::ports::workspace::IWorkspaceReader* reader() const noexcept {
    return reader_;
  }

  /**
   * @brief Increments the data revision and emits the revision signal.
   */
  void bumpDataRevision();

  core::ports::workspace::WorkspaceSnapshot snapshot_;
  core::ports::workspace::IWorkspaceWriter* writer_ = nullptr;
  core::ports::workspace::IWorkspaceReader* reader_ = nullptr;
  int dataRevision_ = 0;
};

} // namespace ui
