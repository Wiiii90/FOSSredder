/**
 * @file core/include/core/application/import/draft/DraftFinalizer.h
 * @brief Declares draft finalization helpers that materialize imported entities.
 */

#pragma once

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/application/import/draft/StatementDraft.h"

#include <string>

namespace core::application::importing::draft {

/**
 * @brief Converts a statement draft into persisted workspace entities.
 */
class DraftFinalizer {
public:
    /**
     * @brief Finalize a statement draft and append the resulting entities to the workspace state.
     * @param state Workspace catalog that receives the generated statement, transactions, and optional contract.
     * @param draft Statement draft to materialize.
     * @return The generated statement identifier, or an empty string when the draft contains no transactions.
     */
    static std::string finalize(core::domain::catalog::WorkspaceCatalog& state, const core::application::importing::draft::StatementDraft& draft);
};

}