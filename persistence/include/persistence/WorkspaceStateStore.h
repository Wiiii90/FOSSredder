/**
 * @file persistence/include/persistence/WorkspaceStateStore.h
 * @brief Declares SQLite-backed loading and saving of the full application
 * state.
 */

#pragma once

#include <memory>

#include "core/application/storage/DeletionImpact.h"
#include "core/application/workspace/WorkspaceSessionState.h"

class SqliteDb;

/**
 * @brief Loads and saves the aggregate application state through SQLite
 * repositories.
 */
class WorkspaceStateStore {
public:
  /** @brief Creates a state store over an existing SQLite database connection
   * wrapper. */
  explicit WorkspaceStateStore(std::shared_ptr<SqliteDb> db);

  /** @brief Loads the complete persisted application state from the database.
   */
  core::application::workspace::WorkspaceSessionState load();
  /** @brief Saves the complete application state and reports deletion impact
   * details. */
  core::domain::DeletionImpact
  save(const core::application::workspace::WorkspaceSessionState &document);

private:
  std::shared_ptr<SqliteDb> db_;
};
